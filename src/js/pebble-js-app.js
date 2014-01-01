//Created by Ben Oxley - www.benoxley.com - www.github.com/ben-oxley
//December 2013
//This project is an implementation of a live-updating tide watch face
//for the Pebble watch developed originally for v1.12 and upgraded to a full
//working version for v2.0 software.

//Good example project at github:
//https://github.com/pebble-hacks/pebble-faces/blob/master/src/js/pebble-js-app.js
//also here: http://developer.getpebble.com/2/guides/javascript-guide.html
//and here: https://developer.getpebble.com/blog/2013/12/20/Pebble-Javascript-Tips-and-Tricks/
var response;
var req;
var tideHeight, timeNow, city;
var fTideHeight, iTideHeight;
var i, j;


function fetchWeather(latitude, longitude) {
  req = new XMLHttpRequest();
  console.log("open");
  latitude = 37.8;
  longitude = -122;
  FEET_TO_MM = 305;
  //http://api.wunderground.com/api/f6d48f234e3e919c/tide/q/CA/San_Francisco.json
  req.open('GET', "http://api.wunderground.com/api/f6d48f234e3e919c/tide/q/"
  + latitude + "," + longitude + ".json", true); //CA/San_Francisco
  //an example file: http://api.wunderground.com/api/f6d48f234e3e919c/tide/q/37.8,-122.json
  req.onload = function(e) {
    if (req.readyState == 4) {
      if(req.status == 200) {
        //console.log(req.responseText);
        response = JSON.parse(req.responseText);
        j = 0;
        console.log("if");
        if (response && response.tide.tideSummary && response.tide.tideSummary.length > 0) {
          sendBytes(0);
        }
      } else {
        console.log("Error");
      }
    }
  }
  req.send(null);
}


function sendBytes(startpos) {
  console.log("Starting Transmission");
  i = startpos;
  if (startpos < response.tide.tideSummary.length) {
    for (i = startpos; i < response.tide.tideSummary.length; i++) {
      var weatherResult = response.tide.tideSummary[i];
      tideHeight = weatherResult.data.height;
      timeNow = Number(weatherResult.utcdate.epoch);
      //timeNow = "1";
      city = weatherResult.date.tzname;
      if (tideHeight !== "") {
        j++;
        if (j > 4) break; //Only send four for now
        //TODO - Clean data on success
        fTideHeight = parseFloat(tideHeight.substr(0,tideHeight.search(" ft")));
        fTideHeight *= FEET_TO_MM;
        iTideHeight = Math.round(fTideHeight);
        var dt = new Date(timeNow*1000);
        console.log(dt);
        console.log(dt.getSeconds() + (60 * dt.getMinutes()) + (60 * 60 * dt.getHours()));
        console.log(iTideHeight);
        console.log(tideHeight);
        console.log(timeNow);
        console.log(city);
        console.log(j-1);
        //timeNow = dt.getSeconds() + (60 * dt.getMinutes()) + (60 * 60 * dt.getHours());
        Pebble.sendAppMessage({
          "position":j-1,
          "timeNow":timeNow,
          "tideHeight":iTideHeight + " ",
          "city":city
        },
          function(e) {
            console.log("Successfully delivered message with transactionId="
              + e.data.transactionId);
            sendBytes(i+1);
          },
          function(e) {
            console.log("Unable to deliver message with transactionId="
              + e.data.transactionId
              + " Error is: " + e.error.message);
          }
          )
        break;
      }
    }
  }
}

function sendFail(error) {

  console.warn('error in sending packets');
//TODO - clean data on fail
}

function locationSuccess(pos) {
  var coordinates = pos.coords;
  fetchWeather(coordinates.latitude, coordinates.longitude);
}

function locationError(err) {
  console.warn('location error (' + err.code + '): ' + err.message);
  //Pebble.sendAppMessage({
  //  "city":"Loc Unavailable",
  //  "tideHeight":"N/A"
  //});
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


