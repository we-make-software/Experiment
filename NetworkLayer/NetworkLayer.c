#include "../NetworkLayer/.h" 
CBuildSignature(void,InitDataLinkLayer,(struct DataLinkLayer*dataLinkLayer)){

}
CBuildSignature(void,FreeDataLinkLayer,(struct DataLinkLayer*dataLinkLayer)){

}
CBuildSignature(DataLinkLayerPacketActionType,Receiver,(struct sk_buff*skb,void*dataLinkLayer)){
    return DataLinkLayerPacketReceiveForOtherSoftware;
}

CBuildInit
CBuildReturnZero
CBuildExit 
CBuildReturnZero

CBuildStart(NetworkLayer)
    CBuildBind(Receiver)
CBuildEnd
