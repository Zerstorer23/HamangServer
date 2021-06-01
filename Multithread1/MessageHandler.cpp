#include "MessageHandler.h"
#include "IOCP_Server.h"
#include "NetworkMessage.h"
#include "PlayerManager.h"
#include "Player.h"
#include "BufferedMessages.h"
#include "PingManager.h"
#include "MessageHandler.h"


void MessageHandler::HandleMessage(NetworkMessage& netMessage)
{
    while (netMessage.HasNext()) {
        int beginPointOfAMessage = netMessage.iterator;
        //1. Check signature
        string signature = netMessage.GetNext();
        bool isMyPacket = (signature.compare(NET_SIG) == 0);
        if (!isMyPacket) continue;

        //2. 메세지 길이 읽기
        int lengthOfMessages = stoi(netMessage.GetNext());
        //3. 메세지 발언자 읽기
        netMessage.sentActorNr = stoi(netMessage.GetNext());
        //4. 메세지 타입 읽기
        MessageInfo messageInfo = (MessageInfo)stoi(netMessage.GetNext());
        int endPointOfAMessage = beginPointOfAMessage + lengthOfMessages;
        cout << signature << "Is my packet from " << beginPointOfAMessage << " to " << endPointOfAMessage << endl;
        if (messageInfo == MessageInfo::ServerRequest) {
            cout << "Received server request" << endl;
            //4. request면 2번째 코드 읽고 switch
            Handle_ServerRequest(netMessage);
        }
        else if (messageInfo == MessageInfo::SetHash) {
            cout << "Handle hash : " << (int)messageInfo << endl;
            Handle_PropertyRequest(netMessage);
            netMessage.SaveStrings(beginPointOfAMessage, endPointOfAMessage);
        }
        else {
            cout << "Received echo message" << endl;
            //0 /1 2 3 4/ 5 6
            //한단위씩 저장됨
            string message = netMessage.SaveStrings(beginPointOfAMessage, endPointOfAMessage);//end exclusive
            if (messageInfo == MessageInfo::RPC || messageInfo == MessageInfo::Instantiate
                || messageInfo == MessageInfo::Destroy || messageInfo == MessageInfo::SyncVar) {
                //모든 방송메세지는 0 sig / 1 len / 2 sender / 3 type / 4 viewID 형식
                //저장이 필요한 타입들
                netMessage.targetViewID = stoi(netMessage.GetNext());
                BufferedMessages::GetInst()->EnqueueMessage(netMessage.sentActorNr, netMessage.targetViewID, message);
            }
            netMessage.iterator = endPointOfAMessage;
        }
    }



}

void MessageHandler::Handle_PropertyRequest(NetworkMessage& netMessage)
{
    //SetHash면 서버 Hash도 업데이트 필요
    //actorNum, SetHash [int]roomOrPlayer [string]Key [object]value
    int target = stoi(netMessage.GetNext());
    cout << "Set hash on " << endl;
    string key = netMessage.GetNext();
    string value = netMessage.GetNext();
    if (target == 0) {
        IOCP_Server::GetInst()->SetProperty(key, value);
        IOCP_Server::GetInst()->PrintProperties();
    }
    else {
        PlayerManager::GetInst()->playerHash[target]->SetProperty(key, value);
        PlayerManager::GetInst()->playerHash[target]->PrintProperties();
    }
}
void MessageHandler::Handle_ServerRequest(NetworkMessage& netMessage)
{
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
    }

}

void MessageHandler::Handle_ServerRequest_ChangeMasterClient(NetworkMessage& netMessage) {
    //actorID , MessageInfo , callbackType, params
    int newMasterActor = stoi(netMessage.GetNext());
    PlayerManager::GetInst()->SetMasterClient(newMasterActor);
    NetworkMessage eolMessage;
    eolMessage.Append(to_string(netMessage.sentActorNr));
    eolMessage.Append(to_string((int)MessageInfo::ServerCallbacks));
    eolMessage.Append(to_string((int)LexCallback::MasterClientChanged));
    eolMessage.Append(to_string(newMasterActor));
    string message = eolMessage.BuildNewSignedMessage();
    DWORD size = message.length();
    LPPER_IO_DATA sendIO = IOCP_Server::GetInst()->CreateMessage(message);
    PlayerManager::GetInst()->BroadcastMessageAll((char*)message.c_str(), size);
}
void MessageHandler::Handle_ServerRequest_SendBufferedRPCs(Player* target) {
    BufferedMessages::GetInst()->SendBufferedMessages(target);

    NetworkMessage eolMessage;
    eolMessage.Append(to_string(target->actorNumber));
    eolMessage.Append(to_string((int)MessageInfo::ServerCallbacks));
    eolMessage.Append(to_string((int)LexCallback::OnLocalPlayerJoined));
    string message = eolMessage.BuildNewSignedMessage();
    LPPER_IO_DATA sendIO = IOCP_Server::GetInst()->CreateMessage(message);
    target->Send(sendIO);
    cout << "Sent buffered RPCs" << endl;

    //actorID , MessageInfo , callbackType, params
// PlayerJoined는 로컬제외 전체방송
    NetworkMessage broadcastMessage;
    broadcastMessage.Append("-1");
    broadcastMessage.Append(to_string((int)MessageInfo::ServerCallbacks));
    broadcastMessage.Append(to_string((int)LexCallback::PlayerJoined));
    target->EncodeToNetwork(broadcastMessage);
    string brmsg = broadcastMessage.BuildNewSignedMessage();
    DWORD size = (DWORD)brmsg.length();
    PlayerManager::GetInst()->BroadcastMessage(target->actorNumber, (char*)brmsg.c_str(), size);
    cout << "IO Created" << endl;
}

void MessageHandler::Handle_ServerRequest_RemoveRPCs(NetworkMessage& netMessage) {
    int actorID = stoi(netMessage.GetNext());
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
    //LEX / 0 =SERVER / PING=MESSAGEINFO / targetPlater /1 OR 0 = INDEX TO REFER / SERVERTIME or EXPECTEDDELAY //Part of local Join
    int targetPlayerNumber = stoi(netMessage.GetNext());
    int isMod = stoi(netMessage.GetNext());//always 1
    (netMessage.GetNext());
    int isLocal = stoi(netMessage.GetNext());
    Player* target = PlayerManager::GetInst()->playerHash[targetPlayerNumber];
    PingManager::GetInst()->Handle_Request_TimeSynch(target, isMod, isLocal);
    if (isLocal) {
        Handle_ServerRequest_SendBufferedRPCs(target);
    }
}