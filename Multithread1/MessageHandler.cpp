#include "MessageHandler.h"
#include "IOCP_Server.h"
#include "NetworkMessage.h"
#include "PlayerManager.h"
#include "Player.h"
#include "BufferedMessages.h"
#include "PingManager.h"
#include "MessageHandler.h"
#include "HashTable.h"
#include "DBMS_NI.h"

bool MessageHandler::CheckMessageComplete(NetworkMessage& netMessage)
{
    if (EASY_LOG) netMessage.PrintOut();
    //2. 메세지 길이 읽기
    if (!netMessage.HasNext()) {
        return false;
    }
    int lengthOfMessages = stoi(netMessage.GetNext());
    bool isComplete = netMessage.SetEndPoint(lengthOfMessages);
    return isComplete;
}

void MessageHandler::HandleMessage(NetworkMessage& netMessage)
{
    while (netMessage.HasNext()) {
        netMessage.SetBeginPoint();
        //1. Check signature
        string signature = netMessage.GetNext();
        bool isMyPacket = (signature.compare(NET_SIG) == 0);
        if (!isMyPacket) {
            cout << u8"SIG :[" << signature << "] vs " << NET_SIG << endl;
            cout << u8"NOT MY PACKET " << endl;
            continue;
        }
        bool isComplete = CheckMessageComplete(netMessage);
        if (!isComplete) return;
        //3. 메세지 발언자 읽기
        netMessage.sentActorNr = stoi(netMessage.GetNext());
        //4. 메세지 타입 읽기
        MessageInfo messageInfo = (MessageInfo)stoi(netMessage.GetNext());

       // cout << signature << "Is my packet from " << netMessage.beginPoint << " to " << netMessage.endPoint << endl;
        if (messageInfo == MessageInfo::ServerRequest) {
           // cout << "Received server request" << endl;
            //4. request면 2번째 코드 읽고 switch
            Handle_ServerRequest(netMessage);
            //방송처리는 각자해야함
        }
        else if (messageInfo == MessageInfo::SetHash) {
         //   cout << "Handle hash : " << (int)messageInfo << endl;
            Handle_PropertyRequest(netMessage);
        }
        else {
        //    cout << "Received echo message" << endl;
            //0 /1 2 3 4/ 5 6
            //한단위씩 저장됨
            string message = netMessage.SaveStringsForBroadcast();//end exclusive
            if (messageInfo == MessageInfo::RPC || messageInfo == MessageInfo::Instantiate) {
                //모든 방송메세지는 0 sig / 1 len / 2 sender / 3 type / 4 viewID 형식
                //저장이 필요한 타입들;
                netMessage.targetViewID = stoi(netMessage.GetNext());
                BufferedMessages::GetInst()->EnqueueMessage(netMessage.sentActorNr, netMessage.targetViewID, message);
            }
  /*          if (messageInfo == MessageInfo::Destroy) {
                cout << L"Received " << message << endl;
            }*/
            netMessage.SetIteratorToEnd();
        }

    }



}


void MessageHandler::Handle_PropertyRequest(NetworkMessage& netMessage)
{
    //SetHash면 서버 Hash도 업데이트 필요
    //actorNum, SetHash [int]roomOrPlayer [string]Key [object]value
    assert(netMessage.HasNext());
    int target = stoi(netMessage.GetNext());
    assert(netMessage.HasNext());
    int numHash = stoi(netMessage.GetNext());
    for (int i = 0; i < numHash; i++) {
        assert(netMessage.HasNext());
        string key = netMessage.GetNext();
        assert(netMessage.HasNext());
        string typeName = netMessage.GetNext();
        assert(netMessage.HasNext());
        string value = netMessage.GetNext();
        if (target == 0) {
            IOCP_Server::GetInst()->customProperty->SetProperty(key, typeName, value);
            if (EASY_LOG) IOCP_Server::GetInst()->customProperty->PrintProperties();
        }
        else {
            PlayerManager::GetInst()->playerHash[target]->customProperty->SetProperty(key, typeName, value);
            if (EASY_LOG)PlayerManager::GetInst()->playerHash[target]->customProperty->PrintProperties();
        }
    }
    netMessage.SaveStringsForBroadcast();
}
void MessageHandler::Handle_ServerRequest(NetworkMessage& netMessage)
{
    assert(netMessage.HasNext());
    LexRequest messageCode = (LexRequest)stoi(netMessage.GetNext());
    switch (messageCode)
    {
    case LexRequest::None:
        break;
    case LexRequest::RemoveRPC:
        Handle_ServerRequest_RemoveRPCs(netMessage);
        break;
    case LexRequest::Receive_modifiedTime:
        Handle_ServerRequest_ReceiveModifiedTime(netMessage);
        break;
    case LexRequest::ChangeMasterClient:
        Handle_ServerRequest_ChangeMasterClient(netMessage);
        break;
    case LexRequest::Ping:
        Handle_ServerRequest_Ping(netMessage);
        break;
    case LexRequest::DBReference:
        DBMS_NI::GetInst()->HandleRequest(netMessage);
        break;
    }

}

