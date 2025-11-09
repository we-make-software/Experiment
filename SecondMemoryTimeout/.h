#ifndef SecondMemoryTimeout_H
#define SecondMemoryTimeout_H
#include "../System/.h"
#include <linux/slab.h>  
struct SecondMemoryTimeoutRecord{
    struct list_head timer,binding,evt;   
    struct work_struct work; 
    void(*function)(struct SecondMemoryTimeoutRecord*); 
    u16 tick;
    struct mutex lock;
};
    HBuildStart(SecondMemoryTimeout)
        HBuildSignature(void,ExtraDeleteAll,(struct SecondMemoryTimeoutRecord*root))
        HBuildSignature(void,ExtraDelete,(struct SecondMemoryTimeoutRecord*root,void(*function)(void *data)))
        HBuildSignature(void,ExtraAdd,(struct SecondMemoryTimeoutRecord*root,void(*function)(void*data),void*data,void*(*duplicationAction)(void*oldData,void*newData)))
        HBuildSignature(void,Update,(struct SecondMemoryTimeoutRecord*root))
        HBuildSignature(void,Disable,(struct SecondMemoryTimeoutRecord*root))
        HBuildSignature(void,Tick,(struct SecondMemoryTimeoutRecord*root,u16*tick))
        HBuildSignature(void,Function,(struct SecondMemoryTimeoutRecord*root,void(*function)(struct SecondMemoryTimeoutRecord*)))
        HBuildSignature(void,Unbinding,(struct SecondMemoryTimeoutRecord*root,struct SecondMemoryTimeoutRecord*bind))
        HBuildSignature(void,UnbindingAll,(struct SecondMemoryTimeoutRecord*root))
        HBuildSignature(void,Binding,(struct SecondMemoryTimeoutRecord*root,struct SecondMemoryTimeoutRecord*bind))
        HBuildSignature(void,BindingCloning,(struct SecondMemoryTimeoutRecord*root,struct SecondMemoryTimeoutRecord*bind))
        HBuildSignature(bool,New,(struct SecondMemoryTimeoutRecord*root,u16*tick,void(*function)(struct SecondMemoryTimeoutRecord*)))
    HBuildEnd
    #define CBuildConnectSecondMemoryTimeout\
        CBuildConnectApplicationProgrammingInterface(SecondMemoryTimeout)
#endif