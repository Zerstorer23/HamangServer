#pragma once


#define NET_DELIM "#"

enum class MessageInfo {
    ServerRequest, RPC, SyncVar, Chat, Instantiate, Destroy, SetHash, ServerCallbacks
};
enum class LexCallback
{
    None, PlayerJoined, PlayerDisconnected, LocalPlayerJoined, Receive_RoomHash, Receive_PlayerHash, MasterClientChanged
};
enum class LexRequest
{
    None, RemoveRPC_ViewID, RemoveRPC_Player, Receive_Initialise, Receive_RPCbuffer
};