<?php
// $Id$

/**
 * @file
 *   Constants used by the WorldView module
 *
 *   This include file implements the defaults for the WorldView module.
 */

/**
 * The KML indentation
 */
define('KML_INDENTATION', "\t");

/**
 * Icons directory to be used
 */
define('WORLDVIEW_ICONS', DRUPAL_ROOT . '/' . drupal_get_path('module', 'WorldView') . '/icons');

/**
 * Prints an indentation block
 *
 * @param int $indentation
 *   The indentation value for the block
 */
function _WorldView_indent($indentation = 0) {
  $output = '';
  for ($i = 0; $i < $indentation; $i++) {
    $output .= KML_INDENTATION;
  }
  return $output;
}


/**
 * Turns an array into KML that can be read by Google Earth
 * 
 * @param array $input
 *   An associative array of KML values to be converted
 * @param int $indentation
 *   Indentation value for proper formatting
 *
 * @return string $KML
 *   A formatted KML string
 */
function _WorldView_print_KML($input = array(), $indentation = 0) {
  $output = '';
  foreach ($input as $key => $value) {
    $output .= _WorldView_indent($indentation);
    $output .= '<' . $key . '>';
    if (is_array($value)) {
      $output .= "\n" . _WorldView_print_KML($value, ++$indentation);
      $output .= _WorldView_indent(--$indentation);
    } else {
      if (strstr($value, "\n")) {
        foreach(explode("\n", $value) as $line) {
          $output .= "\n" . _WorldView_indent($indentation) . $line;;
        }
		$output .=  "\n" . _WorldView_indent($indentation);
      } else {
        $output .= $value;
      }
    }
    if (strpos($key, ' ')) {
      $output .= '</' . substr($key, 0, strpos($key, ' ')) . '>' . "\n";
    } else {
      $output .= '</' . $key . '>' . "\n";
    }
  }
  return $output;
}

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
function _WorldView_Icons(&$dom, $icon = 'default') {
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
  $restIconNode->appendChild($restHref);
  $restIconstyleNode->appendChild($restIconNode);
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
  
  $placeNode->setAttribute('id', 'placemark' . $name);
  $nameNode = $dom->createElement('name', $name);
  $placeNode->appendChild($nameNode);
  $descNode = $dom->createElement('description', $desc);
  $placeNode->appendChild($descNode);
  $styleUrl = $dom->createElement('styleUrl', '#' . $icon . 'Style');
  $placeNode->appendChild($styleUrl);
  
  $pointNode = $dom->createElement('Point');
  $placeNode->appendChild($pointNode);
  
  $coorStr = $lat . ',' . $lon;
  $coorNode = $dom->createElement('coordinates', $coorStr);
  $pointNode->appendChild($coorNode);
  return $placeNode;
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
  return _WorldView_Placemark(&$dom, 'ILRI', 'the place to be', '36.722287', '-1.2686', $icon);
}


/**
 * Creates a placemark for each sampling date
 *
 * @param DOMDocument &$dom
 *   The XML generator
 * @param string $icon
 *   Name of the icon png to be used.
 *
 * @return array DOMElement $placeNode
 *   An array containing DOMElements with sampling placemarks
 *
*/
function _WorldView_SamplingTrips(&$dom, $icon = "jeep") {
  $query = "SELECT DATE_FORMAT(s.VisitDate, '%b %d %Y') AS date, 
              s.origin,
              s.Latitude,
              s.Longitude
            FROM (
              SELECT DISTINCT AnimalID AS AnimalID, 
                VisitDate, 
                origin, 
                count, 
                Latitude, 
                Longitude
              FROM samples
                ) s
              JOIN custom_animals ca ON s.AnimalID = ca.animal_id 
              JOIN elisaTest et ON et.sampleID = s.count 
            GROUP BY date;";
  $samplingTrips = array();
  $conn = _WorldView_external_db(variable_get('worldview_db_host', 'localhost'), 
                                 variable_get('worldview_db_user', ''),
                                 variable_get('worldview_db_pass', ''),
                                 variable_get('worldview_db_name', ''));
  if (!$conn['error']) {
    $results = mysql_query($query);
    foreach($results as $result) {
      array_push($samplingTrips, _WorldView_Placemark(&$dom, $result[0], $result[1], $result[2], $result[3], $icon));
    }
  } else {
    throw new Exception($conn['error']); 
  }
  return $samplingTrips;
}