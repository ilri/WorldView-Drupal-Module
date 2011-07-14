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
 *   DataBridge functions and constants used by the WorldView module
 *
 *   This include file implements a link to extract information directly from
 *   the DataBridge module and plotting them on the WorldView map. 
*/

/**
 * If installed this function gets information from the databridge module.
 *
 * If the DataBridge module is installed and activated in the configuration 
 * page, this function will extract sampling trips and plot them on the map. 
 * Note that all samplings will attempt to use a map icon with the same name 
 * as their sampling class. i.e. "livestock" samplings will attempt to use 
 * "livestock.png" as icon.
 *
 * @return array $places
 *   An array of WorldView_placemarks with sampling trip information.
 *
*/
function WorldView_DataBridge() {
  $placemarks = array();
  if (module_exists('DataBridge')) {
    $places = array();
    
    foreach ( DataBridge_getSamplingInfo() as $class => $trips ) {
      
      foreach ( $trips as $trip ) {
        $place = new WorldView_placemark($trip['latitude'],  // latitude
                                         $trip['longitude'], // longitude
                                         ucfirst($class) . ' sampling in ' . $trip['location'], // title
                                         theme('table', DataBridge_table($trip)), // information
                                         array(array('Download Summary', 
                                                     'databridge/sampling/' . $class . '/' . 
                                                     $trip['location'] . '/' . 
                                                     strftime('%Y-%m-%d', $trip['start_date']) . '/xls'), 
                                               array('View Details', 'databridge/sampling/' . $class . '/' .
                                                     $trip['location'] . '/' . 
                                                     strftime('%Y-%m-%d', $trip['start_date'])), 
                                              ), // link
                                         $trip['start_date'], // start_date
                                         $trip['end_date'], // end_date
                                         $class);
        array_push($places, $place);
      }

    }
  }
  return $places;
}
