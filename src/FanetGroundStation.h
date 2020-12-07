//
// FANET GroundStation
//

#ifndef DF_FANET_GS_FANETGROUNDSTATION_H
#define DF_FANET_GS_FANETGROUNDSTATION_H

#include "GroundStation.h"

namespace fanet {

    class FanetGroundStation : public GroundStation {
    protected:
        FanetGroundStation() : GroundStation() {};
        void init();

	public:
        static FanetGroundStation *getInstance() {
            FanetGroundStation *fanetInstance;
			if (instance_ == 0) {
                fanetInstance = new FanetGroundStation();
                fanetInstance->init();
                instance_ = fanetInstance;
			} else {
                fanetInstance = dynamic_cast<FanetGroundStation*>(instance_);
			}
			return fanetInstance;
		}

        void addTrack(Track *track);

        void sendTrackToFANET(Track *track);
        void sendAirTrackToFANET(AirTrack *track);
        void sendGndTrackToFANET(GroundTrack *track);

		void run();

		void stop() { finished_ = true; };
	};
};

#endif //DF_FANET_GS_FANETGROUNDSTATION_H
