#include "../NetworkLayer/.h"

CBuildConnectMinuteMemoryTimeout

CBuildInit{
    struct net_device *dev;
    rcu_read_lock();
    for_each_netdev(&init_net,dev){
        if(dev->type==ARPHRD_ETHER){

        }
    }
    rcu_read_unlock();
    synchronize_net();
    return 0;
}

CBuildExit{
  
    /*
    list_for_each_entry_safe(dlls,tmp,&DLSourcePoolCreators,list){

    }
    synchronize_net();
   */
    return 0;
}

CBuildStart(DataLinkLayer)

CBuildEnd

/*
    CBuildBind(New)
    CBuildBind(Send)
*/