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
var worldview_path = window.location.hostname + window.location.pathname;
var worldview_url  = 'http://' + worldview_path + '?q=worldview';
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

<!-- Google Maps Fallback -->

function worldViewInitGM() {
  var kenya = new google.maps.LatLng(0, 38);
  var options = {
    zoom: 6,
    center: kenya,
    mapTypeId: google.maps.MapTypeId.HYBRID
  };
  worldview_map = new google.maps.Map(document.getElementById("WorldView_map"), options);
  
  var worldView_KML = new google.maps.KmlLayer(worldview_url);
  worldView_KML.setMap(worldview_map);
}

function worldViewFailure(errorCode) {
  if (errorCode == 'ERR_CREATE_PLUGIN') {
    document.getElementById('WorldView_tou').innerHTML += '<br>To view all features of this page download the <a href="http://www.google.com/earth/explore/products/plugin.html">Google Earth Plugin</a>.';
    worldViewInitGM();
  } else {
    alert('There was an unknown error with error code: '
          + errorCode
          + '. Please report this issue to the site admin.');
  }
}

(function ($) {

  Drupal.behaviors.WorldView = {
    attach: function(context, settings) {
      if ($('#WorldView_map').length) {
        google.setOnLoadCallback(google.earth.createInstance('WorldView_map', 
                                                             worldViewInit, 
                                                             worldViewFailure)
                                );
      }
    }
  };
  
})(jQuery);
