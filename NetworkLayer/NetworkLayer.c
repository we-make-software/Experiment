#include "../NetworkLayer/.h" 
CBuildSignature(void,InitDataLinkLayer,(struct DataLinkLayer*dataLinkLayer)){

}
CBuildSignature(void,FreeDataLinkLayer,(struct DataLinkLayer*dataLinkLayer)){

}
CBuildSignature(DataLinkLayerPacketActionType,Receiver,(struct sk_buff*skb,void*dataLinkLayer)){
    printk(KERN_INFO "DataLinkLayer Receiver called: skb=%p, dataLinkLayer=%p\n", skb, dataLinkLayer);

    // Optional: inspect first few bytes of packet
    if (skb && skb->len >= 6) {
        printk(KERN_INFO "First 6 bytes: %*ph\n", 6, skb_mac_header(skb));
    }
    return DataLinkLayerPacketReceiveForOtherSoftware;
}

CBuildInit
CBuildReturnZero
CBuildExit 
CBuildReturnZero

CBuildStart(NetworkLayer)
    CBuildBind(Receiver)
CBuildEnd
