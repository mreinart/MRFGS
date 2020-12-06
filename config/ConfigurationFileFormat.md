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
            "trackPushing":{                    // forward received tracks
              "active":true,
              "interval":3000                   // [msec]
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
            "user":"APRS-ID",
            "pass":"12345"
          },
          "CWOP":{                              // APRS CWOP
            "active": true,                     // forward weather data to CWOP server
            "server": "129.15.108.117",
            "appName":"MRFGS",
            "appVersion":"0.0.1",
            "user":"CWOP-ID",
            "pass":"12345"
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
            {
              "active":true,                    // flag for processing 
              "type":"FanetWeatherStation",     // type string
              "id":"SkyTraxx-011D",
              "name":"wGermersheim",
              "shortName":"doNotPublish",
              "manufacturerId":"0x01",
              "uniqueId":"0x011D",
              "latitude":49.21,
              "longitude":8.37,
              "altitude":117,
              "cwopId": "WX01011D",
              "cwopPush": false,
              "dbPush": true
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
              "geometry": {
                "type": "Polygon",
                "coordinates": [
                  [
                    [8.006759, 49.237874],
                    [8.006426, 49.237678],
                    [8.006405, 49.237356],
                    [8.006705, 49.237342],
                    [8.006587, 49.236536],
                    [8.006748, 49.236459],
                    [8.006995, 49.236466],
                    [8.007349, 49.236466],
                    [8.007596, 49.236487],
                    [8.007778, 49.236676],
                    [8.007982, 49.236655],
                    [8.008014, 49.236921],
                    [8.008282, 49.236879],
                    [8.008207, 49.236606],
                    [8.008990, 49.236627],
                    [8.008969, 49.236851],
                    [8.009570, 49.237096],
                    [8.008862, 49.237412],
                    [8.008261, 49.237286],
                    [8.007896, 49.237650],
                    [8.008615, 49.237776],
                    [8.008368, 49.238112],
                    [8.008186, 49.238315],
                    [8.007510, 49.238701],
                    [8.006759, 49.237874]
                  ]
                ]
              }
            },
            {
              "active":false,
              "type": 4,
              "id":"LP-Hohe",
              "text": "LP-Hohenberg",
              "layer": 1,
              "geometry": {
                "type": "Polygon",
                "coordinates": [
                  [
                    [7.994367, 49.207659],
                    [7.994968, 49.208002],
                    [7.994710, 49.208689],
                    [7.994217, 49.208598],
                    [7.994152, 49.208759],
                    [7.993627, 49.208626],
                    [7.993584, 49.208724],
                    [7.992822, 49.208535],
                    [7.992736, 49.208423],
                    [7.992843, 49.208262],
                    [7.993755, 49.208486],
                    [7.994163, 49.207729],
                    [7.994367, 49.207659]
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
                  [7.995397, 49.207897],
                  [7.994721, 49.207673],
                  [7.994227, 49.208346]
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
            {
              "active":false,
              "type":0,
              "id": "LP-GER",
              "text": "LP-GER-Festplatz",
              "layer": 1,
              "coordinates": [
                {"lat":49.219097, "lon":8.355693}
              ]
            },
            {
              "active": true,
              "type": 4,
              "id": "LP-GER",
              "text": "LP-GER-Festplatz",
              "layer": 4,
              "geometry": {
                "type": "Polygon",
                "coordinates": [
                  [
                      [8.355693, 49.219097],
                      [8.355306, 49.218200],
                      [8.355178, 49.217555],
                      [8.356862, 49.217506],
                      [8.356873, 49.217548],
                      [8.356433, 49.217891],
                      [8.356433, 49.218018],
                      [8.356465, 49.219076],
                      [8.355693, 49.219097]
                    ]
                ]
              }
            },
            {
              "active":false,
              "type":0,
              "id": "GER-FMZ",
              "text": "GER-FMZ",
              "layer": 1,
              "coordinates": [
                {"lat":49.217646, "lon":8.376764}
              ]
            },
            {
              "active": false,
              "type": 4,
              "id": "LP-GER-FMZ",
              "text": "LP-GER-FMZ",
              "layer": 4,
              "geometry": {
                "type": "Polygon",
                "coordinates": [
                  [
                    [8.376024, 49.218144],
                    [8.376786, 49.217198],
                    [8.376421, 49.217050],
                    [8.376592, 49.216826],
                    [8.377547, 49.217015],
                    [8.376603, 49.218340],
                    [8.376024, 49.218144]
                  ]
                ]
              }
            },
            {
              "active":false,
              "type":0,
              "id": "GER-PAR",
              "text": "GER-Paradeplatz",
              "layer": 1,
              "coordinates": [
                {"lat":49.217562, "lon":8.375047}
              ]
            },
            {
              "active": false,
              "type": 8,
              "id": "LP-GER-PAR",
              "text": "LP-GER-Paradeplatz",
              "layer": 1,
              "windDepend":false,
              "windBits": 255,
              "windSectors": ["N", "NE", "E", "SE", "S", "SW", "W", "NW"],
              "altMin": -127,
              "altMax": 127,          "geometry": {
                "type": "Polygon",
                "coordinates": [
                  [
                    [8.374983, 49.218025],
                    [8.374425, 49.217723],
                    [8.376024, 49.216728],
                    [8.376142, 49.216770],
                    [8.375165, 49.218109],
                    [8.374983, 49.218025]
                  ]
                ]
              }
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
                    [8.260181, 48.763049],
                    [8.260621, 48.762971],
                    [8.261500, 48.763268],
                    [8.262283, 48.763410],
                    [8.262884, 48.763629],
                    [8.263699, 48.764089],
                    [8.264208, 48.764329],
                    [8.263634, 48.764591],
                    [8.262980, 48.764450],
                    [8.262631, 48.764372],
                    [8.262272, 48.764322],
                    [8.262074, 48.764524],
                    [8.261876, 48.764725],
                    [8.261254, 48.764648],
                    [8.260557, 48.764690],
                    [8.260214, 48.764633],
                    [8.260310, 48.764329],
                    [8.260278, 48.764174],
                    [8.260149, 48.763983]
                  ]
                ]
              }
            }
          ]
        }
      }
