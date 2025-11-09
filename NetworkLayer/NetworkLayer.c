#include "../NetworkLayer/.h" 
CBuildSignature(void,InitDataLinkLayer,(struct DataLinkLayer*dataLinkLayer)){

}
CBuildSignature(void,FreeDataLinkLayer,(struct DataLinkLayer*dataLinkLayer)){

}
CBuildSignature(void,ReceiverDataLinkLayer,(struct sk_buff*skb,struct DataLinkLayer*dataLinkLayer)){

}
CBuildInit
CBuildReturnZero
CBuildExit 
CBuildReturnZero

CBuildStart(NetworkLayer)
    CBuildBind(InitDataLinkLayer)
    CBuildBind(FreeDataLinkLayer)
    CBuildBind(ReceiverDataLinkLayer)
CBuildEnd
