#include ".h"
struct MinuteMemoryTimeoutRecordBinding{
    struct list_head bind;
    struct MinuteMemoryTimeoutRecord*root;
};
Struct kmem_cache*TimeOutBindingCachie,*evtCachie;
Struct list_head Timer[60];
Struct delayed_work DelayedWork;
static u8 LastTick;


CBuildSignature(void,ExtraTrigger,(struct MinuteMemoryTimeoutRecord* root)){
    struct EventFunctionTimeout *entry, *tmp;
    mutex_lock(&root->lock);
    list_for_each_entry_safe(entry,tmp,&root->evt,lh) {
        list_del_init(&entry->lh);
        mutex_unlock(&root->lock);
        if (entry->function)
            entry->function(entry->data);
        kmem_cache_free(evtCachie, entry);
        mutex_lock(&root->lock);
    }
    mutex_unlock(&root->lock);
}
CBuildSignature(void,ExtraAdd,(struct MinuteMemoryTimeoutRecord*root,void(*function)(void*data),void*data,void*(*duplicationAction)(void*oldData,void*newData))){
    mutex_lock(&root->lock);
    struct EventFunctionTimeout *entry;
     list_for_each_entry(entry, &root->evt, lh) {
        if (entry->function == function) {
            if(entry->data!=data){
                if(!entry->data)
                    entry->data=data;
                else if(!entry->data&&!data)
                    entry->data=duplicationAction(entry->data,data);
            }   
            mutex_unlock(&root->lock);
            return;
        }
    }
    entry=kmem_cache_alloc(evtCachie,GFP_KERNEL);
    if (!entry) {
        mutex_unlock(&root->lock);
        return;
    }
    entry->function=function;
    entry->data=data;
    INIT_LIST_HEAD(&entry->lh);
    list_add(&entry->lh, &root->evt);
    mutex_unlock(&root->lock);
}
CBuildSignature(void, ExtraDelete,(struct MinuteMemoryTimeoutRecord*root,void(*function)(void *data))) {
    struct EventFunctionTimeout *entry, *tmp;
    mutex_lock(&root->lock);
    list_for_each_entry_safe(entry, tmp, &root->evt, lh) {
        if (entry->function == function) {
            list_del_init(&entry->lh);
            kmem_cache_free(evtCachie, entry);
        }
    }
    mutex_unlock(&root->lock);
}

CBuildSignature(void,ExtraDeleteAll,(struct MinuteMemoryTimeoutRecord*root)){
    struct EventFunctionTimeout*entry,*tmp;
    mutex_lock(&root->lock);
    list_for_each_entry_safe(entry, tmp, &root->evt, lh) {
        list_del_init(&entry->lh);
        kmem_cache_free(evtCachie, entry);
    }
    mutex_unlock(&root->lock);
}


