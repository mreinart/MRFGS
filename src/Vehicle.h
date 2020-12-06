//
// Vehicle
//

#ifndef FANET_VEHICLE_H
#define FANET_VEHICLE_H

#include <map>
#include <string>
#include "TDequeConcurrent.h"

#include "Track.h"

using namespace std;

namespace fanet {

    class Vehicle {
    protected:
        bool initialized_;

        bool dfDbActive_;
        std::string dbHost_;
        std::string readUrl_;

    public:
        Vehicle(string id) : initialized_(false), id(id), name(""),
                                    manufacturerId(0xFB), uniqueId(0x0000),
                                    lastPosition(NULL), lastUpdate(0), maxAgeSeconds(180) {}

        Vehicle(string id, string name, int type, u_int8_t manufacturerId, u_int16_t uniqueId) :
                initialized_(false),
                id(id), name(name), type(type),
                manufacturerId(manufacturerId), uniqueId(uniqueId),
                lastPosition(NULL), lastUpdate(0), maxAgeSeconds(180) {}

        string id;
        string name;
        int type;
        u_int8_t manufacturerId;
        u_int16_t uniqueId;
        Track *lastPosition;
        time_t lastUpdate;
        int maxAgeSeconds;

        bool pullDb;
        bool pushDb;

        virtual string toJson();

        virtual void init();
        virtual void update();
        virtual void updateDfDb();
        virtual void readDfDb();
    };

}; // namespace fanet

#endif //FANET_VEHICLE_H
