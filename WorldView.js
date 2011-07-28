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
var placemarks = new Array();
var infoWindows = new Array();
var worldview_url  = '';
google.load('earth', '1');
var worldview_zoom;
var worldview_lat;
var worldview_lng;

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
     worldview_map.getView().setAbstractView(worldview_map_center);
   }
   
   // Set Map Center
   var worldview_map_center = worldview_map.createLookAt('');
   worldview_map_center.set(worldview_lat, worldview_lng, 950000, worldview_map.ALTITUDE_RELATIVE_TO_GROUND, 0, 0, 0 );
   
   // fetch the KML
   google.earth.fetchKml(worldview_map, worldview_url, finished);
}

function worldViewFailure(errorCode) {
  if (errorCode == 'ERR_CREATE_PLUGIN') {
    document.getElementById('WorldView_tou').innerHTML += '<br>To view all features of this page download the <a href="http://www.google.com/earth/explore/products/plugin.html">Google Earth Plugin</a>.';
    
    <!-- Google Maps Fallback -->
    worldView_load_google_maps(worldview_url, worldview_zoom, worldview_lat, worldview_lng);
  } else {
    alert('There was an unknown error with error code: '
          + errorCode
          + '. Please report this issue to the site admin.');
  }
}

function worldView_load_google_maps(path, mapZoom, lat, lng) {
  
  // Set map variables, I like the Terrain map so it's hard coded.
  var mapCenter = new google.maps.LatLng(lat, lng);
  var options = {
    zoom: mapZoom,
    center: mapCenter,
    mapTypeId: google.maps.MapTypeId.TERRAIN
  };
  
  // Create the map in our div.
  worldview_map = new google.maps.Map(document.getElementById("WorldView_map"), options);
  
  (function ($) {
    // Load the data from the database
    $.get(path + "/google_maps", function(data) {
      
      // Parse the data we got into a DOM
      parser=new DOMParser();
      var xml = parser.parseFromString(data,"text/xml");
      
      var markers = xml.documentElement.getElementsByTagName("marker");
      var temp = "";
      for (var i = 0; i < markers.length; i++) {
        // list variables for debugging
        if (!temp) {
          for (var j = 0; j < markers[i].attributes.length; j++) {
            temp += markers[i].attributes[j].name + " => " + markers[i].attributes[j].value + "\n";
          }
          //alert(temp);
        }
        
        var icon = new google.maps.MarkerImage(markers[i].getAttribute("icon"),
            new google.maps.Size(64, 64),
            // The origin for this image is 0,0.
            new google.maps.Point(0,0),
            // The anchor for this image is the middle of the base at 64,32.
            new google.maps.Point(32, 64));
        
        placemarks[i] = new google.maps.Marker(
          { position: new google.maps.LatLng(markers[i].getAttribute("latitude"), 
                                             markers[i].getAttribute("longitude")),
            map: worldview_map,
            title: markers[i].getAttribute("title"),
            animation: google.maps.Animation.DROP,
            icon: icon
          } );
        
        // Format dates for the info window
        var dates = "";
        if ( markers[i].getAttribute("start_date") ) {
          dates += markers[i].getAttribute("start_date");
        };
        if ( markers[i].getAttribute("end_date") ) {
          dates += " to " + markers[i].getAttribute("end_date");
        };
        
        // Format the placemark user info
        var user_info = "";
        if ( markers[i].getAttribute("created_by") ) {
          user_info += "Created by: " + markers[i].getAttribute("created_by") +
                       " at " + markers[i].getAttribute("created_at");
        }
        if ( markers[i].getAttribute("updated_by") ) {
          user_info += "Updated by: " + markers[i].getAttribute("updated_by") +
                       " at " + markers[i].getAttribute("updated_at");
        }
        
        // Create the final content string
        contentString = "<h3>" + markers[i].getAttribute("title") + "</h3>" + 
                        "<hr>" + 
                        "<small>" + dates + "</small>" + 
                        markers[i].getAttribute("information") + 
                        "<hr>" + 
                        markers[i].getAttribute("link") + "<br>" + 
                        "<small>" + user_info + "</small>"; 
        
        infoWindows[i] = new google.maps.InfoWindow(
          { content: contentString
          } );
        
        google.maps.event.addListener(placemarks[i], 'click', 
          function() {
            showInfo(this);
          } );
      }
      function showInfo( placemark ) {
        for (var i = 0; i < placemarks.length; ++i ) {
          if ( placemarks[i] == placemark ) {
            infoWindows[i].open(worldview_map, placemarks[i]);
            break;
          }
        }
      }
      
    });
  })(jQuery);
}

function worldView_load_google_earth(path, mapZoom, lat, lng) {
  worldview_url = path;
  worldview_zoom = mapZoom;
  worldview_lat = lat;
  worldview_lng = lng;
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