CBuildSignature(void,EventWork,(u8 *tick)){
    struct MinuteMemoryTimeoutRecord*t,*tmp;
    list_for_each_entry_safe(t,tmp,&Timer[*tick],timer) {
        mutex_lock(&t->lock);
        list_del_init(&t->timer);
        mutex_unlock(&t->lock);
        if(LinuxGetPowerStatus())
            schedule_work(&t->work);
        else{
            ExtraTrigger(t);
            t->function(t);
        }
    }
}
CBuildSignature(void,EventFunction,(struct work_struct*work)){
    struct MinuteMemoryTimeoutRecord*mmtr=container_of(work, struct MinuteMemoryTimeoutRecord, work);
    ExtraTrigger(mmtr);
    mmtr->function(mmtr);
}
CBuildSignature(void,EventTrigger,(void)){
    struct timespec64 ts;\
    ktime_get_real_ts64(&ts);\
    schedule_delayed_work(&DelayedWork,(60-ts.tv_sec % 60)*HZ);
}
CBuildSignature(u8,GetIndexTick,(ktime_t*time)){
    return (ktime_to_ns(*time)/(60000000000ULL))%60;
}
CBuildSignature(u8,GetNowIndexTick,(void)){
    return GetIndexTick(&(ktime_t){ ktime_get() });
}
CBuildSignature(void,Disable,(struct MinuteMemoryTimeoutRecord*root)){
    mutex_lock(&root->lock);
    list_del_init(&root->timer);
    mutex_unlock(&root->lock);
}
CBuildSignature(void,Tick,(struct MinuteMemoryTimeoutRecord*root,u8*tick)){
    if(!LinuxGetPowerStatus()||!root||!tick||*tick>60)return;
    Disable(root);
    mutex_lock(&root->lock);
    root->tick = *tick; 
    list_add(&root->timer, &Timer[GetIndexTick(&(ktime_t){ktime_add_ns(ktime_get(), (u64)(*tick)*60000000000ULL)})]);   
    mutex_unlock(&root->lock);  
    struct MinuteMemoryTimeoutRecordBinding *iter, *tmp;
    list_for_each_entry_safe(iter, tmp, &root->binding, bind)
        Tick(iter->root, &iter->root->tick);
}
CBuildSignature(void,Update,(struct MinuteMemoryTimeoutRecord*root)){
    Tick(root, &root->tick); 
}
CBuildSignature(void,Function,(struct MinuteMemoryTimeoutRecord*root,void(*function)(struct MinuteMemoryTimeoutRecord*))){
    root->function=function;  
    Tick(root,&root->tick); 
}
CBuildSignature(void,Unbinding,(struct MinuteMemoryTimeoutRecord*root,struct MinuteMemoryTimeoutRecord*bind)){
    struct MinuteMemoryTimeoutRecordBinding *b, *tmp;
    mutex_lock(&root->lock);
    list_for_each_entry_safe(b, tmp, &root->binding, bind){
        if(b->root==bind){
            list_del_init(&b->bind);
            kmem_cache_free(TimeOutBindingCachie, b);
            break;
        }
    }
    mutex_unlock(&root->lock);
}
CBuildSignature(void,UnbindingAll,(struct MinuteMemoryTimeoutRecord*root)){
    struct MinuteMemoryTimeoutRecordBinding *b, *tmp;
    mutex_lock(&root->lock);
    list_for_each_entry_safe(b, tmp, &root->binding, bind){
        list_del_init(&b->bind);
        kmem_cache_free(TimeOutBindingCachie, b);
    }
    mutex_unlock(&root->lock);
}
CBuildSignature(void,Binding,(struct MinuteMemoryTimeoutRecord*root,struct MinuteMemoryTimeoutRecord*bind)){
    struct MinuteMemoryTimeoutRecordBinding *b, *iter, *tmp;
    if(!LinuxGetPowerStatus()||!bind||!root||bind==root)return;
    list_for_each_entry(b, &root->binding,bind)
        if(b->root==bind)return;
    mutex_lock(&root->lock);
    b=kmem_cache_alloc(TimeOutBindingCachie, GFP_KERNEL);
    if(!b){mutex_unlock(&root->lock);return;}
    INIT_LIST_HEAD(&b->bind);
    b->root=bind;
    list_add(&b->bind, &root->binding);
    mutex_unlock(&root->lock);  
    Tick(root, &root->tick); 
    list_for_each_entry_safe(iter, tmp, &root->binding, bind)
        Tick(iter->root, &iter->root->tick);   
}
CBuildSignature(void,BindingCloning,(struct MinuteMemoryTimeoutRecord*root,struct MinuteMemoryTimeoutRecord*bind)){
    struct MinuteMemoryTimeoutRecordBinding *b;
    list_for_each_entry(b, &bind->binding, bind)
        Binding(root, b->root);
}
CBuildSignature(bool,New,(struct MinuteMemoryTimeoutRecord*root,u8*tick,void(*function)(struct MinuteMemoryTimeoutRecord*))){
    if(!tick||*tick>60)return false;
    mutex_init(&root->lock);
    if(LinuxGetPowerStatus()){
        Tick(root,tick);
        Function(root,function);
        INIT_WORK(&root->work,EventFunction);
        INIT_LIST_HEAD(&root->timer);
        INIT_LIST_HEAD(&root->binding);
         INIT_LIST_HEAD(&root->evt);
        return true;
    }
    else{
       function(root);
       return false;
    }
}
CBuildSignature(void,EventHandler,(struct work_struct*)){
    u8 SnapNowTick=GetNowIndexTick();
    if(SnapNowTick<LastTick){
        for(LastTick++;LastTick<60;LastTick++)
            EventWork(&LastTick);
        LastTick=255;
    }
    for(LastTick++;LastTick<=SnapNowTick;LastTick++)
         EventWork(&LastTick);
    if(LastTick>SnapNowTick)
       LastTick=SnapNowTick;
    EventTrigger();
}
CBuildInit{
    TimeOutBindingCachie=kmem_cache_create("MinuteMemoryTimeoutRecord",sizeof(struct MinuteMemoryTimeoutRecordBinding),0,SLAB_HWCACHE_ALIGN,NULL);
    evtCachie=kmem_cache_create("EVTMinuteMemoryTimeoutRecord",sizeof(struct EventFunctionTimeout),0,SLAB_HWCACHE_ALIGN,NULL);
    for(u8 i=0;i<60;i++)
        INIT_LIST_HEAD(&Timer[i]);
    INIT_DELAYED_WORK(&DelayedWork,EventHandler);
    LastTick=GetNowIndexTick();
    EventTrigger();    
    return 0;
}
CBuildExit{
    cancel_delayed_work_sync(&DelayedWork);
    for(u8 i=0;i<60;i++){
        LastTick++;
        if(LastTick==60)
           LastTick=0;
        EventWork(&LastTick);
    }
    kmem_cache_destroy(TimeOutBindingCachie);   
    kmem_cache_destroy(evtCachie);    
    return 0;
}
CBuildStart(MinuteMemoryTimeout)
    CBuildBind(ExtraDeleteAll)
    CBuildBind(ExtraDelete)
    CBuildBind(ExtraAdd)
    CBuildBind(Update)
    CBuildBind(Disable)
    CBuildBind(Tick)
    CBuildBind(Function)
    CBuildBind(Unbinding)
    CBuildBind(UnbindingAll)
    CBuildBind(Binding)
    CBuildBind(BindingCloning)
    CBuildBind(New)
CBuildEnd


