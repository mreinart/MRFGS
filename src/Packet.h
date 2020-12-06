//
// Packet
//

#ifndef FANET_GS_PACKET_H
#define FANET_GS_PACKET_H

extern "C" {
#include "fanet_GS/fanet_radio.h"
#include "fanet_GS/fanet_struct.h"
};

#include <string>

namespace fanet {

    struct RadioInfo {
        int rssi;
        int prssi;
        float psnr;
        char coding_rate[5];
        bool crc_err;
        uint32_t timestamp;
    };

    class Packet {
        static unsigned int ognTypeFromFanetType(unsigned int fanetAircraftType);

    public:
        Packet() {};
        Packet(sRadioData radioData_, sFanetMAC fanetMAC_, sRawMessage rawMessage_) :
                radioData(radioData_), fanetMAC(fanetMAC_), rawMessage(rawMessage_) {};

        sRadioData radioData;
        sFanetMAC fanetMAC;
        sRawMessage rawMessage;

        virtual std::string toString();
        virtual std::string toRawString();
		virtual std::string toJson();

		static Packet fromJson(std::string json);

     };

};

#endif //FANET_GS_PACKET_H
