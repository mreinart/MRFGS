//
// GroundStation
//

#ifndef DF_FANET_GS_GROUNDSTATION_H
#define DF_FANET_GS_GROUNDSTATION_H

#include <list>
#include <map>
#include "EntityName.h"
#include "Message.h"
#include "Packet.h"
#include "Track.h"
#include "Vehicle.h"
#include "../include/TDequeConcurrent.h"

namespace fanet {

    struct FirmwareInfo {
        uint8_t deviceType;
        uint16_t year;
        uint8_t month;
        uint8_t day;
        bool experimental;
        uint8_t add1;
        uint8_t add2;
    };

    class GroundStation {
    protected:
		static GroundStation *instance_;
		bool initialized_;
		bool finished_;
        std::string fanetId_;
        std::string name_;
		float lat_;
		float lon_;
		int alt_;
        bool sendHwInfo_;
        bool pushTracks_;
        bool pushPackets_;
        std::string trackDbHost_;
        std::string updateUrlTrack_;
        std::string updateUrlDevice_;
        std::string packetDbHost_;
        std::string updateUrlPacket_;

		GroundStation() :
                initialized_(false), finished_(false),
                pushPackets_(false), pushTracks_(false),
                trackDbHost_(""), updateUrlTrack_(""), updateUrlDevice_(""), packetDbHost_(""), updateUrlPacket_("") {};
        virtual void init();

	public:
        std::map<std::string, Device *> deviceMap;
        std::map<std::string, Vehicle *> vehicleMap;
        TDequeConcurrent<Message*> messageToFanetQueue;
        TDequeConcurrent<Message*> messageToInetQueue;
        TDequeConcurrent<Packet*> packetQueue;
        TDequeConcurrent<Track*> trackQueue;
        TDequeConcurrent<EntityName*> nameQueue;
        std::mutex radioMutex;

        virtual ~GroundStation() = default;
        static GroundStation *getInstance() {
			if (instance_ == 0) {
				instance_ = new GroundStation();
                instance_->init();
			}
			return instance_;
		}

        std::string fanetId();
        std::string name();
        u_int8_t manufacturerId;
        u_int16_t uniqueId;
        FirmwareInfo fwInfo;
        bool sendHwInfo;
        float latitude() { return lat_; };
        float longitude() { return lon_; };
        int altitude() { return alt_; };

        void addTrack(Track *track);
        void addName(EntityName *entityName);
        void sendTrackToDfDb(Track *track);

        void sendDeviceToDfDb(Device *device);
        void sendPacketToDfDb(Packet *packet);

        std::string getStatusInfo();

        virtual void run();
        virtual void runIteration();

		virtual void stop() { finished_ = true; };
	};
};

#endif //DF_FANET_GS_GROUNDSTATION_H
