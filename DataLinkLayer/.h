#ifndef DataLinkLayer_h
#define DataLinkLayer_h
#include "../SecondMemoryTimeout/.h"
#include "../MinuteMemoryTimeout/.h"
#include <linux/ethtool.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h> 
#include <linux/workqueue.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <linux/udp.h>

    HBuildStart(DataLinkLayer)
    
    HBuildEnd

    #define CBuildConnectDataLinkLayer \
        CBuildConnectApplicationProgrammingInterface(DataLinkLayer)

#endif