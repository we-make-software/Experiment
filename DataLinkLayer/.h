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
typedef enum{
    DataLinkLayerPacketDropAndFree=0,
    DataLinkLayerPacketDropAndKeep=1,
    DataLinkLayerPacketReceiveForOtherSoftware=2
} DataLinkLayerPacketActionType;
    HBuildStart(DataLinkLayer)
        HBuildSignature(struct sk_buff*,New,(void*dataLinkLayer,u16*ethertype)) 
        HBuildSignature(int,Send,(struct sk_buff*skb))
        HBuildSignature(u16,GetEthertype,(struct sk_buff*skb))
    HBuildEnd

    #define CBuildConnectDataLinkLayer \
        CBuildConnectApplicationProgrammingInterface(DataLinkLayer)

#endif