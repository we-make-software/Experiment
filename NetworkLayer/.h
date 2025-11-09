#ifndef NetworkLayer_h
#define NetworkLayer_h
#include "../DataLinkLayer/.h"
struct DataLinkLayer{
    unsigned char Address[6];
    struct MinuteMemoryTimeoutRecord mmtr; 
    struct mutex l;
};

    #define HStart(description,signature)\
        HBuildStart(description)\
        signature\
        HBuildEnd
    
    #define CStart(description,signature)\
        CBuildStart(description)\
            signature\
        CBuildEnd    
    
    HStart(NetworkLayer,
        HBuildSignature(void,InitDataLinkLayer,(struct DataLinkLayer*))
        HBuildSignature(void,FreeDataLinkLayer,(struct DataLinkLayer*))
        HBuildSignature(void,ReceiverDataLinkLayer,(struct sk_buff*,struct DataLinkLayer*))
    )

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