# Configuration File Format

JSON format with various sections
- station data
- feature switches
- URLs
- access parameters

## example
    {
        "configuration":{               // The configuration object
          "stationId": "FD0017",                // textual ID - 6 characters - does not need to match the FANET-ID
          "stationName": "MRFGSPI17",           // name - char[9] (max)
          "manufacturerId":"0xFC",              // FANET manufacturer ID
          "uniqueId":"0x0017",                  // FANET unique ID
          "position":{                          // for reporting to OGN and distance calculation
            "latitude":49.21,                   // [decimal degrees]
            "longitude":8.37,                   // [decimal degrees]
            "altitude":117                      // [m]
          },
          "hwinfo":{                            // for type 8 packets
            "active": true,
            "type": 17,
            "year":2020,
            "month":12,
            "day": 6,
            "experimental": true,
            "add1": 42,
            "add2": 17
          },
          "features":{                          // feature switches
          
            "fanetRadio":{
              "active":true,
              "interval": 1,                    // [msec]
              "requireValidBit": true,
              "requireValidCRC": true
            },
            "namePublishing":{                  // publish entity names
              "active":true,
              "interval":15                     // [sec] - pause between packets
            },
            "trackPushing":{                    // forward received FANET tracks
              "active":true,
              "interval":3000                   // [msec]
            },
            "trackRelaying":{                   // relay 'other' received tracks to FANET
              "active": true,
              "excludeFanetPlus": true          // exclude FANET+ tracks like SykTraxx which are sending FANET & FLARM to vaoid duplicates
            },
            "weatherPolling":{                  // fetch weather from internet weather stations
              "active":true,
              "interval":60                     // [sec]
            },
            "weatherPushing":{                  // send weather info to FANET
              "active":true,
              "interval":60,                    // [sec]
              "pause": 5,                       // [sec] - pause between packets
              "maxAge": 300                     // [sec] - max age of weather data to send out
            },
            "landmarkPushing":{                 // send landmarks
              "active":true,
              "interval":60,                    // [sec]
              "pause": 5                        // [sec] - pause between packets
            },
            "fanetWeatherRelaying":{            // relay FANET wetaher to internet
              "active": true  
            }
          },
          "APRS":{                              // APRS OGN
            "active": true,
            "server":"glidern5.glidernet.org",
            "appName":"MRGFS",
            "appVersion":"0.0.1",
            "user":"APRS-ID",                   // specify your own - max. 9 characters
            "pass":"12345"                      // generate matching APRS passcode
          },
          "CWOP":{                              // APRS CWOP
            "active": true,                     // forward weather data to CWOP server
            "server": "129.15.108.117",
            "appName":"MRFGS",
            "appVersion":"0.0.1",
            "user":"CWOP-ID",                   // specify your own - max. 9 characters
            "pass":"12345"                      // generate matching APRS passcode
          },
          "PacketDB": {                         // host to send FANET packets to
            "active": true,
            "updateHost": "www.tranier.de",     
            "updateUrlPacket" : "/fgs/m/add_fanet_packet.php"
          },
          "TrackDB": {                          // host to send track info to
            "active": true,
            "updateHost": "www.tranier.de",
            "updateUrlTrack" : "/m/add_fanet_track.php",
            "updateUrlDevice" : "/m/add_fanet_device.php",
            "readUrl" : "/m/get_vehicle_pos_recent.php"
          },
          "WeatherDB": {                        // host to send weather info to
            "active": true,
            "updateHost": "www.tranier.de",
            "updateUrl" : "/m/add_fanet_weather.php",
            "readUrl" : "/m/get_fanet_weather_csv.php"
          },
          "WindyUpdate": {                      // Windy API parameters
            "active": false,
            "windyHost": "stations.windy.com",
            "updateUrl": "/pws/update/",
            "updateApiKey": "API-KEY"
          },
          "WindyStatus": {
            "windyHost": "stations.windy.com",
            "statusUrl": "/pws/station/open/"
          },
 
          
          "weatherStations":[                   // Array of weather stations to be handled
            {
              "active":true,                    // flag for processing 
              "type":"HolfuyWeatherStation",    // type string
              "id":"711",                       // Station ID - string
              "name":"Hohenberg",               
              "shortName":"WHohe",              // Name published to FANET - except if set to 'doNotPublish'
              "manufacturerId":"0xFD",          // FANET manuf. ID
              "uniqueId":"0x0711",              // FANEZ unique ID
              "latitude":49.202584,
              "longitude":8.005003,
              "altitude":555,
              "cwopId": "CWHF0711",             // ID for CWOP publishing
              "cwopPush": false,
              "dbPush": true,                   // push to DB server
              "windyId": "5",                   // Windy API station ID
              "windyPush": false                // flag for push to Windy
            },
            {                                   // SkyTraxx FANET Windstation
              "active":true,                    // flag for processing 
              "type":"FanetWeatherStation",     // type string
              "id":"SkyTraxx-011D",
              "name":"wGermersheim",
              "shortName":"doNotPublish",
              "manufacturerId":"0x01",          // SkyTraxx Windstations FANET manuf. ID
              "uniqueId":"0x011D",              // FANET unique ID
              "latitude":49.21,
              "longitude":8.37,
              "altitude":117,
              "cwopId": "WX01011D",             // CWOP ID
              "cwopPush": false,                // currently not pushed to CWOP
              "dbPush": true                    // send to DB server for as basis for chart visualization on website
            }
          ],
          
          
          "fanetNames":[                        // Array of names to be published to FANET
            { 
                "active":false,                 // flag for processing
                "id":"042442",                  // internal ID
                "name":"AirWhere-04-2442",      
                "shortName":"Aw2442",           // name publshed to FANET
                "manufacturerId":"0x04",        // FANET manuf. ID
                "uniqueId":"0x2442"             // FANET unique ID
            }
          ],
          
          
          "landmarks":[                         // Array of Landmark geo information
            {                                           // POI example with text label
              "active":false,
              "type":0, "id":"GER1", "text": "GER1",
              "layer": 0,
              "coordinates": [ {"lat":49.2111, "lon":8.371} ]
            },
            {
              "active":false,
              "type":0, "layer":0, "id":"Hohe-LP", "text": "LP-Hohenberg",
              "coordinates": [ {"lat":49.202584, "lon":8.005003} ]
            },
            {                                   // Orensfels landing site shape
              "active":false,                   // currently not published
              "type":4,
              "id":"LP-Ori",
              "text": "LP-Ori",
              "layer": 4,
              "geometry": {                     // GeoJson format section
                "type": "Polygon",              // use e.g. https://geoman.io/geojson-editor
                "coordinates": [
                  [
                    [8.006759, 49.237874],
                    ...
                    [8.006759, 49.237874]
                  ]
                ]
              }
            },
            
            {
              "active":false,
              "type":0,
              "id":"LP-Hohe",
              "text": "LP-Hohenberg",
              "layer": 1,
              "coordinates": [
                {"lat":49.208423, "lon":7.992736}
              ]
            },
            {                                               // Approach procedure example
              "active":false,                                           // currently inactive
              "type":2, "id":"Hohe-RVA", "text": "Hohe Rechtsvolte",    // right turn
              "layer": 1,
              "geometry": {
                "type": "LineString",
                "coordinates": [
                  [7.994785, 49.209236],
                  ...
                ]
              }
            },
            {
              "active":false,
              "type":5,
              "layer":1,
              "id":"Hohe-RV-Pos", "text": "Hohe Position Rechtsvolte",
              "geometry": {
                "type": "Point",
                "coordinates": [7.99441, 49.20918]
              },
              "diameter": 50
            },
            {                                           // Merkur landing site
              "active": true,
              "type": 4,
              "id": "LP-Merkur",
              "text": "LP-Merkur",
              "layer": 4,
              "geometry": {
                "type": "Polygon",
                "coordinates": [
                  [
                    [8.260149, 48.763983],
                    ...
                    [8.260149, 48.763983]
                  ]
                ]
              }
            }
          ]
        }
      }
