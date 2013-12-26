function fetchWeather(latitude, longitude) {
  var response;
  var req = new XMLHttpRequest();
  //http://api.wunderground.com/api/f6d48f234e3e919c/tide/q/CA/San_Francisco.json
  req.open('GET', "http://api.wunderground.com/api/f6d48f234e3e919c/tide/q/"
  + latitude + "," + longitude + ".json", true); //CA/San_Francisco
  req.onload = function(e) {
    if (req.readyState == 4) {
      if(req.status == 200) {
        console.log(req.responseText);
        response = JSON.parse(req.responseText);
        var tideHeight, timeNow, city;
        var i, j;
        if (response && response.tide.tideSummary && response.tide.tideSummary.length > 0) {
          for (i = 0; i < response.tide.tideSummary.length; i++) {
            var weatherResult = response.tide.tideSummary[i];
            tideHeight = weatherResult.data.height;
            timeNow = Math.int(weatherResult.utcdate.epoch);
            timeNow = "1";
            city = weatherResult.date.tzname;
            if (tideHeight != "") {
              console.log(tideHeight);
              console.log(timeNow);
              console.log(city);
              Pebble.sendAppMessage({
                "timeNow":timeNow,
                "tideHeight":tideHeight + "\u00B0C",
                "city":city});
          }
        }

      } else {
        console.log("Error");
      }
    }
  }
  req.send(null);
}

function locationSuccess(pos) {
  var coordinates = pos.coords;
  fetchWeather(coordinates.latitude, coordinates.longitude);
}

function locationError(err) {
  console.warn('location error (' + err.code + '): ' + err.message);
  Pebble.sendAppMessage({
    "city":"Loc Unavailable",
    "tideHeight":"N/A"
  });
}

var locationOptions = { "timeout": 15000, "maximumAge": 60000 }; 


Pebble.addEventListener("ready",
                        function(e) {
                          console.log("connect!" + e.ready);
                          locationWatcher = window.navigator.geolocation.watchPosition(locationSuccess, locationError, locationOptions);
                          console.log(e.type);
                        });

Pebble.addEventListener("appmessage",
                        function(e) {
                          window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
                          console.log(e.type);
                          console.log(e.payload.temperature);
                          console.log("message!");
                        });

Pebble.addEventListener("webviewclosed",
                                     function(e) {
                                     console.log("webview closed");
                                     console.log(e.type);
                                     console.log(e.response);
                                     });


