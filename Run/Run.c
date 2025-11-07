#include ".h"
extern void LinuxStart(void);
extern void LinuxExit(void);
#ifndef MODULE
#include <linux/notifier.h>
#include <linux/reboot.h>
    static int LinuxExitInit(struct notifier_block *nb, unsigned long action, void *data){
        LinuxExit();
        return NOTIFY_OK;
    }
    static struct notifier_block NotifierLinuxExit= {.notifier_call = LinuxExitInit};
#else
    Void __exit LinuxExitInit(void){ 
        LinuxExit();
    }
    module_exit(LinuxExitInit);
#endif
RunInit(Run){
    #ifndef MODULE
        register_reboot_notifier(&NotifierLinuxExit);
    #endif
    LinuxStart();
}