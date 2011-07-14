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
 * Handles placemarks internally in the module.
 *
 * A placemark is basically just an associative array. This class handles how 
 * it the internal data is converted to KML for Google Earth, supplied to 
 * Google Maps etc.
*/
class WorldView_placemark {
  var $latitude;
  var $longitude;
  var $title;
  var $link;
  var $information;
  var $start_date;
  var $end_date;
  var $icon;
  var $db_id;
  
  var $created_by;
  var $created_at;
  var $updated_by;
  var $updated_at;
  
  /**
   * Constructor. Creates an empty placemark if no variables are passed.
   *
   * @param $latitude = NULL
   *   Latitude of the placemark in decimal degrees.
   * @param $longitude = NULL
   *   Longitude of the placemark in decimal degrees.
   * @param $title = ""
   *   Title of the placemark.
   * @param $information = NULL
   *   Information to show when the placemark is expanded. May include HTML 
   *   tags.
   * @param $link = NULL
   *   Array formated as [URL, link_text].
   * @param $start_date = NULL
   *   Start date of the placemark event.
   * @param $end_date = NULL
   *   End date of the placemark event.
  */
  public function __construct($latitude = NULL, $longitude = NULL, $title = "", $information = NULL, $link = NULL, $start_date = NULL, $end_date = NULL, $icon = NULL) {
    $this->latitude    = $latitude;
    $this->longitude   = $longitude;
    $this->title       = $title;
    $this->information = $information;
    $this->start_date  = $start_date;
    $this->end_date    = $end_date;
    $this->icon        = $icon;
    
    // Do fanciness with links
    if ( $link && is_array($link) ) {
      if ( is_array($link[0]) ) {
        foreach ( $link as $l ) {
          if ( $this->link ) {
            $this->link .= l($l[0], $l[1]) . ' | ';
          } else {
            $this->link = l($l[0], $l[1]) . ' | ';            
          }
        }
        $this->link = substr($this->link, 0, -3);
      } else {
        $this->link = l($link[0], $link[1]);
      }
    } else {
      $this->link = NULL;
    }
  }

  /**
   * Saves the placemark to the database.
  */
  public function save() {
    global $user;
    $place = array('latitude'    => $this->latitude,
                   'longitude'   => $this->longitude,
                   'title'       => $this->title,
                   'link'        => $this->link,
                   'information' => $this->information,
                   'start_date'  => $this->$start_date,
                   'end_date'    => $this->$end_date,
                   'icon'        => $this->$icon,
                   );
    if ( $this->$db_id ) {
      $place['id'] = $this->$db_id;
      $place['updated_by'] = $user->name;
      $place['updated_at'] = time();
    } else {
      $place['created_by'] = $user->name;
      $place['created_at'] = time();
    }
    db_insert('worldview_places')
              ->fields($place)
              ->execute();
  }
  
  /**
   * Loads a placemark from the database given an $id.
   *
   * @param int $id
   *   Database id of the placemark to load.
  */
  public function load($id) {
    $db_info = db_select('worldview_places', 'w')
                        ->fields('w') // Equivalent of "SELECT * FROM"
                        ->condition('id', $id, '=')
                        ->execute()
                        ->fetchAssoc();
    
    $this->latitude    = $db_info['latitude'];
    $this->longitude   = $db_info['longitude'];
    $this->title       = $db_info['title'];
    $this->information = $db_info['information'];
    $this->link        = $db_info['link'];
    $this->start_date  = $db_info['start_date'];
    $this->end_date    = $db_info['end_date'];
    $this->icon        = $db_info['icon'];
    
    $this->db_id       = $db_info['id'];
    $this->created_by  = $db_info['created_by'];
    $this->created_at  = $db_info['created_at'];
    $this->updated_by  = $db_info['updated_by'];
    $this->updated_at  = $db_info['updated_at'];
  }
  
}

/**
 * Gets all places from the database into an array.
 *
 * @return array $places
 *   An array of all places in the internal database
*/
function WorldView_get_places() {
  
  // Get places from the internal database
  $ids = db_select('worldview_places', 'w')
                   ->fields('w', array('id')) 
                   ->execute()
                   ->fetchAssoc();
  $places = array();
  foreach ( $ids as $id ) {
    $place = new WorldView_placemark();
    if ( is_array($id) ) {
      $place->load($id['id']);
    } else {
      $place->load($id);
    }
    array_push($places, $place);
  }
  
  // Get places from Databridge if it is installed and activated
  if ( module_exists('DataBridge') && variable_get('worldview_databridge', False)) {
    $places = array_merge($places, WorldView_DataBridge());
  }
  
  return $places;
}

/**
 * Returns module settings to the javascript that load the maps.
 *
 * @return JSON $settings
 *   An array of settings in JSON format.
*/
function WorldView_get_settings() {
  $settings = array('path' => url('worldview/data', array('absolute' => TRUE, )),
                    'mode' => variable_get('worldview_default', 'google_earth'),
                    'zoom' => 7,
                    'lat'  => 0,
                    'lng'  => 38,
                    );
  return drupal_json_output($settings);
}

/**
 * Formats and returns data to the maps. 
 *
 * The map data is returned in KML or GML format depending on what kind of 
 * mapping is used. Both these formats are XML based.
*/
function WorldView_data() {
  $places = WorldView_get_places();
  
//  eval('$data = WorldView_' . variable_get('worldview_default', 'google_earth') . '_data($places)');
  print WorldView_google_earth_data($places);

  exit(0);
}
