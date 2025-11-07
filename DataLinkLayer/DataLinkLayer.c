#include "../NetworkLayer/.h"
#define SOCKET_POOLS      4
#define BUFFER_INCREMENT  50
#define SKB_SIZE          1514
#define SOURCE_TIMEOUT_MS 600900
#define DEST_TIMEOUT_MS   600800
#define DECREASE_MS       1500
#define RECOVERY_MS       500

struct SocketBufferPool {
    struct list_head data;
    struct mutex lock;
    u64 count;
};
struct BufferPool {
    u32 increaser;
    ktime_t timestamp;
    struct mutex lock;
    struct work_struct creator;
    struct delayed_work decrease;
    struct SocketBufferPool socketBufferPools[SOCKET_POOLS];
};

struct DataLinkLayerSource {
    struct packet_type pt;
    struct list_head list;
    struct list_head destinations;
    struct BufferPool bufferPool;
    struct delayed_work timeout;
    struct mutex lock;
};

struct DataLinkLayerDestination {
    struct list_head list;
    u8 Address[6];
    struct BufferPool bufferPool;
    struct delayed_work timeout;
    struct DataLinkLayerSource *dataLinkLayerSource;
    struct DataLinkLayer ddl;
};

static LIST_HEAD(DLSourcePoolCreators);

CBuildSignature(void,DLDestPool_Free,(struct DataLinkLayerDestination *dlld)){
    list_del(&dlld->list);
    cancel_work_sync(&dlld->bufferPool.creator);
    INIT_WORK(&dlld->bufferPool.creator,NULL);
    cancel_delayed_work_sync(&dlld->bufferPool.decrease);
    dlld->bufferPool.increaser=0;
    dlld->bufferPool.timestamp=0;
    for(u8 i=0;i<SOCKET_POOLS;i++){
        struct SocketBufferPool *pool=&dlld->bufferPool.socketBufferPools[i];
        mutex_lock(&pool->lock);
        struct sk_buff *skb,*tmp;
        list_for_each_entry_safe(skb,tmp,&pool->data,list){
            list_del(&skb->list);
            kfree_skb(skb);
        }
        pool->count=0;
        mutex_unlock(&pool->lock);
    }
    kfree(dlld);
}
CBuildSignature(void,DLSourcePool_Free,(struct DataLinkLayerSource *dlls,bool force)){
    struct DataLinkLayerDestination *dlld,*tmp;
    list_for_each_entry_safe(dlld,tmp,&dlls->destinations,list) DLDestPool_Free(dlld);
    mutex_lock(&dlls->bufferPool.lock);
    cancel_work_sync(&dlls->bufferPool.creator);
    if(force) INIT_WORK(&dlls->bufferPool.creator,NULL);
    cancel_delayed_work_sync(&dlls->bufferPool.decrease);
    dlls->bufferPool.increaser=0;
    dlls->bufferPool.timestamp=0;
    for(u8 i=0;i<SOCKET_POOLS;i++){
        struct SocketBufferPool *pool=&dlls->bufferPool.socketBufferPools[i];
        mutex_lock(&pool->lock);
        struct sk_buff *skb,*tmp;
        list_for_each_entry_safe(skb,tmp,&pool->data,list){
            list_del(&skb->list);
            kfree_skb(skb);
        }
        pool->count=0;
        mutex_unlock(&pool->lock);
    }
    mutex_unlock(&dlls->bufferPool.lock);
}

CBuildSignature(void,DLSourcePool_Timeout,(struct work_struct *work)){
    struct DataLinkLayerSource *dlls=container_of(work,struct DataLinkLayerSource,timeout.work);
    DLSourcePool_Free(dlls,false);
}

