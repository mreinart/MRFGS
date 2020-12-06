//
// Track
//

#include "Track.h"

#include "loguru.hpp"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;
using namespace rapidjson;
using namespace fanet;

void GroundTrack::initFromFanetPacket(sGroundTracking *tr) {
    sprintf(origin, "FAN");
    sprintf(callsign, "%02X%04X", tr->s_address_manufactur_id, tr->s_address_unique_id);
    id = "GndTrack-" + string(callsign);
    tracking = tr->tracking;
    timestamp = time(0);
    type = tr->ground_type;
    manufacturerId = tr->s_address_manufactur_id;
    uniqueId = tr->s_address_unique_id;
    latitude = tr->latitude;
    longitude = tr->longitude;
    altitude = 0;
    distance = tr->distance;
    climbRate = 0.0;
    turnRate = 0.0;
}

void AirTrack::initFromFanetPacket(sAirTracking *tr) {
    sprintf(origin, "FAN");
    sprintf(callsign, "%02X%04X", tr->s_address_manufactur_id, tr->s_address_unique_id);
    id = "AirTrack-" + string(callsign);        // TODO: Track ID format / config
    tracking = tr->tracking;
    timestamp = time(0);
    type = tr->aircraft_type;
    manufacturerId = tr->s_address_manufactur_id;
    uniqueId = tr->s_address_unique_id;
    latitude = tr->latitude;
    longitude = tr->longitude;
    altitude = tr->altitude;
    speed = tr->speed;
    heading = tr->heading;
    climbRate = tr->climb;
    turnRate = 0.0;
    if (tr->turn_rate_on)
        turnRate = tr->turn_rate;
    distance = tr->distance;
}

string Track::toString() {
	struct tm *timeinfo;
	timeinfo = localtime(&this->timestamp);
	char timeBuf[80];
	strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", timeinfo);
	stringstream ss;
	ss << "Track: "
	   << this->id
            << " - " << this->origin
            << " - " << this->callsign
	   << " - " << this->type;
	if (this->tracking) {
	    ss << "/tracking";
	} else {
	    ss << "/NO-track";
	}
	ss << "/" << timeBuf << "(" << this->timestamp << ")"
	   << "/" << this->latitude
	   << "/" << this->longitude
	   << "/" << this->altitude
	   << "/" << this->heading
	   << "/" << this->speed
	   << "/" << this->distance;
	return string(ss.str());
}

string AirTrack::toString() {
	return "Air" + Track::toString();
}

string GroundTrack::toString() {
	return "Gnd" + Track::toString();
}

string Track::toJson() {
	StringBuffer sb;
	Writer<StringBuffer> writer(sb);

	writer.StartObject();
	{
        writer.Key("type");
        writer.String("Track");
        writer.Key("id");
        writer.String(this->id.c_str());
        writer.Key("callsign");
        writer.String(this->callsign);
        writer.Key("origin");
        writer.String(this->origin);
        writer.Key("timestamp");
        writer.Uint(this->timestamp);
        writer.Key("tracking");
        writer.Bool(this->tracking);

        char xbuf[5];
        writer.Key("manufacturerId");
        sprintf(xbuf, "%02X", this->manufacturerId);
        writer.String(xbuf);
        writer.Key("uniqueId");
        sprintf(xbuf, "%04X", this->uniqueId);
        writer.String(xbuf);

        writer.Key("latitude");
        writer.Double(this->latitude);
        writer.Key("longitude");
        writer.Double(this->longitude);
        writer.Key("altitude");
        writer.Int(this->altitude);
        writer.Key("course");
        writer.Double(this->heading);
        writer.Key("speed");
        writer.Double(this->speed);
        writer.Key("distance");
        writer.Double(this->distance);
    }
    writer.EndObject();
    cout << sb.GetString() << endl;
    return sb.GetString();
}

void Track::initFromJson(std::string json) {
    this->id = "---";
    try {
        Document d;
        d.Parse(json.c_str());
        if (!d.IsObject())
            return;
        Value::ConstMemberIterator itr = d.FindMember("type");
        if (itr == d.MemberEnd()) {
            return;
        } else if (0 != strcmp("Track", itr->value.GetString())) {
            return;
        }
        Value &value = d["id"];
        this->id = value.GetString();
        value = d["callsign"];
        string str = value.GetString();
        strcpy(this->callsign, str.c_str());
        value = d["origin"];
        str = value.GetString();
        strcpy(this->origin, str.c_str());
        value = d["timestamp"];
        this->timestamp = value.GetInt();
        value = d["tracking"];
        this->tracking = value.GetBool();
        this->manufacturerId = strtol(d["manufacturerId"].GetString(), NULL, 16);
        this->uniqueId = strtol(d["uniqueId"].GetString(), NULL, 16);
        value = d["latitude"];
        this->latitude = value.GetFloat();
        value = d["longitude"];
        this->longitude = value.GetFloat();
        value = d["altitude"];
        this->altitude = value.GetInt();
        value = d["course"];
        this->heading = value.GetFloat();
        value = d["speed"];
        this->speed = value.GetFloat();
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Issue with Track : %s", e.what());
    } catch (...) {
        LOG_F(ERROR, "Issue with Track");
    }
}

unsigned int Track::ognTypeFromFanetType(unsigned int fanetAircraftType) {
    // see https://github.com/wbuczak/ogn-commons-java/blob/master/src/main/java/org/ogn/commons/beacon/AircraftType.java
    // UNKNOWN(0), GLIDER(1), TOW_PLANE(2), HELICOPTER_ROTORCRAFT(3), PARACHUTE(4), DROP_PLANE(5),
    // HANG_GLIDER(6), PARA_GLIDER(7), POWERED_AIRCRAFT(8), JET_AIRCRAFT(9), UFO(10), BALLOON(11), AIRSHIP(12), UAV(13),
    // STATIC_OBJECT(15);
    //
    // https://www.programcreek.com/java-api-examples/?code=glidernet%2Fogn-commons-java%2Fogn-commons-java-master%2Fsrc%2Fmain%2Fjava%2Forg%2Fogn%2Fcommons%2Fbeacon%2Fdescriptor%2FAircraftDescriptorProvider.java#
    //
    //  RANDOM(0) - changing (random) address generated by the device
    //	ICAO(1)
    //	FLARM(2)  FLARM HW
    //	OGN(3)    OGN tracker HW
    //
    // translate FANET type to OGN type
    unsigned int ognType = 0;
    switch (fanetAircraftType) {
        case 0: // Other -> UNKNOWN
            ognType = 0;
            break;
        case 1: // Paraglider
            ognType = 7;
            break;
        case 2: // Hangglider
            ognType = 6;
            break;
        case 3: // Balloon
            ognType = 11;
            break;
        case 4: // Glider
            ognType = 1;
            break;
        case 5: // Aircraft -> POWERED_AIRCRAFT
            ognType = 8;
            break;
        case 6: // Helicopter
            ognType = 3;
            break;
        case 7: // UAV
            ognType = 13;
            break;
    }
    return ognType;
}

u_int8_t Track::ognType() {
    return ognTypeFromFanetType(this->type);
}

u_int8_t Track::fanetType() {
    return this->type;
};
