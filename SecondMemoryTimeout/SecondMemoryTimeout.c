#include ".h"
struct SecondMemoryTimeoutRecordBinding{
    struct list_head bind;
    struct SecondMemoryTimeoutRecord*root;
};
Struct kmem_cache*TimeOutBindingCachie,*evtCachie;
Struct list_head Timer[3600];
Struct delayed_work DelayedWork;
static u16 LastTick;
CBuildSignature(void,ExtraTrigger,(struct SecondMemoryTimeoutRecord* root)){
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
CBuildSignature(void,ExtraAdd,(struct SecondMemoryTimeoutRecord*root,void(*function)(void*data),void*data,void*(*duplicationAction)(void*oldData,void*newData))){
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
CBuildSignature(void,ExtraDelete,(struct SecondMemoryTimeoutRecord*root,void(*function)(void *data))) {
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
CBuildSignature(void,ExtraDeleteAll,(struct SecondMemoryTimeoutRecord*root)){
    struct EventFunctionTimeout*entry,*tmp;
    mutex_lock(&root->lock);
    list_for_each_entry_safe(entry, tmp, &root->evt, lh) {
        list_del_init(&entry->lh);
        kmem_cache_free(evtCachie, entry);
    }
    mutex_unlock(&root->lock);
}
CBuildSignature(void,EventWork,(u16 *tick)){
    struct SecondMemoryTimeoutRecord*t,*tmp;
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
    struct SecondMemoryTimeoutRecord*sntr=container_of(work, struct SecondMemoryTimeoutRecord, work);
    ExtraTrigger(sntr);
    sntr->function(sntr);
}
CBuildSignature(void,EventTrigger,(void)){
    schedule_delayed_work(&DelayedWork, HZ);
}
CBuildSignature(u16, GetIndexTick,(ktime_t *time)) {
    return (ktime_to_ns(*time) / 1000000000ULL) % 3600;
}
CBuildSignature(u16,GetNowIndexTick,(void)){
    return GetIndexTick(&(ktime_t){ ktime_get() });
}
CBuildSignature(void,Disable,(struct SecondMemoryTimeoutRecord*root)){
    mutex_lock(&root->lock);
    list_del_init(&root->timer);
    mutex_unlock(&root->lock);
}
CBuildSignature(void,Tick,(struct SecondMemoryTimeoutRecord*root,u8*tick)){
    if(!LinuxGetPowerStatus()||!root||!tick||*tick>3600)return;
    Disable(root);
    mutex_lock(&root->lock);
    root->tick = *tick; 
    list_add(&root->timer, &Timer[GetIndexTick(&(ktime_t){ktime_add_ns(ktime_get(), (u64)(*tick)*60000000000ULL)})]);   
    mutex_unlock(&root->lock);  
    struct SecondMemoryTimeoutRecordBinding *iter, *tmp;
    list_for_each_entry_safe(iter, tmp, &root->binding, bind)
        Tick(iter->root, &iter->root->tick);
}

CBuildSignature(void,Update,(struct SecondMemoryTimeoutRecord*root)){
    Tick(root, &root->tick); 
}

CBuildSignature(void,Function,(struct SecondMemoryTimeoutRecord*root,void(*function)(struct SecondMemoryTimeoutRecord*))){
    root->function=function;  
    Tick(root,&root->tick); 
}
CBuildSignature(void,Unbinding,(struct SecondMemoryTimeoutRecord*root,struct SecondMemoryTimeoutRecord*bind)){
    struct SecondMemoryTimeoutRecordBinding *b, *tmp;
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
CBuildSignature(void,UnbindingAll,(struct SecondMemoryTimeoutRecord*root)){
    struct SecondMemoryTimeoutRecordBinding *b, *tmp;
    mutex_lock(&root->lock);
    list_for_each_entry_safe(b, tmp, &root->binding, bind){
        list_del_init(&b->bind);
        kmem_cache_free(TimeOutBindingCachie, b);
    }
    mutex_unlock(&root->lock);
}
CBuildSignature(void,Binding,(struct SecondMemoryTimeoutRecord*root,struct SecondMemoryTimeoutRecord*bind)){
    struct SecondMemoryTimeoutRecordBinding *b, *iter, *tmp;
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
CBuildSignature(void,BindingCloning,(struct SecondMemoryTimeoutRecord*root,struct SecondMemoryTimeoutRecord*bind)){
    struct SecondMemoryTimeoutRecordBinding *b;
    list_for_each_entry(b, &bind->binding, bind)
        Binding(root, b->root);
}
CBuildSignature(bool,New,(struct SecondMemoryTimeoutRecord*root,u16*tick,void(*function)(struct SecondMemoryTimeoutRecord*))){
    if(!tick||*tick>3600)return false;
    mutex_init(&root->lock);
    if(LinuxGetPowerStatus()){
        Tick(root,tick);
        Function(root,function);
        INIT_WORK(&root->work,EventFunction);
        INIT_LIST_HEAD(&root->timer);
        INIT_LIST_HEAD(&root->binding);
        return true;
    }
    else{
       function(root);
       return false;
    }
}
CBuildSignature(void,EventHandler,(struct work_struct*)){
    u16 SnapNowTick=GetNowIndexTick();
    if(SnapNowTick<LastTick){
        for(LastTick++;LastTick<3600;LastTick++)
            EventWork(&LastTick);
        LastTick=65535;
    }
    for(LastTick++;LastTick<=SnapNowTick;LastTick++)
        EventWork(&LastTick);
    if(LastTick>SnapNowTick)
       LastTick=SnapNowTick;
    EventTrigger();
}
CBuildInit{
    TimeOutBindingCachie=kmem_cache_create("SecondMemoryTimeoutRecord",sizeof(struct SecondMemoryTimeoutRecordBinding),0,SLAB_HWCACHE_ALIGN,NULL);
    evtCachie=kmem_cache_create("EVTSecondMemoryTimeoutRecord",sizeof(struct EventFunctionTimeout),0,SLAB_HWCACHE_ALIGN,NULL);
    for(u16 i=0;i<3600;i++)
        INIT_LIST_HEAD(&Timer[i]);
    INIT_DELAYED_WORK(&DelayedWork,EventHandler);
    LastTick=GetNowIndexTick();
    EventTrigger();    
    return 0;
}
CBuildExit{
    cancel_delayed_work_sync(&DelayedWork);
    for(u16 i=0;i<3600;i++){
        LastTick++;
        if(LastTick==3600)
           LastTick=0;
        EventWork(&LastTick);
    }
    kmem_cache_destroy(TimeOutBindingCachie);    
    kmem_cache_destroy(evtCachie);   
    return 0;
}
CBuildStart(SecondMemoryTimeout)
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


