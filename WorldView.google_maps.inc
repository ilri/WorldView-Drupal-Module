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
 *   Google Maps functions and constants used by the WorldView module
 *
 *   This include file implements google maps functions for the WorldView 
 *   module. 
*/

/**
 * Returns the WorldView data formatted for Google Maps.
 *
 * @param array $places
 *   An array of WorldView_placemarks to plot on the map
 *
 * @return $dom
 *   An XML-formatted page.
*/
function WorldView_google_maps_data($places) {
  // Set content type for Drupal
  
  /* Create a new DOMDocument that will hold our XML, formatted output takes 
   * slightly more space but feels more neat and makes debugging easier.
   */
  $dom = new DOMDocument('1.0', 'UTF-8');
  $dom->formatOutput = true;

  // Creates an XML markers element and append it to the DOMDocument.
  $mnode = $dom->createElement('markers');
  $markers = $dom->appendChild($mnode);
  
  // Creates an XML icons element and append it to the DOMDocument.
  $inode = $dom->createElement('icons');
  $icons = $dom->appendChild($inode);
  
  // Get a list of all icons we should load
  $icon_names = array();
  foreach ( $places as $place ) {
    if ( !in_array($place->icon, $icon_names) ) {
      array_push($icon_names, $place->icon);
    }
  }
  
  // Add all the places we got
  foreach ( $places as $place ) {
    
    // Get the icon url right
    $url = strpos($place->icon, ".") ? $place->icon :  $place->icon . '.png';
    $url = url('', array('absolute' => True)) . '/' . drupal_get_path('module', 'WorldView') . '/icons/' . $url;
    
    $node = $dom->createElement("marker");  
    $newnode = $markers->appendChild($node);
    $newnode->setAttribute("latitude", $place->latitude);
    $newnode->setAttribute("longitude", $place->longitude);
    $newnode->setAttribute("title", $place->title);
    $newnode->setAttribute("link", l($place->link, $place->link));
    $newnode->setAttribute("information", $place->information);
    $newnode->setAttribute("start_date", $place->start_date ? strftime('%Y-%m-%d', $place->start_date) : NULL);
    $newnode->setAttribute("end_date", $place->end_date ? strftime('%Y-%m-%d', $place->end_date) : NULL);
    $newnode->setAttribute("icon", $url);
    $newnode->setAttribute("created_by", $place->created_by ? $place->created_by : NULL);
    $newnode->setAttribute("created_at", $place->created_at ? strftime('%Y-%m-%d', $place->created_at) : NULL);
    $newnode->setAttribute("updated_by", $place->updated_by ? $place->updated_by : NULL);
    $newnode->setAttribute("updated_at", $place->updated_at ? strftime('%Y-%m-%d', $place->updated_at) : NULL);
  }
  
  // Return the XML Document.
  return $dom->saveXML();
}
