#ifndef NetworkLayer_h
#define NetworkLayer_h
#include "../DataLinkLayer/.h"
struct DataLinkLayer{

};


    HBuildStart(NetworkLayer)
        HBuildSignature(DataLinkLayerPacketActionType,Receiver,(struct sk_buff*skb,void*dataLinkLayer)) 
        HBuildSignature(void,InitDataLinkLayer,(struct DataLinkLayer*dataLinkLayer))
        HBuildSignature(void,FreeDataLinkLayer,(struct DataLinkLayer*dataLinkLayer))

    HBuildEnd

     #define CBuildConnectNetworkLayer \
        CBuildConnectApplicationProgrammingInterface(NetworkLayer)

#endif



/*

Network Layer (Layer 3) – IP header (IPv4/IPv6).

Transport Layer (Layer 4) – TCP/UDP headers.

Session Layer (Layer 5) – Tracks sessions/connections. In practice, Linux apps and kernel mostly implement this inside TCP connections; there’s no separate header.

Presentation Layer (Layer 6) – Encryption, encoding, compression. Usually done in application protocols (TLS/SSL, etc.). Kernel does not modify this for raw packets.

Application Layer (Layer 7) – DNS, HTTP, FTP, etc. The actual data payload.

*/