CBuildSignature(void,DLSourcePool_Creator,(struct work_struct *work)){
    struct BufferPool *pool=container_of(work,struct BufferPool,creator);
    struct DataLinkLayerSource *dlls=container_of(pool,struct DataLinkLayerSource,bufferPool);
    u64 size=((pool->increaser+1)*BUFFER_INCREMENT)+1;
    for(u8 i=0;i<SOCKET_POOLS;i++){
        struct SocketBufferPool *sockPool=&pool->socketBufferPools[i];
        mutex_lock(&sockPool->lock);
        for(u64 j=sockPool->count;j<size;j++){
            struct sk_buff *skb=alloc_skb(SKB_SIZE,GFP_KERNEL);
            if(!skb) break;
            skb->dev=dlls->pt.dev;
            skb_reset_mac_header(skb);
            memcpy(skb_put(skb,12)+6,dlls->pt.dev->dev_addr,6);
            list_add_tail(&skb->list,&sockPool->data);
            sockPool->count++;
        }
        mutex_unlock(&sockPool->lock);
    }
}

CBuildSignature(void,BufferPool_Decrease,(struct work_struct *work)){
    struct BufferPool *pool=container_of(work,struct BufferPool,decrease.work);
    if(!work_pending(&pool->creator)) goto renew;
    mutex_lock(&pool->lock);
    if(ktime_to_ms(ktime_sub(ktime_get(),pool->timestamp))<RECOVERY_MS && pool->increaser) pool->increaser--;
    mutex_unlock(&pool->lock);
    if(!pool->increaser) return;
renew:
    schedule_delayed_work(&pool->decrease,msecs_to_jiffies(DECREASE_MS));
}
CBuildSignature(void,BufferPool_Init,(struct BufferPool *pool,void(*creator)(struct work_struct*))){
    pool->increaser=0;
    pool->timestamp=0;
    mutex_init(&pool->lock);
    INIT_DELAYED_WORK(&pool->decrease,BufferPool_Decrease);
    for(u8 i=0;i<SOCKET_POOLS;i++){
        struct SocketBufferPool *sockPool=&pool->socketBufferPools[i];
        mutex_init(&sockPool->lock);
        sockPool->count=0;
        INIT_LIST_HEAD(&sockPool->data);
    }
    INIT_WORK(&pool->creator,creator);
}

CBuildSignature(struct sk_buff*,DLSourcePool_GetBuffer,(struct DataLinkLayerSource *dlls)){
    for(u8 i=0;i<SOCKET_POOLS;i++){
        struct SocketBufferPool *sockPool=&dlls->bufferPool.socketBufferPools[i];
        if(mutex_trylock(&sockPool->lock)){
            if(!sockPool->count){ mutex_unlock(&sockPool->lock); continue; }
            struct sk_buff *skb=list_first_entry(&sockPool->data,struct sk_buff,list);
            list_del(&skb->list);
            sockPool->count--;
            mutex_unlock(&sockPool->lock);
            if(!sockPool->count && work_pending(&dlls->bufferPool.creator)) schedule_work(&dlls->bufferPool.creator);
            schedule_delayed_work(&dlls->timeout,msecs_to_jiffies(SOURCE_TIMEOUT_MS));
            return skb;
        }
    }
    struct sk_buff *skb=alloc_skb(SKB_SIZE,GFP_KERNEL);
    if(!skb) return NULL;
    skb->dev=dlls->pt.dev;
    skb_reset_mac_header(skb);
    u8 *data=skb_put(skb,12);
    memcpy(data+6,dlls->pt.dev->dev_addr,6);
    if(work_pending(&dlls->bufferPool.creator)){
        mutex_lock(&dlls->bufferPool.lock);
        dlls->bufferPool.increaser++;
        dlls->bufferPool.timestamp=ktime_get();
        mutex_unlock(&dlls->bufferPool.lock);
        schedule_delayed_work(&dlls->bufferPool.decrease,msecs_to_jiffies(DECREASE_MS));
        schedule_work(&dlls->bufferPool.creator);
    }
    schedule_delayed_work(&dlls->timeout,msecs_to_jiffies(SOURCE_TIMEOUT_MS));
    return skb;
}

