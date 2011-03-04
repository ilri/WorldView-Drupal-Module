<!-- Old code! -->

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
     kenya.set(-4, 38, 900000, worldview_map.ALTITUDE_RELATIVE_TO_GROUND, 
                0, 20, 0 );
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
  
  var worldView_KML = new google.maps.KmlLayer(worldview_url);
  worldView_KML.setMap(worldview_map);
  worldview_map = new google.maps.Map(document.getElementById("WorldView_map"), options);
}


<!-- new code! -->

function worldViewFailure(errorCode) {
  if (errorCode == 'ERR_CREATE_PLUGIN') {
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
