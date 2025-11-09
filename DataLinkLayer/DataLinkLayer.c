#include "../NetworkLayer/.h"
static u8 DeviceTimeoutMinute=10;
struct Device{
    struct DataLinkLayer dll;
    struct list_head lh;
    struct Hardware*h;
};
struct Hardware{
    struct packet_type pt;
    struct list_head lh,dlh;
    struct mutex l;
};
static LIST_HEAD(Hardwares);
CBuildConnectMinuteMemoryTimeout
CBuildSignature(struct sk_buff*, New, (void*dataLinkLayer)) {
    struct DataLinkLayer* dll = (struct DataLinkLayer*)dataLinkLayer;
    struct sk_buff*skb=alloc_skb(1514,GFP_KERNEL);
    if(!skb)
        return NULL;
    skb_reset_mac_header(skb);
    unsigned char*dskb=skb_put(skb,12);
    memcpy(dskb,dll->Address,6);
    struct Device*d=container_of(dll,struct Device,dll);
    memcpy(dskb+6,d->h->pt.dev->dev_addr, 6);
    GetMinuteMemoryTimeout()->Update(&dll->mmtr);
    return skb;
}
CBuildSignature(void,SetEthertype,(struct sk_buff* skb,u16 type)){
    *(u16*)skb_put(skb, 2)=type;
}
CBuildConnectNetworkLayer
CBuildSignature(struct Device*,SearchDevice,(struct sk_buff*skb,struct Hardware*h)){
    struct ethhdr*eth=eth_hdr(skb);
    struct Device*dtmp,*dsrc;
    list_for_each_entry_safe(dsrc,dtmp,&h->dlh, lh)
        if(!memcmp(dsrc->dll.Address, eth->h_source,6))
            return dsrc;
    return NULL;    
}
CBuildSignature(void,ExitDevice,(struct MinuteMemoryTimeoutRecord* mmtr)){
    struct DataLinkLayer*dll=container_of(mmtr,struct DataLinkLayer,mmtr);
    struct Device*d=container_of(dll,struct Device,dll);
    mutex_lock(&d->h->l);
    list_del_init(&d->lh);
    mutex_unlock(&d->h->l); 
    GetNetworkLayer()->FreeDataLinkLayer(dll);
    kfree(d);
}
CBuildSignature(void,InitDevice,(struct sk_buff*skb,struct Hardware*h)){
    mutex_lock(&h->l);
    struct Device*d=SearchDevice(skb,h);
    if(d){
        mutex_unlock(&h->l); 
        GetMinuteMemoryTimeout()->Update(&d->dll.mmtr);
        GetNetworkLayer()->ReceiverDataLinkLayer(skb,&d->dll);
        return;
    }
    d=kmalloc(sizeof(*d), GFP_KERNEL);
    if (!d){
        mutex_unlock(&h->l);
        return;
    }
    d->h = h;
    INIT_LIST_HEAD(&d->lh);
    mutex_init(&d->dll.l);
    memcpy(d->dll.Address, eth_hdr(skb)->h_source, 6);
    GetMinuteMemoryTimeout()->New(&d->dll.mmtr, &DeviceTimeoutMinute, ExitDevice);
    GetNetworkLayer()->InitDataLinkLayer(&d->dll);
    list_add(&d->lh, &h->dlh);
    mutex_unlock(&h->l);
    GetNetworkLayer()->ReceiverDataLinkLayer(skb,&d->dll);
}
CBuildSignature(void,HardwareReceiver,(struct sk_buff*skb,struct Hardware*h)){
    struct Device*d=SearchDevice(skb,h);
    if(d){
        GetMinuteMemoryTimeout()->Update(&d->dll.mmtr);
        GetNetworkLayer()->ReceiverDataLinkLayer(skb,&d->dll);
    }
    else 
        InitDevice(skb,h);
}
struct ForwardReceiverContent{
    struct Hardware*h;
    struct sk_buff*skb;
};
struct ForwardReceiverWork{
    struct ForwardReceiverContent*frc;
    struct work_struct wt;
} __attribute__((aligned(1)));
static struct kmem_cache*ForwardReceiverContentCachie;
CBuildSignature(void,ContinueReceiver,(struct work_struct*work)){
    struct ForwardReceiverWork*frw=container_of(work,struct ForwardReceiverWork,wt);
    struct Hardware*h=frw->frc->h;
    struct sk_buff*skb=frw->frc->skb;
    kmem_cache_free(ForwardReceiverContentCachie, frw->frc);
    HardwareReceiver(skb,h);
} 
CBuildSignature(int,DirectReceiver,(struct sk_buff*skb,struct net_device*dev,struct packet_type*pt,struct net_device*orig_dev)){
    if(!skb||skb->pkt_type==PACKET_OUTGOING)return NET_RX_SUCCESS;
    #ifdef MODULE
        struct tcphdr *tcph;
        struct udphdr *udph;
        switch(skb->protocol){
            case htons(ETH_P_IP):{
                struct iphdr*iph=ip_hdr(skb);
                if(!iph)
                    return NET_RX_SUCCESS;
                if(iph->protocol==IPPROTO_TCP){
                    tcph=tcp_hdr(skb);
                    if(!tcph||ntohs(tcph->source)==22||ntohs(tcph->dest)==22)
                        return NET_RX_SUCCESS;
                }else if(iph->protocol==IPPROTO_UDP){
                    udph=udp_hdr(skb);
                    if(!udph||ntohs(udph->source)==22||ntohs(udph->dest)==22)
                        return NET_RX_SUCCESS;
                }
                break;
            }
            case htons(ETH_P_IPV6):{
                struct ipv6hdr*ip6h=ipv6_hdr(skb);
                if(!ip6h)
                    return NET_RX_SUCCESS;
                if(ip6h->nexthdr==IPPROTO_TCP){
                    tcph=tcp_hdr(skb);
                    if(!tcph||ntohs(tcph->source)==22||ntohs(tcph->dest)==22)
                        return NET_RX_SUCCESS;
                }else if(ip6h->nexthdr==IPPROTO_UDP){
                    udph=udp_hdr(skb);
                    if(!udph||ntohs(udph->source)==22||ntohs(udph->dest)==22)
                        return NET_RX_SUCCESS;
                }
                break;
            }
        }
    #endif
    struct ForwardReceiverContent*frc=kmem_cache_alloc(ForwardReceiverContentCachie, GFP_KERNEL);
    if(!frc) 
        return NET_RX_DROP;
    *frc=(struct ForwardReceiverContent){
        .skb=skb,
        .h=container_of(pt, struct Hardware, pt)
    };
    struct ForwardReceiverWork*frw=(struct ForwardReceiverWork*)skb->cb;
    frw->frc=frc;
    skb_get(skb);
    INIT_WORK(&frw->wt,ContinueReceiver);
    schedule_work(&frw->wt);
    return NET_RX_DROP;
}
CBuildSignature(void,SynchronizeNet,(void)){
    synchronize_net();
}
CBuildInit{
    ForwardReceiverContentCachie=kmem_cache_create("ForwardReceiverContent",sizeof(struct ForwardReceiverContent),0,SLAB_HWCACHE_ALIGN,NULL);
    struct net_device *dev;
    rcu_read_lock();
    for_each_netdev(&init_net,dev){
        if(dev->type==ARPHRD_ETHER){
            struct Hardware *hardware=kmalloc(sizeof(*hardware),GFP_KERNEL);
            if(!hardware) 
                continue;
            *hardware=(struct Hardware){ 
                .pt.dev=dev,
                .pt.type=htons(ETH_P_ALL),
                .pt.func=DirectReceiver
            }; 
            mutex_init(&hardware->l);
            INIT_LIST_HEAD(&hardware->lh);
            list_add(&hardware->lh,&Hardwares);
            dev_add_pack(&hardware->pt);
        }
    }
    rcu_read_unlock();
    NetworkInit(SynchronizeNet);
    return 0;
}
CBuildExit{
    struct Hardware *h,*tmp;
    list_for_each_entry_safe(h, tmp, &Hardwares, lh) {
        dev_remove_pack(&h->pt);
        list_del(&h->lh);
        kfree(h);
    }
    synchronize_net();
    kmem_cache_destroy(ForwardReceiverContentCachie);  
    return 0;
}
CBuildStart(DataLinkLayer)
    CBuildBind(New)
    CBuildBind(SetEthertype)
CBuildEnd