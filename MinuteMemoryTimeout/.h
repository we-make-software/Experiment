#ifndef MinuteMemoryTimeout_H
#define MinuteMemoryTimeout_H
#include "../System/.h"
#include <linux/slab.h>  
    struct MinuteMemoryTimeoutRecord{
        struct list_head timer,binding,evt;   
        struct work_struct work; 
        void(*function)(struct MinuteMemoryTimeoutRecord*); 
        u8 tick;
        struct mutex lock;
    };
    HBuildStart(MinuteMemoryTimeout)
        HBuildSignature(void,ExtraDeleteAll,(struct MinuteMemoryTimeoutRecord*root))
        HBuildSignature(void,ExtraDelete,(struct MinuteMemoryTimeoutRecord*root,void(*function)(void *data)))
        HBuildSignature(void,ExtraAdd,(struct MinuteMemoryTimeoutRecord*root,void(*function)(void*data),void*data,void*(*duplicationAction)(void*oldData,void*newData)))
        HBuildSignature(void,Update,(struct MinuteMemoryTimeoutRecord*root))
        HBuildSignature(void,Disable,(struct MinuteMemoryTimeoutRecord*root))
        HBuildSignature(void,Tick,(struct MinuteMemoryTimeoutRecord*root,u8*tick))
        HBuildSignature(void,Function,(struct MinuteMemoryTimeoutRecord*root,void(*function)(struct MinuteMemoryTimeoutRecord*)))
        HBuildSignature(void,Unbinding,(struct MinuteMemoryTimeoutRecord*root,struct MinuteMemoryTimeoutRecord*bind))
        HBuildSignature(void,UnbindingAll,(struct MinuteMemoryTimeoutRecord*root))
        HBuildSignature(void,Binding,(struct MinuteMemoryTimeoutRecord*root,struct MinuteMemoryTimeoutRecord*bind))
        HBuildSignature(void,BindingCloning,(struct MinuteMemoryTimeoutRecord*root,struct MinuteMemoryTimeoutRecord*bind))
        HBuildSignature(bool,New,(struct MinuteMemoryTimeoutRecord*root,u8*tick,void(*function)(struct MinuteMemoryTimeoutRecord*)))
    HBuildEnd

    #define CBuildConnectMinuteMemoryTimeout\
            CBuildConnectApplicationProgrammingInterface(MinuteMemoryTimeout)

#endif