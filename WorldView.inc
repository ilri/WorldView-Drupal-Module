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
                   ->execute();
  $places = array();
  while ( $id = $ids->fetchAssoc() ) {
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
                    'zoom' => variable_get('worldview_zoom', 7),
                    'lat'  => variable_get('worldview_lat', 0),
                    'lng'  => variable_get('worldview_lon', 38),
                    );
  return drupal_json_output($settings);
}

/**
 * Formats and returns data to the maps. 
 *
 * The map data is returned in KML or GML format depending on what kind of 
 * mapping is used. Both these formats are XML based.
*/
function WorldView_data($source = NULL) {
  $places = WorldView_get_places();
  $source = $source ? $source : variable_get('worldview_default', 'google_earth');
  
  eval('echo WorldView_' . $source . '_data($places);');

  exit(0);
}

/**
 * Returns a form to be used by the WorldView_edit function.
 *
 * @param array $form
 *   The form that is to be generated, supplied by drupal
 * @param array &$form_state
 *   The current state of the form, supplied by drupal
 * @param array $entry
 *   Contains the form fields.
 *
 * @return array $form
 *   A drupal form.
*/
function WorldView_input($form, &$form_state, $entry) {
  // Set id to define if we're editing or creating a new entry
  $form['id'] =
    array('#type' => 'hidden',
    '#value' => isset($entry['id']) ? $entry['id'] : NULL,
    );
  
  // fieldset for position information
  $form['worldview_position'] = array(
    '#type' => 'fieldset',
    '#title' => t('Position'),
    '#collapsible' => TRUE,
  );
  $form['worldview_position']['latitude'] = 
    array('#type' => 'textfield',
    '#prefix' => '<table><tr><td>',
    '#suffix' => '</td>',
	  '#title' => t('Latitude'),
	  '#size' => 20,
	  '#maxlength' => 30,
	  '#description' => t("Latitude part of the position in Decimal Degrees."),
	  '#required' => TRUE,
	  '#default_value' => $entry['latitude']
	  );
  $form['worldview_position']['longitude'] = 
    array('#type' => 'textfield',
    '#prefix' => '<td>',
    '#suffix' => '</td></tr></table>',
    '#title' => t('Longitude'),
    '#size' => 20,
    '#maxlength' => 30,
    '#description' => t("Longitude part of the position in Decimal Degrees."),
    '#required' => TRUE,
    '#default_value' => $entry['longitude']
    );
  
  // fieldset for general information
  $form['worldview_general'] = array(
    '#type' => 'fieldset',
    '#title' => t('General Information'),
    '#collapsible' => TRUE,
  );
  $form['worldview_general']['title'] = 
    array('#type' => 'textfield',
    '#prefix' => '<table><tr><td>',
    '#suffix' => '</td>',
    '#title' => t('Title'),
    '#size' => 50,
    '#maxlength' => 50,
    '#description' => t("Title of the map entry."),
    '#required' => TRUE,
    '#default_value' => $entry['title']
    );
  $form['worldview_general']['link'] = 
    array('#type' => 'textfield',
    '#prefix' => '<td>',
    '#suffix' => '</td>',
    '#title' => t('Link'),
    '#size' => 50,
    '#maxlength' => 50,
    '#description' => t("Link to be placed on the entry."),
    '#required' => FALSE,
    '#default_value' => $entry['link']
    );
  $form['worldview_general']['icon'] = 
    array('#type' => 'textfield',
    '#prefix' => '<td>',
    '#suffix' => '</td></tr>',
    '#title' => t('Icon'),
    '#size' => 25,
    '#maxlength' => 25,
    '#description' => t("Icon to be placed on the map."),
    '#required' => FALSE,
    '#default_value' => $entry['icon']
    );
  $form['worldview_general']['information'] = 
    array('#type' => 'textarea',
    '#prefix' => '<tr><td colspan="3">',
    '#suffix' => '</td></tr></table>',
    '#title' => t('Information'),
    '#description' => t("The main content of the entry."),
    '#required' => FALSE,
    '#default_value' => $entry['information']
    );

  // fieldset for time information
  $form['worldview_time'] = array(
    '#type' => 'fieldset',
    '#title' => t('Time'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['worldview_time']['start_date'] = 
    array('#type' => 'textfield',
    '#prefix' => '<table><tr><td>',
    '#suffix' => '</td>',
    '#title' => t('Start Date'),
    '#size' => 20,
    '#maxlength' => 30,
    '#description' => t("Start date and time formatted as 'YYYY-MM-DD HH:MM'."),
    '#required' => FALSE,
    '#default_value' => $entry['start_date']
    );
  $form['worldview_time']['end_date'] = 
    array('#type' => 'textfield',
    '#prefix' => '<td>',
    '#suffix' => '</td></tr></table>',
    '#title' => t('End date'),
    '#size' => 20,
    '#maxlength' => 30,
    '#description' => t("End date and time formatted as 'YYYY-MM-DD HH:MM'."),
    '#required' => FALSE,
    '#default_value' => $entry['end_date']
    );
	
	// SUBMIT!!
	$form['submit'] = 
	  array('#type' => 'submit',
	  '#value' => 'Submit form',
    '#title' => t('Submit.'),
	   
	  );
	
	// return the form
	return $form;
}

/*
 * Validates the input from WorldView_input before it is saved to the database.
 * 
 * This function makes sure that all dates are in the expected format, that all 
 * required fields are supplied and so on.
 *
 * @param array $form
 *   The form that is to be generated, supplied by drupal
 * @param array &$form_state
 *   The current state of the form, supplied by drupal
*/
function WorldView_input_validate(&$form, &$form_state) {
}

/*
 * Submits the input from WorldView_input to the internal database.
 *
 * @param array $form
 *   The form that is to be generated, supplied by drupal
 * @param array &$form_state
 *   The current state of the form, supplied by drupal
*/
function WorldView_input_submit(&$form, &$form_state) {
  if ( !$form_state['values']['id'] ) {
    WorldView_insert($form_state['values']);
  }
  else {
    WorldView_update($form_state['values']);
  }
}
