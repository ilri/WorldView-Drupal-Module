
<!-- Old code! -->

var worldview_ge;
google.load('earth', '1');

function worldViewInit() {
   google.earth.createInstance('WorldView_map', worldViewInitCB, worldViewFailureCB);
}

function worldViewInitCB(instance) {
   worldview_ge = instance;
  
   worldview_ge.getLayerRoot().enableLayerById(worldview_ge.LAYER_BORDERS, true);
   worldview_ge.getWindow().setVisibility(true);
   
   function finished(object) {
     if (!object) {
       // wrap alerts in API callbacks and event handlers
       // in a setTimeout to prevent deadlock in some browsers
       setTimeout(function() {
         alert('Bad or null KML.');
       }, 0);
       return;
     }
     worldview_ge.getFeatures().appendChild(object);
     // fly to Kenya
     var center = worldview_ge.createLookAt('');
     center.set(-0.5, 38,
       0, // altitude
       worldview_ge.ALTITUDE_RELATIVE_TO_GROUND,
       0, // heading
       20, // straight-down tilt
       1300000 // range (inverse of zoom)
       );
     worldview_ge.getView().setAbstractView(center);
   }
   // fetch the KML
   var url = 'http://localhost/drupal-7.0/?q=worldview/ILRI';
   google.earth.fetchKml(worldview_ge, url, finished);
}

function worldViewFailureCB(errorCode) {
  if (errorCode == 'ERR_CREATE_PLUGIN') {
    worldview_ge.getWindow().setVisibility(false);
  } else {
    alert('There was an unknown error with error code: ' + errorCode + '. Please report this issue to the site admin.');
  }
}

<!-- new code! -->

(function ($) {

  Drupal.behaviors.WorldView = {
    attach: function(context, settings) {
      if ($('#WorldView_map').length) {
        google.setOnLoadCallback(worldViewInit);
      }
    }
  };

})(jQuery);