#include "../NetworkLayer/.h" 
CBuildSignature(void,InitDataLinkLayer,(struct DataLinkLayer*dataLinkLayer)){

}
CBuildSignature(void,FreeDataLinkLayer,(struct DataLinkLayer*dataLinkLayer)){

}
CBuildSignature(void,ReceiverDataLinkLayer,(struct sk_buff*skb,struct DataLinkLayer*dataLinkLayer)){
 printk(KERN_INFO "Hello world\n");
}
CBuildInit
CBuildReturnZero
CBuildExit 
CBuildReturnZero

CStart(NetworkLayer,
    CBuildBind(InitDataLinkLayer)
    CBuildBind(FreeDataLinkLayer)
    CBuildBind(ReceiverDataLinkLayer)
)
