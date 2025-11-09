#ifndef System_h
#define System_h
#include "../Run/.h"
typedef enum {
    PowerStatusOff=0,
    PowerStatusOn=1,
} LinuxPowerStatusType;
typedef enum{
    SystemProcessLoading=0,
    SystemProcessComplete=1
} SystemProcessType;
    
    #ifndef System_h_declarations
        extern void LinuxShutdown(void);
        extern void LinuxReboot(void);
        extern LinuxPowerStatusType*LinuxGetPowerStatus(void);
        extern void SystemInit(unsigned char*name,void*ApplicationProgrammingInterface,int(*Init)(int),int(*Exit)(int));
        extern int SystemGetInitState(unsigned char*name);
        extern int SystemGetExitState(unsigned char*name);
        extern void*SystemGetApplicationProgrammingInterface(unsigned char*name);
        extern void SystemBuildRemover(unsigned char*name);
        extern SystemProcessType*GetSystemProcess(void);
        extern void NetworkInit(void(*function)(void));
        struct EventFunctionTimeout{
            void*data;
            void(*function)(void*);
            struct list_head lh;
        };
        #define Struct static struct      
        
        #define HBuildStart(description)\
            struct ApplicationProgrammingInterface##description{

        #define HBuildSignature(type,name,signature)\
            type(*name)signature;
                    
        #define HBuildEnd\
            int(*InitState)(void);int(*ExitState)(void);};

        #define CBuildSignature(type,name,signature)\
            static type name signature
                    
        #define CBuildInit\
            static int SystemBuildInitFunction(int state)

        #define CBuildExit\
            static int SystemBuildExitFunction(int state)

        #define CBuildStart(description) \
            static void __exit LinuxKernelObjectExit(void){\
                SystemBuildRemover(#description);\
            }\
            module_exit(LinuxKernelObjectExit);\
            static int SystemGetInitStateByApplicationProgrammingInterface(void){ \
                return SystemGetInitState(#description); \
            } \
            static int SystemGetExitStateByApplicationProgrammingInterface(void){ \
                return SystemGetExitState(#description); \
            } \
            RunInit(description){ \
                SystemInit(#description, &((struct ApplicationProgrammingInterface##description){

        #define CBuildBind(name) \
                .name=name,
  
        #define CBuildEnd \
                .InitState=SystemGetInitStateByApplicationProgrammingInterface,.ExitState=SystemGetExitStateByApplicationProgrammingInterface}),SystemBuildInitFunction,SystemBuildExitFunction);}

        #define CBuildConnectApplicationProgrammingInterface(description) \
            CBuildSignature(struct ApplicationProgrammingInterface##description*,Get##description,(void)){ \
                static struct ApplicationProgrammingInterface##description*api=NULL;\
                if(!api)\
                    api=(struct ApplicationProgrammingInterface##description*)SystemGetApplicationProgrammingInterface(#description);\
                return api;\
            }
       
        #define CBuildReturnZero { return 0; }


    #endif


#endif
