# MRFGS
FANET Groundstation

## References
 * FANET protocol specification https://github.com/3s1d/fanet-stm32/blob/master/Src/fanet/radio/protocol.txt
 * Radio/Packet Code from https://github.com/Betschi/FANET_V2019_02_07

## FANET Packet Types

### receive and process FANET packets
* Type 0 - ACK
* Type 1 - (Air)Tracking
* Type 2 - Name
* Type 3 - Message
* Type 4 - Service - Weather - relay Skytraxx Windstations
* Type 7 - Groundtracking
* Type 8 - Device Info - (old)
* Type 9 - Thermals
* Type A - Device Info - (new)

### send FANET packets
* Type 0 - ACK
* Type 1 - (Air)Tracking
* Type 2 - Name
* Type 3 - Message
* Type 4 - Service (Weatherdata from local Weatherstations)
* Type 5 - Landmarks
* Type 7 - Groundtracking
