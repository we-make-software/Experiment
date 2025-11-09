#define System_h_declarations
#include ".h"
#include <linux/reboot.h>
#define DefaultExportSymbol(type,name,signature)\
    type name signature;\
    EXPORT_SYMBOL(name);\
    type name signature

struct FunctionPayload{
    struct list_head List;
    int(*Function)(int);
    int State;
    unsigned char*Name;
};
struct ApplicationProgrammingInterfacePayload{
    struct list_head List;
    void*(*ApplicationProgrammingInterface);
    unsigned char*Name;
};
static SystemProcessType SystemProcess=SystemProcessLoading;
DefaultExportSymbol(SystemProcessType*,GetSystemProcess,(void)){
    return &SystemProcess;
}
static LIST_HEAD(InitPayloadList);
static LIST_HEAD(ExitPayloadList);
static LIST_HEAD(ApplicationProgrammingInterfacePayloadList);
DefaultExportSymbol(void, SystemBuildRemover,(unsigned char* name)) {
    struct FunctionPayload*entry,*tmp;
    struct ApplicationProgrammingInterfacePayload*apiEntry,*apiTmp;
    list_for_each_entry_safe(entry, tmp, &ExitPayloadList, List) {
        if(strcmp(entry->Name, name)==0) {
            entry->Function(-2); 
            list_del(&entry->List);
            kfree(entry);
            break;
        }
    }
    list_for_each_entry_safe(apiEntry,apiTmp,&ApplicationProgrammingInterfacePayloadList, List) {
        if (strcmp(apiEntry->Name,name)==0) {
            list_del(&apiEntry->List);
            kfree(apiEntry);
            break;
        }
    }
}
DefaultExportSymbol(int,SystemGetInitState,(unsigned char*name)){
    struct FunctionPayload *entry;
    list_for_each_entry(entry,&InitPayloadList,List) {
        if(strcmp(entry->Name,name)==0)
            return entry->State;
    }
    return -1;
}
DefaultExportSymbol(int,SystemGetExitState,(unsigned char*name)){
    struct FunctionPayload *entry;
    list_for_each_entry(entry,&ExitPayloadList,List) {
        if(strcmp(entry->Name,name)==0)
            return entry->State;
    }
    return -1;
}
DefaultExportSymbol(void*,SystemGetApplicationProgrammingInterface,(unsigned char*name)){
    struct ApplicationProgrammingInterfacePayload *entry;
    list_for_each_entry(entry,&ApplicationProgrammingInterfacePayloadList,List) {
        if(strcmp(entry->Name,name)==0)
            return entry->ApplicationProgrammingInterface;
    }
    return NULL;
}
Void SystemExit(void){
    printk(KERN_INFO "Run module unloaded: Goodbye from kernel!\n");
    while(!list_empty(&ExitPayloadList)){   
        struct FunctionPayload *entry, *tmp;
        list_for_each_entry_safe(entry,tmp,&ExitPayloadList,List) {
            if(!(entry->State=entry->Function(entry->State))){
                list_del(&entry->List);
                kfree(entry);
                continue;
            }
            list_move_tail(&entry->List, &ExitPayloadList);
        }
    }
}
static LinuxPowerStatusType LinuxPowerStatus=PowerStatusOn;

static void(*networkStart)(void)=NULL;
DefaultExportSymbol(void,NetworkInit,(void(*function)(void))){
    networkStart=function;
}
DefaultExportSymbol(void,LinuxStart,(void)){
    LinuxPowerStatus=PowerStatusOn;
    while(!list_empty(&InitPayloadList)){   
        struct FunctionPayload *entry, *tmp;
        list_for_each_entry_safe(entry,tmp,&InitPayloadList,List) {
            if(!(entry->State=entry->Function(entry->State))){
                list_del(&entry->List);
                kfree(entry);
                continue;
            }
            list_move_tail(&entry->List, &InitPayloadList);
        }
    }
    SystemProcess=SystemProcessComplete;
    networkStart();
    networkStart=NULL;
}  
DefaultExportSymbol(void,SystemInit,(unsigned char*name,void*ApplicationProgrammingInterface,int(*Init)(int),int(*Exit)(int))){
    struct ApplicationProgrammingInterfacePayload*applicationProgrammingInterfacePayload=kmalloc(sizeof(struct ApplicationProgrammingInterfacePayload),GFP_KERNEL);
    applicationProgrammingInterfacePayload->ApplicationProgrammingInterface=ApplicationProgrammingInterface;
    applicationProgrammingInterfacePayload->Name=name;
    list_add_tail(&applicationProgrammingInterfacePayload->List,&ApplicationProgrammingInterfacePayloadList);
    struct FunctionPayload*initPayload=kmalloc(sizeof(struct FunctionPayload),GFP_KERNEL);
    initPayload->Function=Init;
    initPayload->State=1;
    initPayload->Name=name;
    list_add_tail(&initPayload->List,&InitPayloadList);
    struct FunctionPayload*exitPayload=kmalloc(sizeof(struct FunctionPayload),GFP_KERNEL);
    exitPayload->Function=Exit;
    exitPayload->State=1;
    exitPayload->Name=name;
    list_add_tail(&exitPayload->List,&ExitPayloadList);
    if(SystemProcess)
        LinuxStart();
}    
DefaultExportSymbol(LinuxPowerStatusType*,LinuxGetPowerStatus,(void)){
    return &LinuxPowerStatus;
}
DefaultExportSymbol(void,LinuxExit,(void)){
    if(LinuxPowerStatus==PowerStatusOn){
        LinuxPowerStatus=PowerStatusOff;
        SystemExit();
    }
}
DefaultExportSymbol(void,LinuxShutdown,(void)){
    if(LinuxPowerStatus==PowerStatusOn){
        LinuxPowerStatus=PowerStatusOff;
        LinuxExit();
        kernel_power_off();
    }
}
DefaultExportSymbol(void,LinuxReboot,(void)){
    if(LinuxPowerStatus==PowerStatusOn){
        LinuxPowerStatus=PowerStatusOff;
        LinuxExit();
        kernel_restart(NULL);
    }
}
RunInit(System){}
static void __exit LinuxKernelObjectExit(void){}
module_exit(LinuxKernelObjectExit);