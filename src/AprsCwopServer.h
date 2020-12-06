//
// AprsCwopServer
//

#ifndef FANET_GS_APRS_CWOP_SERVER_H
#define FANET_GS_APRS_CWOP_SERVER_H

#include <fstream>
#include <list>
#include "GroundStation.h"
#include "Track.h"
#include "WeatherStation.h"

using namespace std;

namespace fanet {

    class AprsCwopServer {
        static AprsCwopServer *instance_;
        bool initialized_;

        string aprsUser;
        string aprsPass;
        int socket_;
        fstream trackFile_;

        void encodeTrackAPRS(Track *track, char *trackStr);
        void sendAPRSbuffer(const char *trackBuffer);

        AprsCwopServer() : initialized_(false) {};

        void init();

    public:

        static AprsCwopServer *getInstance() {
            if (instance_ == 0) {
                instance_ = new AprsCwopServer();
                instance_->init();
            }
            return instance_;
        }

        string stationName;
        string server;
        unsigned int port;
        string appName;
        string appVersion;
        bool isActive;

		void sendKeepAlive();
		void sendMessage(const char *message);
		void sendReceiverBeacon(GroundStation *groundStation);

        void encodeWeatherAPRS(WeatherStation *ws, char *aprsStr);

        void connect();
        boolean isConnected();
    };

}

#endif //FANET_GS_APRS_CWOP_SERVER_H
