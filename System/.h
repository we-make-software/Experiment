/*
 * Status codes used in FunctionPayload->Function:
 *   0  - Cannot be used for full delete; reserved for active/running state
 *  -1  - Not found
 *  -2  - Forced removal; called when removing a module exit
 * Setting state to other value make the function in the tail of the list
 * and can be used for delayed removal.
 * Positive values can be used for custom states.
 * This idea its build in to CBuildInit and CBuildExit macros.
 * This to function requires to return a state value if 0 is returned the function will be remove from the list.
 * Buidling concept for header:
 * #ifndef Project_h // Prevent multiple inclusions
 * #define Project_h 
 *  HBuildStart(Project) // Start building struct for API
 *     HBuildSignature(return_type,FunctionName,(parameters)); // Function signature
 *  HBuildEnd // End building struct for API
 *  #define CBuildConnectProject CBuildConnectApplicationProgrammingInterface(Project) // Build function generator function to get API struct pointer
 * #endif
 * Building concept for code:
 *  CBuildSignature(return_type,FunctionName,(parameters)){return_type;} // Function signature
 *  CBuildInit{ // Start building init function
 *     // Your code here
 *     return state; // Return state code
 *  } // End building init function
 *  CBuildExit{ // Start building exit function
 *    // Your code here
 *   return state; // Return state code
 *  } // End building exit function
 *  CBuildStart(Project) // Start building module init function
 *   CBuildBind(FunctionName) // Bind function to API struct
 *  CBuildEnd // End building module init function
 * 
 * Init a module and build API in Code:
 * CBuildConnectProject // Build function to get API struct pointer
 * 
 * Example usage:
 * struct ApplicationProgrammingInterfaceProject*api=GetProject();
 * if(api!=NULL)
 *  api->FunctionName(parameters);
 * 
 * Useful Commands:
 * LinuxShutdown(); // Shutdown the system
 * LinuxReboot(); // Reboot the system
 * LinuxPowerStatusType*status=LinuxGetPowerStatus(); // Get power status
 * SystemGetInitState("Project"); // Get init state of Project module
 * SystemGetExitState("Project"); // Get exit state of Project module
 * GetSystemProcess(); // Get current system process state
 * All other functions are internal and should not be used directly.
*/
#include "../Run/.h"
#ifndef System_h
#define System_h
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
            static struct ApplicationProgrammingInterface##description*Get##description(void) { \
                static struct ApplicationProgrammingInterface##description*api=NULL;\
                if(!api)\
                    api=(struct ApplicationProgrammingInterface##description*)SystemGetApplicationProgrammingInterface(#description);\
                return api;\
            }
       

    #endif


#endif
