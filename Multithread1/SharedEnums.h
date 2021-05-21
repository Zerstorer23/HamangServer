#pragma once


#define NET_DELIM "#"
#define NET_SIG "LEX"

enum class MessageInfo {
    ServerRequest, RPC, SyncVar, Chat, Instantiate, Destroy, SetHash, ServerCallbacks
};
enum class LexCallback
{
    None, PlayerJoined, PlayerDisconnected, OnLocalPlayerJoined, MasterClientChanged,
    BufferedRPCsLoaded,
    RoomInformationReceived,
    Ping
};
enum class LexRequest
{
    None, RemoveRPC_ViewID, RemoveRPC_Player, Receive_Initialise, Receive_RPCbuffer,Ping
};