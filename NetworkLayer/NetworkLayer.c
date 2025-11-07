#include "../NetworkLayer/.h" 
CBuildSignature(void,InitDataLinkLayer,(struct DataLinkLayer*dataLinkLayer)){

}
CBuildSignature(void,FreeDataLinkLayer,(struct DataLinkLayer*dataLinkLayer)){

}



CBuildInit{

    return 0;
}

CBuildExit{

    return 0;
}

CBuildStart(DataLinkLayer)

CBuildEnd
