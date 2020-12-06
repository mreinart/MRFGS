//
// AprsOgnManager
//

#ifndef FANET_GS_APRS_OGN_MANAGER_H
#define FANET_GS_APRS_OGN_MANAGER_H

#include <fstream>
#include <list>
#include "GroundStation.h"
#include "Track.h"
#include "WeatherStation.h"

using namespace std;

namespace fanet {

    class AprsOgnManager {
        static AprsOgnManager *instance_;
        bool initialized_;

        string aprsUser;
        string aprsPass;
        int socket_;
        fstream trackFile_;

        void encodeTrackAPRS(Track *track, char *trackStr);
        void sendAPRSbuffer(const char *trackBuffer);

        AprsOgnManager() : initialized_(false) {};

        void init();

    public:

        static AprsOgnManager *getInstance() {
            if (instance_ == 0) {
                instance_ = new AprsOgnManager();
                instance_->init();
            }
            return instance_;
        }

        string stationName;
        string server;
        unsigned int port;
        string appName;
        string appVersion;

		void sendKeepAlive();
		void sendMessage(const char *message);
		void sendTrack(Track *track);
		void sendTrackList(list<Track *> trackList);
		void sendReceiverBeacon(GroundStation *groundStation);

        Track *parseAprsTrack(const char *aprsLine);

        void connect();
        boolean isConnected();
        void recveiverLoop();
    };

}

#endif //FANET_GS_APRS_OGN_MANAGER_H
