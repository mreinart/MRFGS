//
// Message
//

#ifndef FANET_GS_MESSAGE_H
#define FANET_GS_MESSAGE_H

extern "C" {
#include "fanet_GS/fanet_struct.h"
};

#include <string>

namespace fanet {

    class Message {

        public:
            Message()
                    : message(""), source(""), dest(""),
                      timestamp(0), unicast(false),
                      s_manufacturerId(0x00), s_uniqueId(0x0000),
                      d_manufacturerId(0x00), d_uniqueId(0x0000) {}

            char message[256];
            char source[7];
            char dest[7];
            time_t timestamp;
            bool unicast;
            u_int8_t s_manufacturerId;
            u_int16_t s_uniqueId;
            u_int8_t d_manufacturerId;
            u_int16_t d_uniqueId;

            virtual std::string toString();

            virtual std::string toJson();

            void initFromJson(std::string json);

            void initFromText(std::string sender, std::string receiver, bool unicast, std::string message);
    };
};

#endif //FANET_GS_MESSAGE_H