CBuildSignature(void,DLDestPool_Creator,(struct work_struct *work)){
    struct BufferPool *pool=container_of(work,struct BufferPool,creator);
    struct DataLinkLayerDestination *dlld=container_of(pool,struct DataLinkLayerDestination,bufferPool);
    u64 size=((pool->increaser+1)*BUFFER_INCREMENT)+1;
    for(u8 i=0;i<SOCKET_POOLS;i++){
        struct SocketBufferPool *sockPool=&pool->socketBufferPools[i];
        mutex_lock(&sockPool->lock);
        for(u64 j=sockPool->count;j<size;j++){
            struct sk_buff *skb=DLSourcePool_GetBuffer(dlld->dataLinkLayerSource);
            if(!skb) break;
            memcpy(skb_mac_header(skb),dlld->Address,6);
            list_add_tail(&skb->list,&sockPool->data);
            sockPool->count++;
        }
        mutex_unlock(&sockPool->lock);
    }
}

CBuildSignature(void,DLDestPool_Timeout,(struct work_struct *work)){
    struct DataLinkLayerDestination *dlld=container_of(work,struct DataLinkLayerDestination,timeout.work);
    DLDestPool_Free(dlld);
}

CBuildSignature(struct DataLinkLayerDestination*,DLGetDestination,(struct DataLinkLayerSource *dlls,u8(*address)[6])){
    struct DataLinkLayerDestination *dlld;
    list_for_each_entry(dlld,&dlls->destinations,list) if(memcmp(dlld->Address,*address,6)==0) return dlld;
    return NULL;
}

CBuildConnectNetworkLayer

CBuildSignature(void*,DLInitDestination,(struct DataLinkLayerSource *dlls,u8 (*address)[6])){
    struct DataLinkLayerDestination *dlld=DLGetDestination(dlls,address);
    if(dlld) goto Update;
    mutex_lock(&dlls->lock);
    if((dlld=DLGetDestination(dlls,address))){ mutex_unlock(&dlls->lock); goto Update; }
    dlld=kmalloc(sizeof(*dlld),GFP_KERNEL);
    if(!dlld){ mutex_unlock(&dlls->lock); return NULL; }
    memcpy(dlld->Address,*address,6);
    INIT_LIST_HEAD(&dlld->list);
    dlld->dataLinkLayerSource=dlls;
    BufferPool_Init(&dlld->bufferPool,DLDestPool_Creator);
    INIT_DELAYED_WORK(&dlld->timeout,DLDestPool_Timeout);
    GetNetworkLayer()->InitDataLinkLayer(&dlld->ddl);
    list_add(&dlld->list,&dlls->destinations);
    mutex_unlock(&dlls->lock);
Update:
    schedule_delayed_work(&dlld->timeout,msecs_to_jiffies(DEST_TIMEOUT_MS));
    return dlld;
}

CBuildSignature(int,Send,(struct sk_buff*skb)){ return dev_queue_xmit(skb); }

