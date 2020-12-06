//
// Track
//

#ifndef FANET_GS_TRACK_H
#define FANET_GS_TRACK_H

extern "C" {
#include "fanet_GS/fanet_struct.h"
};

#include <string>

namespace fanet {

    class Track {
        static unsigned int ognTypeFromFanetType(unsigned int fanetAircraftType);

    public:
        Track()
                : id(""), tracking(false), timestamp(0), type(0),
                  manufacturerId(0xFB), uniqueId(0x0000),
                  latitude(0.0), longitude(0.0), altitude(0),
                  heading(0.0), speed(0.0)  {}

        std::string id;
        char origin[4];
        char callsign[7];
        bool tracking;
		time_t timestamp;
        u_int8_t type;
		u_int8_t manufacturerId;
		u_int16_t uniqueId;
		float latitude;
		float longitude;
		int altitude;
		float heading;
        float speed;
        float climbRate;
        float turnRate;
        float distance;

        virtual u_int8_t ognType();
        virtual u_int8_t fanetType();

		virtual std::string toString();

		virtual std::string toJson();

		virtual void initFromJson(std::string json);

     };

    class AirTrack : public Track {
    public:
        AirTrack()
                : Track() {}

		virtual void initFromFanetPacket(sAirTracking *track);

		virtual std::string toString();
    };

	class GroundTrack : public Track {
	public:
		GroundTrack()
				: Track(), activity("?") {}

		virtual void initFromFanetPacket(sGroundTracking *tr);

		virtual std::string toString();

		std::string activity;
	};

	class Device {
	public:
		std::string id;
		std::string name;
        u_int8_t manufacturerId;
        u_int16_t uniqueId;
        int type;
        uint32_t timestamp;
		Track *firstPosition;
		Track *lastPosition;

		Device() :
		    id(""), name(""),
		    manufacturerId(0), uniqueId(0),
		    type(0), timestamp(0),
		    firstPosition(nullptr), lastPosition(nullptr) {};

		Device(std::string id) :
		    id(id), name(""),
		    manufacturerId(0), uniqueId(0),
		    type(0), timestamp(0),
		    firstPosition(nullptr), lastPosition(nullptr) {};

	};

};

#endif //FANET_GS_TRACK_H
