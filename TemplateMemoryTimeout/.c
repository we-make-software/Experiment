#include ".h"
struct TemplateMemoryTimeoutBinding{
    struct list_head bind;
    struct TemplateMemoryTimeout*root;
};
static struct kmem_cache*TmeOutBindingCachie;
static struct list_head Timer[60];
static struct delayed_work DelayedWork;
static u8 LastTick;
CBuildSignature(void,EventWork,(u8 *tick)){
    struct TemplateMemoryTimeout*t,*tmp;
    list_for_each_entry_safe(t,tmp,&Timer[*tick],timer) {
        mutex_lock(&t->lock);
        list_del_init(&t->timer);
        mutex_unlock(&t->lock);
        if(LinuxGetPowerStatus())
            schedule_work(&t->work);
        else
            t->function(t);
    }
}
CBuildSignature(void,EventFunction,(struct work_struct*work)){
    struct TemplateMemoryTimeout*TemplateMemoryTimeout=container_of(work, struct TemplateMemoryTimeout, work);
    TemplateMemoryTimeout->function(TemplateMemoryTimeout);
}
CBuildSignature(void,EventTrigger,(void)){
    EventTriggerCode
}
CBuildSignature(u8,GetIndexTick,(ktime_t*time)){
    return IndexTickCode
}
CBuildSignature(u8,GetNowIndexTick,(void)){
    return GetIndexTick(&(ktime_t){ ktime_get() });
}
CBuildSignature(void,Disable,(struct TemplateMemoryTimeout*root)){
    mutex_lock(&root->lock);
    list_del_init(&root->timer);
    mutex_unlock(&root->lock);
}
CBuildSignature(void,Tick,(struct TemplateMemoryTimeout*root,u8*tick)){
    if(!LinuxGetPowerStatus()||!root||!tick||*tick>60)return;
    Disable(root);
    mutex_lock(&root->lock);
    root->tick = *tick; 
    list_add(&root->timer, &Timer[GetIndexTick(&(ktime_t){ktime_add_ns(ktime_get(), (u64)(*tick)*60000000000ULL)})]);   
    mutex_unlock(&root->lock);  
    struct TemplateMemoryTimeoutBinding *iter, *tmp;
    list_for_each_entry_safe(iter, tmp, &root->binding, bind)
        Tick(iter->root, &iter->root->tick);
}
CBuildSignature(void,Function,(struct TemplateMemoryTimeout*root,void(*function)(struct TemplateMemoryTimeout*))){
    root->function=function;  
    Tick(root,&root->tick); 
}
CBuildSignature(void,Unbinding,(struct TemplateMemoryTimeout*root,struct TemplateMemoryTimeout*bind)){
    struct TemplateMemoryTimeoutBinding *b, *tmp;
    mutex_lock(&root->lock);
    list_for_each_entry_safe(b, tmp, &root->binding, bind){
        if(b->root==bind){
            list_del_init(&b->bind);
            kmem_cache_free(TmeOutBindingCachie, b);
            break;
        }
    }
    mutex_unlock(&root->lock);
}
CBuildSignature(void,UnbindingAll,(struct TemplateMemoryTimeout*root)){
    struct TemplateMemoryTimeoutBinding *b, *tmp;
    mutex_lock(&root->lock);
    list_for_each_entry_safe(b, tmp, &root->binding, bind){
        list_del_init(&b->bind);
        kmem_cache_free(TmeOutBindingCachie, b);
    }
    mutex_unlock(&root->lock);
}
CBuildSignature(void,Binding,(struct TemplateMemoryTimeout*root,struct TemplateMemoryTimeout*bind)){
    struct TemplateMemoryTimeoutBinding *b, *iter, *tmp;
    if(!LinuxGetPowerStatus()||!bind||!root||bind==root)return;
    list_for_each_entry(b, &root->binding,bind)
        if(b->root==bind)return;
    mutex_lock(&root->lock);
    b=kmem_cache_alloc(TmeOutBindingCachie, GFP_KERNEL);
    if(!b){mutex_unlock(&root->lock);return;}
    INIT_LIST_HEAD(&b->bind);
    b->root=bind;
    list_add(&b->bind, &root->binding);
    mutex_unlock(&root->lock);  
    Tick(root, &root->tick); 
    list_for_each_entry_safe(iter, tmp, &root->binding, bind)
        Tick(iter->root, &iter->root->tick);   
}
CBuildSignature(void,BindingCloning,(struct TemplateMemoryTimeout*root,struct TemplateMemoryTimeout*bind)){
    struct TemplateMemoryTimeoutBinding *b;
    list_for_each_entry(b, &bind->binding, bind)
        Binding(root, b->root);
}
CBuildSignature(bool,New,(struct TemplateMemoryTimeout*root,u8*tick,void(*function)(struct TemplateMemoryTimeout*))){
    if(!tick||*tick>60)return false;
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
    u8 SnapNowTick=GetNowIndexTick();
    if(SnapNowTick<LastTick){
        for(LastTick++;LastTick<60;LastTick++)
            EventWork(&LastTick);
        LastTick=255;
    }
    bool Decrease=false;
    for(LastTick++;LastTick<=SnapNowTick;LastTick++){
        EventWork(&LastTick);
        Decrease=true;
    }
    if(Decrease)
        LastTick--;
    EventTrigger();
}
CBuildInit{
    TmeOutBindingCachie=kmem_cache_create(TOSTRING(TemplateMemoryTimeout),sizeof(struct TemplateMemoryTimeoutBinding),0,SLAB_HWCACHE_ALIGN,NULL);
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
    kmem_cache_destroy(TmeOutBindingCachie);    
    return 0;
}
CBuildStart(TemplateTemplateMemoryTimeout)
    CBuildBind(Disable)
    CBuildBind(Tick)
    CBuildBind(Function)
    CBuildBind(Unbinding)
    CBuildBind(UnbindingAll)
    CBuildBind(Binding)
    CBuildBind(BindingCloning)
    CBuildBind(New)
CBuildEnd