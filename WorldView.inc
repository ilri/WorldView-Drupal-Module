<?php
// $Id$

/**
 * Copyright 2011 ILRI
 *
 * This file is part of WorldView.
 * 
 * WorldView is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * WorldView is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with WorldView.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *   Functions and constants used by the WorldView module
 *
 *   This include file implements the defaults for the WorldView module.
*/

/**
 * WorldView directory
*/
define('WORLDVIEW_ROOT', DRUPAL_ROOT . '/' . drupal_get_path('module', 'WorldView'));

/**
 * A simple interface to an external database
 *
 * @param string $host
 *   Hostname of the external database
 * @param string $user
 *   Username for the external database
 * @param string $pass
 *   Password for the external database
 * @param string $name
 *   Name of the external database
 *
 * @return array $connection
 *   An associative array with three values:
 *   - link: a resource containing the database interface
 *   - status: a status message
 *   - error: any error messages
*/
function _WorldView_external_db($host, $user, $pass, $name) {
  // Connect to database and verify connection
  $connection['link'] = mysql_connect($host, $user, $pass);
  if (!$connection['link']) {
    $connection['error'] = mysql_error();
    $connection['status'] = 'Not Connected.';
  }
  else {
    if (!mysql_select_db($name, $connection['link'])) {
      $connection['error'] = 'Database ' . $name . ' not found.';
      $connection['status'] = 'Not Connected.';
    } else {
        $connection['error'] = NULL;
        $connection['status'] = $name.'@'.$host;
    }
  }
  return $connection;
}

/**
 * Formats WorldView icons to be used in Google Earth.
 *
 * Creates an associative array of icons and default formatting information to
 * be used by _WorldView_print_KML.
 *
 * @param DOMDocument &$dom
 *   The XML generator
 * @param string $icon
 *   Name of the icon png to be loaded.
 *
 * @return DOMElement $restStyleNode
 *   A DOMElement containing the icon style
*/
function _WorldView_Icon(&$dom, $icon = 'default', $x = 0.5, $y = 0.5) {
  if (substr($icon, -4) == '.png') {
    $icon = substr($icon, 0, -4);
  }
  
  $restStyleNode = $dom->createElement('Style');
  $restStyleNode->setAttribute('id', $icon . 'Style');
  $restIconstyleNode = $dom->createElement('IconStyle');
  $restIconstyleNode->setAttribute('id', $icon . 'Icon');
  $restIconNode = $dom->createElement('Icon');
  $restHref = $dom->createElement('href', url('', array('absolute' => True)) . '/'
                                          . drupal_get_path('module', 'WorldView')
                                          . '/icons/' . $icon . '.png');
  $hotspot = $dom->createElement('hotSpot');
  $hotspot->setAttribute('x', $x);
  $hotspot->setAttribute('y', $y);
  $hotspot->setAttribute('xunits', 'fraction');
  $hotspot->setAttribute('yunits', 'fraction');
  $restIconNode->appendChild($restHref);
  $restIconstyleNode->appendChild($restIconNode);
  $restIconstyleNode->appendChild($hotspot);
  $restStyleNode->appendChild($restIconstyleNode);
  return $restStyleNode;
}

/**
 * Creates a KML placemark
 *
 * @param DOMDocument &$dom
 *   The XML generator
 * @param string $name
 *   Name of the placemark.
 * @param string $desc
 *   description of the placemark.
 * @param string $lat
 *   Latitude of the placemark.
 * @param string $lon
 *   Longtitude of the placemark.
 * @param string $icon
 *   Name of the icon png to be used.
 *
 * @return DOMElement $placeNode
 *   A DOMElement containing the ILRI placemark
*/
function _WorldView_Placemark(&$dom, $name, $desc, $lat, $lon, $icon = "default") {
  $placeNode = $dom->createElement('Placemark');
  
  $placeNode->setAttribute('id', 'placemark' . str_replace(' ', '_', $name));
  $nameNode = $dom->createElement('name', '');
  $placeNode->appendChild($nameNode);
  $descNode = $dom->createElement('description', '<h3>' . $name . '</h3>' . $desc);
  $placeNode->appendChild($descNode);
  $styleUrl = $dom->createElement('styleUrl', '#' . $icon . 'Style');
  $placeNode->appendChild($styleUrl);
  
  $pointNode = $dom->createElement('Point');
  $placeNode->appendChild($pointNode);
  
  $coorStr = $lon . ',' . $lat;
  $coorNode = $dom->createElement('coordinates', $coorStr);
  $pointNode->appendChild($coorNode);
  return $placeNode;
}

/**
 * If installed this function gets information from the databridge module.
 *
 * If the DataBridge module is installed this function will extract sampling
 * types and plot them on the map. If not installed this function does 
 * nothing and can safely be ignored. Note that all samplings will attempt to
 * use a map icon with the same name as their sampling class. i.e. "livestock" 
 * samplings will attempt to use "livestock.png" as icon.
 *
 * @return array $samplings
 *   A nested array of sampling information.
 *
*/
function WorldView_DataBridge(&$dom) {
  $placemarks = array();
  if (module_exists('DataBridge')) {
    $raw = DataBridge_getSamplingInfo();
    $data = array();
    foreach ($raw as $class => $trips) {
      $data[$class] = array();
      foreach ($trips as $id => $trip) {
        $info = '';
        
        // Add dates
        
        $info .= strftime('%Y-%m-%d', $trip['start_date']);
        $info .= $trip['end_date'] ?  ' to ' . strftime('%Y-%m-%d %H:%M', $trip['end_date']) : '';
        
        // Print the table
        $info .= theme('table', DataBridge_table($trip));
        
        // In case the user has permission to view sensitive data, add links 
        // to the information pane. This currently doesn't work as verifying 
        // that users are logged in is tricky - since the data is requested by
        // the google earth server and not by the client.
        //
        // NOTE: TO REMEDY THIS PROBLEM THE INTITAL PERMISSION CHECK IS OMITTED
        /*
        if (user_access('access DataBridge private')) {
        */
          $info .= '<hr />' . l('Download Summary', 'databridge/sampling/' . $class . '/' . $trip['location'] . 
                                              '/' . strftime('%Y-%m-%d', $trip['start_date']) . '/xls') . 
                      ' | ' . l('View Details', 'databridge/sampling/' . $class . '/' . $trip['location'] . 
                                                          '/' . strftime('%Y-%m-%d', $trip['start_date'])); 
        /*
        }
        */
        
        // Concatenate everything into a neat information array
        array_push($placemarks,  _WorldView_Placemark($dom, $trip['location'] . ' ' . ucfirst($class) . ' Sampling', 
                                                            $info, 
                                                            $trip['latitude'], 
                                                            $trip['longitude'], 
                                                            $class));
      }
    }
  }
  return $placemarks;
}

/**
 * ILRI ONLY FUNCTIONS - REMOVE THESE OR AT LEAST DON'T RELY ON THEM AS THEY
 * ARE PRONE TO CHANGING TO FIT THE SPECIFIC NEEDS OF THE ILRI IMPLEMENTATION
*/

/**
 * Creates a placemark for ILRI
 *
 * TODO: replace with a static data handler.
 *
 * @param DOMDocument &$dom
 *   The XML generator
 * @param string $icon
 *   Name of the icon png to be used.
 *
 * @return DOMElement $placeNode
 *   A DOMElement containing the ILRI placemark
*/
function _WorldView_ILRI(&$dom, $icon = "default") {
  return _WorldView_Placemark($dom, 'ILRI', 'the place to be', '-1.2686', '36.722287', $icon);
}
