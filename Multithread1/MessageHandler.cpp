#include "MessageHandler.h"
#include "IOCP_Server.h"
#include "NetworkMessage.h"
#include "PlayerManager.h"
#include "Player.h"
#include "BufferedMessages.h"
#include "PingManager.h"
#include "MessageHandler.h"
#include "HashTable.h"


void MessageHandler::HandleMessage(NetworkMessage& netMessage)
{
    while (netMessage.HasNext()) {
        netMessage.SetBeginPoint();
        //1. Check signature
        wstring signature = netMessage.GetNext();
        /*
        TODO
        string�� ���ļ� ������ /0�� ������ 0LEX vs LEX �� ������
        �ٵ� 0�� �ȳְ� ������ string ��ȯ�ÿ� ������ ����

        ->0 ���� �ް� ���� �������� 0����.
        �Ʒ��� ���� ���� �Ƹ� �������ʳ� ���� Ȯ�� �ʿ�
        */
        if (signature.length() > 0 &&signature.c_str()[0] == 0 && signature.length() == 4) {
         signature = signature.substr(1, 3);
        }
    /*    wcout << L"SIG" << signature <<" vs "<< NetworkMessage::ServerSignature << endl;
        for (int i = 0; i < signature.length(); i++) {
            wcout << signature.c_str()[i] << " vs " << NetworkMessage::ServerSignature.c_str()[i] << endl;
        }*/

        bool isMyPacket = (signature.compare(NET_SIG) == 0);
        if (!isMyPacket) {
            wcout << L"SIG :[" << signature << "] vs " << NET_SIG << endl;
            wcout << L"NOT MY PACKET " << endl;
            continue;
        }
        if(EASY_LOG) netMessage.PrintOut();

        //2. �޼��� ���� �б�
        int lengthOfMessages = stoi(netMessage.GetNext());
        //3. �޼��� �߾��� �б�
        netMessage.sentActorNr = stoi(netMessage.GetNext());
        //4. �޼��� Ÿ�� �б�
        MessageInfo messageInfo = (MessageInfo)stoi(netMessage.GetNext());
        netMessage.SetEndPoint(lengthOfMessages);
       // cout << signature << "Is my packet from " << netMessage.beginPoint << " to " << netMessage.endPoint << endl;
        if (messageInfo == MessageInfo::ServerRequest) {
           // cout << "Received server request" << endl;
            //4. request�� 2��° �ڵ� �а� switch
            Handle_ServerRequest(netMessage);
            //���ó���� �����ؾ���
        }
        else if (messageInfo == MessageInfo::SetHash) {
         //   cout << "Handle hash : " << (int)messageInfo << endl;
            Handle_PropertyRequest(netMessage);
        }
        else {
        //    cout << "Received echo message" << endl;
            //0 /1 2 3 4/ 5 6
            //�Ѵ����� �����
            wstring message = netMessage.SaveStringsForBroadcast();//end exclusive
            if (messageInfo == MessageInfo::RPC 
                || messageInfo == MessageInfo::Instantiate
                || messageInfo == MessageInfo::Destroy  
                //|| messageInfo == MessageInfo::SyncVar
                ) {
                //��� ��۸޼����� 0 sig / 1 len / 2 sender / 3 type / 4 viewID ����
                //������ �ʿ��� Ÿ�Ե�
                netMessage.targetViewID = stoi(netMessage.GetNext());
                DEBUG_MODE wcout << L"RPC Saved " << message << endl;
                BufferedMessages::GetInst()->EnqueueMessage(netMessage.sentActorNr, netMessage.targetViewID, message);
            }
            netMessage.SetIteratorToEnd();
        }
       DEBUG_MODE cout << endl;
       DEBUG_MODE cout << endl;
    }



}

void MessageHandler::Handle_PropertyRequest(NetworkMessage& netMessage)
{
    //SetHash�� ���� Hash�� ������Ʈ �ʿ�
    //actorNum, SetHash [int]roomOrPlayer [string]Key [object]value
    int target = stoi(netMessage.GetNext());
    int numHash = stoi(netMessage.GetNext());
    for (int i = 0; i < numHash; i++) {
        wstring key = netMessage.GetNext();
        wstring typeName = netMessage.GetNext();
        wstring value = netMessage.GetNext();
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
    }

}

void MessageHandler::Handle_ServerRequest_ChangeMasterClient(NetworkMessage& netMessage) {
    //actorID , MessageInfo , callbackType, params
    int newMasterActor = stoi(netMessage.GetNext());
    PlayerManager::GetInst()->SetMasterClient(newMasterActor);
    NetworkMessage eolMessage;
    eolMessage.Append(to_wstring(netMessage.sentActorNr));
    eolMessage.Append(to_wstring((int)MessageInfo::ServerCallbacks));
    eolMessage.Append(to_wstring((int)LexCallback::MasterClientChanged));
    eolMessage.Append(to_wstring(newMasterActor));
    wstring message = eolMessage.BuildNewSignedMessage();
   // DWORD size = message.length();
   // LPPER_IO_DATA sendIO = IOCP_Server::GetInst()->CreateMessage(message);
    PlayerManager::GetInst()->BroadcastMessageAll(message);
}
void MessageHandler::Handle_ServerRequest_SendBufferedRPCs(Player* target) {
    BufferedMessages::GetInst()->SendBufferedMessages(target);

    NetworkMessage eolMessage;
    eolMessage.Append(to_wstring(target->actorNumber));
    eolMessage.Append(to_wstring((int)MessageInfo::ServerCallbacks));
    eolMessage.Append(to_wstring((int)LexCallback::OnLocalPlayerJoined));
    wstring message = eolMessage.BuildNewSignedMessage();
   // LPPER_IO_DATA sendIO = IOCP_Server::GetInst()->CreateMessage(message);
    target->Send(message);
    cout << "Sent buffered RPCs" << endl;

    //actorID , MessageInfo , callbackType, params
// PlayerJoined�� �������� ��ü���
    NetworkMessage broadcastMessage;
    broadcastMessage.Append(L"-1");
    broadcastMessage.Append(to_wstring((int)MessageInfo::ServerCallbacks));
    broadcastMessage.Append(to_wstring((int)LexCallback::PlayerJoined));
    target->EncodeToNetwork(broadcastMessage);
    wstring brmsg = broadcastMessage.BuildNewSignedMessage();
    PlayerManager::GetInst()->BroadcastMessage(target->actorNumber, brmsg);
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
    //LEX / int: sentPlayer / MsgIngo: ServerRequest / ReqInfo : modify time / bool: requestRPC
    int targetPlayerNumber = netMessage.sentActorNr;
    int requestRPC = stoi(netMessage.GetNext());
    Player* target = PlayerManager::GetInst()->playerHash[targetPlayerNumber];
    PingManager::GetInst()->TimeSynch_Receive(target);
    if (requestRPC) {
        Handle_ServerRequest_SendBufferedRPCs(target);
    }
}
void MessageHandler::Handle_ServerRequest_Ping(NetworkMessage& netMessage) {
    //actorID , MessageInfo , callbackType, params
    NetworkMessage eolMessage;
    eolMessage.Append(to_wstring(netMessage.sentActorNr));
    eolMessage.Append(to_wstring((int)MessageInfo::ServerCallbacks));
    eolMessage.Append(to_wstring((int)LexCallback::Ping_Received));
    wstring message = eolMessage.BuildNewSignedMessage();
    //LPPER_IO_DATA sendIO = IOCP_Server::GetInst()->CreateMessage(message);
    PlayerManager::GetInst()->playerHash[netMessage.sentActorNr]->Send(message);
}