<?php

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
 *   Google Earth functions and constants used by the WorldView module
 *
 *   This include file implements google earth functions for the WorldView 
 *   module. 
*/

/**
 * Returns the WorldView data formatted for Google Earth.
 *
 * @param array $places
 *   An array of WorldView_placemarks to plot on the map
 *
 * @return $dom
 *   A KML-formatted page.
*/
function WorldView_google_earth_data($places) {
  // Set content type for Drupal
  drupal_add_http_header('Content-Type', 'application/vnd.google-earth.kml+xml');
  drupal_add_http_header('Content-Disposition', 'inline; filename=worldview.kml;');
  
  /* Create a new DOMDocument that will hold our KML, formatted output takes 
   * slightly more space but feels more neat and makes debugging easier.
   */
  $dom = new DOMDocument('1.0', 'UTF-8');
  $dom->formatOutput = true;
  
  // Creates the root KML element and appends it to the root document.
  $node = $dom->createElementNS('http://earth.google.com/kml/2.2', 'kml');
  $parNode = $dom->appendChild($node);

  // Creates a KML Document element and append it to the KML element.
  $dnode = $dom->createElement('Document');
  $docNode = $parNode->appendChild($dnode);
  
  // Get a list of all icons we should load
  $icons = array();
  foreach ( $places as $place ) {
    if ( !in_array($place->icon, $icons) ) {
      array_push($icons, $place->icon);
    }
  }
  
  // Load all icons we need
  foreach ( $icons as $icon ) {
    $docNode->appendChild(_WorldView_KML_Icon($dom, $icon));
  }
  
  // Add a folder for places
  $fnode = $dom->createElement('Folder');
  $folderNode = $parNode->appendChild($fnode);
  
  // Add all the places we got
  foreach ( $places as $place ) {
    $folderNode->appendChild(_WorldView_KML_Placemark($dom, $place));
  }
  
  // Return the KML Document.
  return $dom->saveXML();
}

/**
 * Creates a KML placemark
 *
 * @param DOMDocument &$dom
 *   The XML generator
 * @param WorldView_placemark $place
 *   The placemark to add to the DOM
 *
 * @return DOMElement $placeNode
 *   A DOMElement containing the placemark
*/
function _WorldView_KML_Placemark(&$dom, $place) {
  $placeNode = $dom->createElement('Placemark');
  
  $id = $place->db_id ? $place->db_id : str_replace(' ', '_', $place->title);
  $placeNode->setAttribute('id', 'placemark_' . $id);
  $nameNode = $dom->createElement('name', '');
  $placeNode->appendChild($nameNode);
  
  // Format the placemark dates
  $dates  = $place->start_date ? strftime('%Y-%m-%d', $place->start_date) : '';
  $dates .= $place->end_date ?  ' to ' . strftime('%Y-%m-%d', $place->end_date) : '';
  
  // Format the placemark user info
  $user_info  = $place->created_by ? 'Created by: ' . $place->created_by . 
                                     ' at ' . strftime('%Y-%m-%d %H:%M', $place->created_at)
                                   : '';
  $user_info .= $place->updated_by ? 'Updated by: ' . $place->updated_by . 
                                     ' at ' . strftime('%Y-%m-%d %H:%M', $place->updated_at)
                                   : '';
  
  $descNode = $dom->createElement('description', '<h3>' . $place->title . '</h3>' . 
                                                 '<hr>' . 
                                                 '<small>' . $dates . '</small>' . 
                                                 $place->information . 
                                                 '<hr>' . 
                                                 $place->link . '<br>' . 
                                                 '<small>' . $user_info . '</small>');
  $placeNode->appendChild($descNode);
  
  if (substr($place->icon, -4) == '.png') {
    $icon = substr($place->icon, 0, -4);
  } else {
    $icon = $place->icon;
  }
  $styleUrl = $dom->createElement('styleUrl', '#' . $icon . 'Style');
  $placeNode->appendChild($styleUrl);
  
  $pointNode = $dom->createElement('Point');
  $placeNode->appendChild($pointNode);
  
  $coorStr = $place->longitude . ',' . $place->latitude;
  $coorNode = $dom->createElement('coordinates', $coorStr);
  $pointNode->appendChild($coorNode);
  return $placeNode;
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
function _WorldView_KML_Icon(&$dom, $icon = 'default', $x = 0.5, $y = 0) {
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
