#pragma once


#define NET_DELIM "#"
#define NET_SIG "LEX"

enum class MessageInfo {
    ServerRequest, RPC, SyncVar, Chat, Instantiate, Destroy, SetHash, ServerCallbacks
};
enum class LexCallback
{
    None, PlayerJoined, PlayerDisconnected, OnLocalPlayerJoined, MasterClientChanged,
 //   BufferedRPCsLoaded,
    RoomInformationReceived,
    PushServerTime
    , HashChanged
};
enum class LexRequest
{
    None, RemoveRPC, ChangeMasterClient, Receive_modifiedTime
};