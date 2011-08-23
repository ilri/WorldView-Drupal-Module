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
 *   Database functions and constants used by the WorldView module
 *
 *   This include file implements a link to extract information from the 
 *   internal Drupal database. 
*/

/**
 * Loads an entry from the database or returns an empty entry.
 *
 * @param int $id = NULL
 *   If set, it is the database id to extract.
 *
 * @return array $entry
 *   An associative array of information.
*/
function WorldView_load($id = NULL) {
  if ( $id ) {
    try {
      $event = db_select('worldview_places', 't')
                          ->fields('t') // Equivalent of "SELECT * FROM"
                          ->condition('id',  $id , '=')
                          ->execute();
      return $event->fetchAssoc();
    }
    catch (Exception $e) {
      drupal_set_message($e, 'error');
    }
  }
  return array('latitude'    => '',
               'longitude'   => '',
               'title'       => '',
               'link'        => '',
               'information' => '',
               'start_date'  => '',
               'end_date'    => '',
               'icon'        => '');
}

/**
 * Inserts a map entry into the internal database.
 *
 * @param array $entry
 *   An associative array of fields to insert
*/
function WorldView_insert($entry) {
  $trn = db_transaction();
  global $user;
  
  $fields = array('created_by' => $user->name,
                  'created_at' => time(),);
                  
  $field_names = array('latitude', 'longitude', 'title', 'link', 'information', 'start_date', 'end_date', 'icon');
  
  foreach ($field_names as $field) {
    $fields[$field] = (isset($entry[$field]) and $entry[$field]) ? $entry[$field] : NULL;
  }
  
  try {
    db_insert('worldview_places')
              ->fields($fields)
              ->execute();
  }
  catch (Exception $e) {
    $trn->rollback();
    watchdog_exception('type', $e);
    return False;
  }
  return True;
}

/**
 * Updates a map entry in the internal database.
 *
 * @param array $entry
 *   An associative array of fields to update
*/
function WorldView_update($entry) {
  $trn = db_transaction();
  global $user;
  
  $fields = array('updated_by' => $user->name,
                  'updated_at' => time(),);
                  
  $field_names = array('latitude', 'longitude', 'title', 'link', 'information', 'start_date', 'end_date', 'icon');
  
  foreach ($field_names as $field) {
    $fields[$field] = (isset($entry[$field]) and $entry[$field]) ? $entry[$field] : NULL;
  }
  
  try {
    db_update('worldview_places')
              ->fields($fields)
              ->condition('id', $entry['id'], '=')
              ->execute();
  }
  catch (Exception $e) {
    $trn->rollback();
    watchdog_exception('type', $e);
    return False;
  }
  return True;
}
