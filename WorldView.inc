<?php
// $Id$

/**
 * @file
 *   Functions and constants used by the WorldView module
 *
 *   This include file implements the defaults for the WorldView module.
*/

/**
 * Icons directory to be used
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
 * Returns a HTML table from an array
 * 
 * Creates a HTML table from an array formatted as array[rows][cols], note that
 * the width of the table will be calculated from the first row so make sure to
 * fill the first row with blank columns if the table needs to be wider than 
 * the first row.
 *
 * @param string title
 *   Title string of the table
 * @param array table
 *   An array of table values formatted as array[rows][cols]
 *
 * @return string html
 *   A HTML string containing the finished table
*/
function _WorldView_Table($title = '', $table = array(array(''))) {
  $cols = count($table[0]);
  $html = '<table><tr><th colspan=' . $cols . '>' . $title . '</td></tr>';
  foreach ($table as $row) {
    $html .= '<tr>';
    foreach ($row as $col) {
      $html .= '<td>' . $col . '</td>';
    }
    $html .= '</tr>';
  }
  return $html . '</table>';
}

/**
 * TEMPORARY FUNCTIONS - REMOVE THESE OR AT LEAST DON'T RELY ON THEM
*/

/**
 * Creates a placemark for ILRI
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

/**
 * Creates a placemark for each Livestock sampling date and origin
 *
 * @param DOMDocument &$dom
 *   The XML generator
 * @param string $icon
 *   Name of the icon png to be used.
 *
 * @return array DOMElement $placeNode
 *   An array containing DOMElements with sampling placemarks
*/
function _WorldView_SamplingTrips(&$dom, $icon = "jeep") {
  $query = "SELECT DATE_FORMAT(s.VisitDate, '%b %d %Y') AS date, 
                   s.origin,
                   s.Latitude,
                   s.Longitude,
                   SUM(IF(s.project = 5 AND s.AnimalID LIKE 'AVD%', 1, 0)) AS sampled_animals, 
                   SUM(IF(ca.organism = 2, 1, 0)) AS sampled_goats, 
                   SUM(IF(ca.organism = 2 AND ca.sex = 'Female', 1, 0)) AS female_goats, 
                   SUM(IF(ca.organism = 3 AND ca.sex = 'Female', 1, 0)) AS female_sheep, 
                   SUM(IF(ca.organism = 2 AND ca.Status = 'Alive', 1, 0)) AS goats_alive, 
                   SUM(IF(ca.organism = 2 AND ca.Status = 'Dead', 1, 0)) AS goats_dead, 
                   SUM(IF(ca.organism = 3 AND ca.Status = 'Alive', 1, 0)) AS sheep_alive, 
                   SUM(IF(ca.organism = 3 AND ca.Status = 'Dead', 1, 0)) AS sheep_dead, 
                   SUM(IF(ca.organism = 3 AND et.STATUS = 'POSITIVE', 1, 0)) AS sheep_positive, 
                   SUM(IF(ca.organism = 2 AND et.STATUS = 'POSITIVE', 1, 0)) AS goats_positive, 
                   TO_DAYS(s.VisitDate) AS days
            FROM (
                   SELECT AnimalID, 
                          VisitDate, 
                          origin, 
                          project, 
                          count, 
                          Latitude, 
                          Longitude
                    FROM samples
                    GROUP BY AnimalID
                  ) s
                  JOIN custom_animals ca ON s.AnimalID = ca.animal_id 
                  LEFT JOIN elisaTest et ON et.sampleID = s.count 
            WHERE VisitDate AND origin IS NOT NULL AND s.project = 5
            GROUP BY TO_DAYS(date), origin;";
  $samplingTrips = array();
  $conn = _WorldView_external_db(variable_get('worldview_db_host', 'localhost'), 
                                 variable_get('worldview_db_user', ''),
                                 variable_get('worldview_db_pass', ''),
                                 variable_get('worldview_db_name', ''));
  if (!$conn['error']) {
    $results = mysql_query($query, $conn['link']);
    while($result = mysql_fetch_row($results)) {
      $name        = $result[1] . ' on ' . $result[0] . '.';
      $description = _WorldView_Table($result[4] . ' animals sampled', 
                                      array(array('Sheep', $result[7] . ' F', ($result[4] - $result[5] - $result[7]) . ' M'),
                                            array('Goats', $result[6] . ' F', ($result[5] - $result[6]) . ' M')
                                            )
                                      ) .  
                     _WorldView_Table('Animal Status',
                                      array(array('', 'Alive', 'Dead', 'Unacc.'),
                                            array('Sheep', $result[10], $result[11], ($result[4] - $result[5]) - ($result[10] + $result[11])),
                                            array('Goats', $result[8], $result[9], $result[5] - $result[8] - $result[9])
                                            )
                                      ) . 
                     _WorldView_Table('ELISA Results',
                                      array(array('', 'Positive', 'Negative'),
                                            array('Sheep', $result[12], ($result[4] - $result[5]) - $result[12]),
                                            array('Goats', $result[13], $result[5] - $result[13])
                                            )
                                      );
      $latitude    = $result[2];
      $longitude   = $result[3];
      array_push($samplingTrips, _WorldView_Placemark($dom, $name, $description, $latitude, $longitude, $icon));
    }
  } else {
    throw new Exception($conn['error']); 
  }
  return $samplingTrips;
}

/**
 * Creates a placemark for each Wildlife sampling date and origin
 *
 * @param DOMDocument &$dom
 *   The XML generator
 * @param string $icon
 *   Name of the icon png to be used.
 *
 * @return array DOMElement $placeNode
 *   An array containing DOMElements with sampling placemarks
*/
function _WorldView_WildlifeTrips(&$dom, $icon = "jeep") {
  $query = "SELECT DATE_FORMAT(VisitDate, '%b %d %Y') AS date, 
                   origin,
                   Latitude,
                   Longitude,
                   SUM(IF(count, 1, 0)) AS samples
            FROM samples
            WHERE VisitDate AND origin IS NOT NULL AND project = 5
            AND AnimalID NOT IN (SELECT animal_id FROM custom_animals)
            GROUP BY TO_DAYS(date), origin;";
  $samplingTrips = array();
  $conn = _WorldView_external_db(variable_get('worldview_db_host', 'localhost'), 
                                 variable_get('worldview_db_user', ''),
                                 variable_get('worldview_db_pass', ''),
                                 variable_get('worldview_db_name', ''));
  if (!$conn['error']) {
    $results = mysql_query($query, $conn['link']);
    while($result = mysql_fetch_row($results)) {
      $name        = $result[1] . ' on ' . $result[0] . '.';
      $description = 'Wildlife sampling trip.<hr/> ' . $result[4] . ' samples collected.';
      $latitude    = $result[2];
      $longitude   = $result[3];
      array_push($samplingTrips, _WorldView_Placemark($dom, $name, $description, $latitude, $longitude, $icon));
    }
  } else {
    throw new Exception($conn['error']); 
  }
  return $samplingTrips;
}
