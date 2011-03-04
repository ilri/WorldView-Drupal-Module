<!-- Old code! -->

var worldview_ge;
google.load('earth', '1');

function worldViewInit(instance) {
   worldview_ge = instance;
   
   worldview_ge.getLayerRoot().enableLayerById(worldview_ge.LAYER_BORDERS,
                                               true);
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
     var kenya = worldview_ge.createLookAt('');
     kenya.set(-4, 38, 900000, worldview_ge.ALTITUDE_RELATIVE_TO_GROUND, 
                0, 20, 0 );
     worldview_ge.getView().setAbstractView(kenya);
   }
   // fetch the KML
   var url = 'http://localhost/drupal-7.0/?q=worldview/ILRI';
   google.earth.fetchKml(worldview_ge, url, finished);
}


<!-- new code! -->

function worldViewFailure(errorCode) {
  if (errorCode == 'ERR_CREATE_PLUGIN') {
    worldview_ge.getWindow().setVisibility(false);
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
