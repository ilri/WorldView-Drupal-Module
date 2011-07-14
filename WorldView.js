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
 * This file contains the javascripts to load and initialize the Google Earth 
 * or Google Maps window on the website. The scripts are in the process of 
 * being re-written and more comments and explanations will be added as soon as
 * that is done.
*/

var worldview_map;
var worldview_url  = '';
google.load('earth', '1');

function worldViewInit(instance) {
   worldview_map = instance;
   worldview_map.getLayerRoot().enableLayerById(worldview_map.LAYER_BORDERS, true);
   worldview_map.getWindow().setVisibility(true);
   
   function finished(object) {
     if (!object) {
       // wrap alerts in API callbacks and event handlers
       // in a setTimeout to prevent deadlock in some browsers
       setTimeout(function() {
         alert('Bad or null KML.');
       }, 0);       
       return;
     }
     worldview_map.getFeatures().appendChild(object);
     // fly to Kenya
     var kenya = worldview_map.createLookAt('');
     kenya.set(0, 38, 950000, worldview_map.ALTITUDE_RELATIVE_TO_GROUND, 
                0, 0, 0 );
     worldview_map.getView().setAbstractView(kenya);
   }
   // fetch the KML
   google.earth.fetchKml(worldview_map, worldview_url, finished);
}

function worldViewFailure(errorCode) {
  if (errorCode == 'ERR_CREATE_PLUGIN') {
    document.getElementById('WorldView_tou').innerHTML += '<br>To view all features of this page download the <a href="http://www.google.com/earth/explore/products/plugin.html">Google Earth Plugin</a>.';
    
    <!-- Google Maps Fallback -->
    worldView_load_google_maps();
  } else {
    alert('There was an unknown error with error code: '
          + errorCode
          + '. Please report this issue to the site admin.');
  }
}

function worldView_load_google_maps(path, mapZoom, lat, lng) {
  
  // Set map variables, I like the Hybrid map so it's hard coded.
  var mapCenter = new google.maps.LatLng(lat, lng);
  var options = {
    zoom: mapZoom,
    center: mapCenter,
    mapTypeId: google.maps.MapTypeId.TERRAIN
  };
  
  // Create the map in our div.
  worldview_map = new google.maps.Map(document.getElementById("WorldView_map"), options);
  
  // Load the data and add it to our map.
  var worldView_GML = new google.maps.KmlLayer(path);
  worldView_GML.setMap(worldview_map);
}

function worldView_load_google_earth(path, mapZoom, lat, lng) {
  worldview_url = path;
  google.setOnLoadCallback(google.earth.createInstance('WorldView_map', 
                                                       worldViewInit, 
                                                       worldViewFailure)
                          );
}

(function ($) {

  Drupal.behaviors.WorldView = {
    attach: function(context, settings) {
      
      // Only load our map scripts if there is a map div on the page
      if ($('#WorldView_map').length) {
        
        // Load settings from drupal
        $.get('?q=worldview/settings', function(data) {
          
          // Init the map using the mode from drupal
          eval('worldView_load_' + data.mode + '("' + data.path  + '", ' 
                                                    + data.zoom + ', ' + 
                                                    + data.lat  + ', ' + 
                                                    + data.lng  + ')');     
        }, "json");
      }
    }
  };
  
})(jQuery);