void MessageHandler::Handle_ServerRequest_ChangeMasterClient(NetworkMessage& netMessage) {
    //actorID , MessageInfo , callbackType, params
    assert(netMessage.HasNext());
    int newMasterActor = stoi(netMessage.GetNext());
    PlayerManager::GetInst()->SetMasterClient(newMasterActor);
    NetworkMessage eolMessage;
    eolMessage.Append(to_string(netMessage.sentActorNr));
    eolMessage.Append(to_string((int)MessageInfo::ServerCallbacks));
    eolMessage.Append(to_string((int)LexCallback::MasterClientChanged));
    eolMessage.Append(to_string(newMasterActor));
    string message = eolMessage.BuildNewSignedMessage();
   // DWORD size = message.length();
   // LPPER_IO_DATA sendIO = IOCP_Server::GetInst()->CreateMessage(message);
    PlayerManager::GetInst()->BroadcastMessageAll(message);
}
/*void MessageHandler::Handle_ServerRequest_SendBufferedRPCs(Player* target) {

}*/

void MessageHandler::Handle_ServerRequest_RemoveRPCs(NetworkMessage& netMessage) {
    assert(netMessage.HasNext());
    int actorID = stoi(netMessage.GetNext());
    assert(netMessage.HasNext());
    int viewID = stoi(netMessage.GetNext());
    if (actorID != -1) {
        if (viewID != -1) {
            BufferedMessages::GetInst()->RemoveRPC(actorID, viewID);
        }
        else {
            BufferedMessages::GetInst()->RemovePlayerNr(actorID);
        }
    }
    else {
        BufferedMessages::GetInst()->RemoveViewID(viewID);
    }

}
void MessageHandler::Handle_ServerRequest_ReceiveModifiedTime(NetworkMessage& netMessage)
{
    //LEX / int: sentPlayer / MsgIngo: ServerRequest / ReqInfo : modify time / bool: requestRPC
    int targetPlayerNumber = netMessage.sentActorNr;
    assert(netMessage.HasNext());
    int requestRPC = stoi(netMessage.GetNext());
    Player* target = PlayerManager::GetInst()->playerHash[targetPlayerNumber];
    PingManager::GetInst()->TimeSynch_Receive(target);
 /*   if (requestRPC) {
        Handle_ServerRequest_SendBufferedRPCs(target);
    }*/
}
void MessageHandler::Handle_ServerRequest_Ping(NetworkMessage& netMessage) {
    //actorID , MessageInfo , callbackType, params

    long long timeNow = PingManager::GetInst()->GetTimeNow();
    NetworkMessage eolMessage;
    eolMessage.Append((netMessage.sentActorNr));
    eolMessage.Append(((int)MessageInfo::ServerCallbacks));
    eolMessage.Append(((int)LexCallback::Ping_Received));
    eolMessage.Append(to_string(timeNow));
    string message = eolMessage.BuildNewSignedMessage();
    //LPPER_IO_DATA sendIO = IOCP_Server::GetInst()->CreateMessage(message);
    PlayerManager::GetInst()->playerHash[netMessage.sentActorNr]->Send(message,false);
}