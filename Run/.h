#ifndef Run_h
#define Run_h
#include <linux/module.h>
#include <linux/kernel.h>

#define Void static void

#ifdef MODULE
    #define LinuxRun(x) module_init(x)
#else
    #define LinuxRun(x) late_initcall(x)
#endif


#define RunInit(description)\
    Void ModuleInit(void);\
    MODULE_DESCRIPTION(#description);\
    MODULE_LICENSE("GPL");\
    MODULE_AUTHOR("WeMakeSoftware");\
    static int __init LinuxRunInit(void){\
        ModuleInit();\
        return 0;\
    }\
    LinuxRun(LinuxRunInit);\
    Void ModuleInit(void)

#endif
