#pragma once


#define NET_DELIM u8"#"s
#define NET_SIG u8"LEX"s

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