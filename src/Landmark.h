//
// Landmark / LandmarkManager
//

#ifndef DF_FANET_GS_LANDMARK_H
#define DF_FANET_GS_LANDMARK_H

#include <string>
#include <array>
#include <list>

extern "C" {
#include "fanet_GS/fanet_struct.h"
#include "fanet_GS/fanet_radio.h"
}


using namespace std;

namespace fanet {

    struct Coordinate2D {
        float lat;
        float lon;
    };

/*
https://github.com/3s1d/fanet-stm32/blob/master/Src/fanet/radio/protocol.txt

Landmarks (Type = 5)
Note: Landmarks are completely independent. Thus the first coordinate in each packet has to be an absolute one. All others are compressed in relation to the one before.
Note2: Identification/detection shall be done by hashing the whole payload, excluding bytes 0, 1 and, 2 (optional). That way one quietly can change the layer to 'Don't care' and quickly
	destroy the landmark w/o having to wait for it's relative live span to be exceeded.
Note3: In case a text has the same postion as the first position of any other landmark then the text is considered to be the label of that landmark.

[Byte 0]
bit 4-7		Time to live +1 in 10min (bit 7 scale 6x or 1x, bit 4-6) (0->10min, 1->20min, ..., F->8h)
bit 0-3		Subtype:
			0:     Text
			1:     Line
			2:     Arrow
			3:     Area
			4:     Area Filled
			5:     Circle
			6:     Circle Filled
			7:     3D Line		suitable for cables
			8:     3D Area		suitable for airspaces (filled if starts from GND=0)
			9:     3D Cylinder	suitable for airspaces (filled if starts from GND=0)
			10-15: TBD
[Byte 1]
bit 7-5		Reserved
bit 4		Internal wind dependency (+1byte wind sector)
bit 3-0		Layer:
			0:     Info
			1:     Warning
			2:     Keep out
			3:     Touch down
			4:     No airspace warn zone		(not yet implemented)
			5-14:  TBD
			15:    Don't care
[Byte 2 only if internal wind bit is set] Wind sectors +/-22.5degree (only display landmark if internal wind is within one of the advertised sectors.
									If byte 2 is present but is zero, landmark gets only displayed in case of no wind)
bit 7 		NW
bit 6		W
bit 5		SW
bit 4 		S
bit 3 		SE
bit 2 		E
bit 1 		NE
bit 0 		N

[n Elements]
			Text (0): 		Position (Absolute) + String 				//(2 Byte aligned, zero-termination is optional)
			Line/Arrow (1,2):	Position (1st absolute others compressed, see below, minimum 2 elements)
			Area (filled)(3,4): 	Position (1st absolute others compressed, see below, minimum 3 elements)
			Circle (filled)(5,6):	n times: Position (1st absolute others compressed, see below) + Radius (1Byte in 50m, bit 7 scale 8x or 1x, bit 0-6)
			3D Line (7):		n times: Position (1st in packet absolute others compressed, see below) + Altitude (('1Byte signed'+109) * 25m (-127->-450m, 127->5900m))
			3D Area (8):		Altitude bottom, top (each: ('1Byte signed'+109) * 25m (-127->-450m, 127->5900m), only once) +
							n times: Position (1st absolute others compressed, see below)
			3D Cylinder (9):	n times: Position (1st absolute others compressed, see below) + Radius (1Byte in 50m, bit 7 scale 8x or 1x, bit 0-6) +
							Altitude bottom, top (each: ('1Byte signed'+109) * 25m (-127->-450m, 127->5900m), only once)

     */

    class Landmark {
    public:
        Landmark() :
            id(""), text(""), timeToLive(0), layer(0),
            diameter(0), altMin(0), altMax(0),
            windDep(false), windBits(0), lastUpdate(0)
            {};

        u_int8_t type;
        string id;
        string text;
        u_int8_t timeToLive;
        u_int8_t layer;
        u_int16_t diameter;
        int16_t altMin;
        int16_t altMax;
        bool windDep;
        u_int8_t windBits;
        time_t lastUpdate;
        list<Coordinate2D> coordinates;

        string toString();

        virtual void encode_raw_message(sRawMessage *);

        };

    class LandmarkManager {

    private:
        static LandmarkManager *instance_;
        bool initialized_;
        bool finished_;

        LandmarkManager() : initialized_(false), finished_(false) {};


    public:
        list <Landmark> landmarkList;

        static LandmarkManager *getInstance() {
            if (instance_ == 0) {
                instance_ = new LandmarkManager();
            }
            return instance_;
        }

        void sendRawMessage(sRawMessage *_tx_message);
        void sendLandmark(Landmark *landmark);

        void init();
        void reinit();

        void run();
        void stop() { finished_ = true; };

    };

}

#endif //DF_FANET_GS_LANDMARK_H
