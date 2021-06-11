#pragma once


#define NET_DELIM "#"
#define NET_SIG "LEX"

enum class MessageInfo {
    ServerRequest, RPC, SyncVar, Chat, Instantiate, Destroy, SetHash, ServerCallbacks
};
enum class LexCallback
{
    None, PlayerJoined, PlayerDisconnected, OnLocalPlayerJoined, MasterClientChanged,
    HashChanged,
    Disconnected,
    ModifyServerTime,
    RoomInformationReceived
    , Ping_Received
};
enum class LexRequest
{
    None, RemoveRPC, ChangeMasterClient, Receive_modifiedTime,Ping
};