CBuildSignature(struct sk_buff*,New,(void*dataLinkLayer,u16*value)){
    struct DataLinkLayerDestination *dlld=(struct DataLinkLayerDestination*)dataLinkLayer;
    for(u8 i=0;i<SOCKET_POOLS;i++){
        struct SocketBufferPool *sockPool=&dlld->bufferPool.socketBufferPools[i];
        if(mutex_trylock(&sockPool->lock)){
            if(!sockPool->count){ mutex_unlock(&sockPool->lock); continue; }
            struct sk_buff *skb=list_first_entry(&sockPool->data,struct sk_buff,list);
            list_del(&skb->list);
            sockPool->count--;
            mutex_unlock(&sockPool->lock);
            if(!sockPool->count && work_pending(&dlld->bufferPool.creator)) schedule_work(&dlld->bufferPool.creator);
            schedule_delayed_work(&dlld->timeout,msecs_to_jiffies(DEST_TIMEOUT_MS));
            schedule_delayed_work(&dlld->dataLinkLayerSource->timeout,msecs_to_jiffies(SOURCE_TIMEOUT_MS));
            return skb;
        }
    }
    struct sk_buff *skb=DLSourcePool_GetBuffer(dlld->dataLinkLayerSource);
    if(!skb) return NULL;
    memcpy(skb_mac_header(skb),dlld->Address,6);
    if(work_pending(&dlld->bufferPool.creator)){
        mutex_lock(&dlld->bufferPool.lock);
        dlld->bufferPool.increaser++;
        dlld->bufferPool.timestamp=ktime_get();
        mutex_unlock(&dlld->bufferPool.lock);
        schedule_delayed_work(&dlld->bufferPool.decrease,msecs_to_jiffies(DECREASE_MS));
        schedule_work(&dlld->bufferPool.creator);
    }
    schedule_delayed_work(&dlld->timeout,msecs_to_jiffies(DEST_TIMEOUT_MS));
    if(value) memcpy(skb_put(skb,2),&(u16){htons(*value)},2);
    return skb;
}

CBuildSignature(int,DLReceiver,(struct sk_buff*skb,struct net_device*dev,struct packet_type*pt,struct net_device*orig_dev)){
    if(skb->pkt_type==PACKET_OUTGOING) return NET_RX_SUCCESS;
    struct DataLinkLayerSource *dlls=container_of(pt,struct DataLinkLayerSource,pt);
    schedule_delayed_work(&dlls->timeout,msecs_to_jiffies(SOURCE_TIMEOUT_MS));
    void *dlld=DLInitDestination(dlls,(u8(*)[6])(skb_mac_header(skb)+6));
    if(!dlld){ kfree(skb); return NET_RX_DROP; }
    switch(GetNetworkLayer()->Receiver(skb,dlld)){
        case DataLinkLayerPacketDropAndFree: kfree(skb);
        case DataLinkLayerPacketDropAndKeep: return NET_RX_DROP;
        default: return NET_RX_SUCCESS;
    }
}

CBuildSignature(u16,GetEthertype,(struct sk_buff*skb)){ return ntohs(*(u16*)(skb_mac_header(skb)+12)); }
CBuildSignature(void,SetEthertype,(struct sk_buff*skb,u16*value)){ memcpy(skb_put(skb,2),&(u16){htons(*value)},2); }

CBuildInit{
    struct net_device *dev;
    rcu_read_lock();
    for_each_netdev(&init_net,dev){
        if(dev->type==ARPHRD_ETHER){
            struct DataLinkLayerSource *dlls=kmalloc(sizeof(*dlls),GFP_KERNEL);
            if(!dlls) continue;
            dlls->pt.type=htons(ETH_P_ALL);
            dlls->pt.dev=dev;
            dlls->pt.func=DLReceiver;
            mutex_init(&dlls->lock);
            INIT_DELAYED_WORK(&dlls->timeout,DLSourcePool_Timeout);
            INIT_LIST_HEAD(&dlls->destinations);
            INIT_LIST_HEAD(&dlls->list);
            BufferPool_Init(&dlls->bufferPool,DLSourcePool_Creator);
            list_add(&dlls->list,&DLSourcePoolCreators);
            dev_add_pack(&dlls->pt);
        }
    }
    rcu_read_unlock();
    synchronize_net();
    return 0;
}

CBuildExit{
    struct DataLinkLayerSource *dlls,*tmp;
    list_for_each_entry_safe(dlls,tmp,&DLSourcePoolCreators,list){
        dev_remove_pack(&dlls->pt);
        cancel_delayed_work_sync(&dlls->timeout);
        DLSourcePool_Free(dlls,true);
        list_del(&dlls->list);
        kfree(dlls);
    }
    synchronize_net();
    return 0;
}

CBuildStart(DataLinkLayer)
    CBuildBind(New)
    CBuildBind(Send)
    CBuildBind(GetEthertype)
CBuildEnd
