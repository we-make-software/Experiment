/**
 * Description:
 * This module implements a basic Data Link Layer abstraction for Ethernet devices.
 * It provides a simple, high-performance interface to create, manipulate, and send
 * network packets (sk_buffs) while handling MAC addresses and ethertype fields.
 *
 * The DataLinkLayer provides the following public API functions:
 *  - New(void* dataLinkLayer,u16*ethertype): Allocates or retrieves a preallocated sk_buff ready for use.
 *  - Send(struct sk_buff* skb): Sends a prepared sk_buff to the network interface.
 *  - GetEthertype(struct sk_buff* skb): Retrieves the Ethernet type field from a given skb.

 *
 * Key Concepts:
 * 1. Buffer Pooling:
 *      - To minimize packet allocation overhead, the DataLinkLayer maintains a pool of
 *        preallocated sk_buffs for each source and destination.
 *      - These pools are organized in small fixed-size SocketBufferPools inside a larger
 *        BufferPool.
 *      - When a new packet is needed, it is retrieved from the pool if available, reducing
 *        expensive kmalloc/free calls at runtime.
 *      - If no buffer is available, a new sk_buff is allocated on demand.
 *
 * 2. Preallocation Benefits:
 *      - Reduces latency in sending packets since buffers are already prepared.
 *      - Helps maintain throughput in high-performance networking scenarios.
 *      - Avoids unnecessary fragmentation or scatter-gather operations at packet creation.
 *
 * Notes:
 *  - The module only exposes a minimal set of functions for external use.
 *  - MAC address handling and packet creation are managed internally.
 *  - More advanced functionality, such as packet forwarding, filtering, or higher-level
 *    network processing, is handled by the NetworkLayer project.
 *
 */

#ifndef DataLinkLayer_h
#define DataLinkLayer_h
#include "../System/.h"
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