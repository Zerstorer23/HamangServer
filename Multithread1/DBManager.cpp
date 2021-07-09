#include "DBManager.h"
#include "NetworkMessage.h"
#include "PlayerManager.h"
#include "Player.h"
DEFINITION_SINGLE(DBManager)

DBManager::DBManager() {
}
DBManager::~DBManager() {
	instance->CloseDatabase();
}

int DBManager::callback(void* NotUsed, int argc, char** argv, char** azColName)
{
    NotUsed = 0;

    for (int i = 0; i < argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }

    printf("\n");
    /*
    콜백의 첫 번째 인자는 sqlite3_exec() 함수의 네 번째 인자로 넘긴 값입니다. 특별한 경우가 아니면 잘 사용되지 않습니다.
    두 번째 인자는 출력되는 행의 컬럼 개수입니다.
    세 번째 인자는 각 컬럼의 값입니다.
    네 번째 인자는 각 컬럼의 이름입니다.

    콜백 안의 코드를 보면, 모든 컬럼을 순회하며 이름과 값을 출력하는 것을 볼 수 있습니다.
    */
    return 0;
}

int DBManager::Callback_LogIn(void* param, int argc, char** argv, char** azColName)
{
    int actorID = *((int * )param);
    int numLogins = atoi(argv[0]);
    cout <<actorID<< " / Login result " << numLogins << endl;

    //TODO
    numLogins = 1;
    NetworkMessage netMessage;
    netMessage.Append(0);
    netMessage.Append(((int)MessageInfo::ServerCallbacks));
    netMessage.Append((int)LexCallback::DB_Received);
    netMessage.Append((int)LexDBTable::Statistics);
    netMessage.Append((int)LexDBcode::LogIn);
    netMessage.Append(numLogins);
    PlayerManager::GetInst()->playerHash[actorID]->Send(netMessage.BuildNewSignedMessage(),true);
    return 0;
}
int DBManager::Callback_Stat(void* param, int argc, char** argv, char** azColName)
{
    int actorID = *((int*)param);
    string key = azColName[0];
    string value = argv[0];
    NetworkMessage netMessage;
    netMessage.Append(0);
    netMessage.Append(((int)MessageInfo::ServerCallbacks));
    netMessage.Append((int)LexCallback::DB_Received);
    netMessage.Append((int)LexDBTable::Statistics);
    netMessage.Append((int)LexDBcode::Get);
    netMessage.Append(key);
    netMessage.Append(value);
    PlayerManager::GetInst()->playerHash[actorID]->Send(netMessage.BuildNewSignedMessage(), true);
    return 0;
}
void DBManager::ExecSQL(string line)
{
    char* err_msg = 0;
    int rc = sqlite3_exec(db, line.c_str(), callback, 0, &err_msg);
    //rc = sqlite3_exec(db, sql, callback, 0, &err_msg);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        exit(-1);
    }
}

void DBManager::ExecSQL(int actorID, string line, SqlCallback function )
{
    char* err_msg = 0;
    cout << "Execute " << line << endl;
    int rc = sqlite3_exec(db, line.c_str(), function, &actorID, &err_msg);
    //rc = sqlite3_exec(db, sql, callback, 0, &err_msg);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        exit(-1);
    }
}

void DBManager::SetStat(string table, string uid, string field, string value)
{
    string update =
        "UPDATE "+table+" "
        "SET " + field + " = " + (value) + " "
        "WHERE UID='" + uid + "';";
    ExecSQL(update);

    /*
    UPDATE Statistics
    SET Kills = 2
    WHERE UID = 'TEST'
    */
}

void DBManager::GetStat(int actorID, string table,string uid, string field)
{
    string command =
        "SELECT " + field + " "
        "FROM "+table+" "
        "WHERE UID ='" + uid + "'";
    ExecSQL(actorID, command,Callback_Stat);
}

void DBManager::AddStat(string table, string uid, string field, string value)
{
    string command =
        "UPDATE " + table + " "
        "SET " + field + " = " + field + " + " + (value) + " "
        "WHERE UID='" + uid + "';";
    ExecSQL(command);
    /*
    UPDATE Statistics
SET Kills = Kills + 1
WHERE UID = 'TEST';
    */
}

void DBManager::DoLogIn(int actorID, string uid, string password)
{
    cout << "Login id " << uid << " password " << password << " actor " << actorID << endl;
    string command =
        "SELECT COUNT(*) "
        "FROM Statistics "
        "WHERE UID ='" + uid + "' AND Password ='"+password+"'";
    ExecSQL(actorID, command, Callback_LogIn);
}

void DBManager::HandleRequest(NetworkMessage & netMessage)
{
    /*
        LexNetworkMessage message = new LexNetworkMessage(LocalPlayer.actorID,
                (int)MessageInfo.ServerRequest,
                (int)LexRequest.DBReference,
                (int)code,
                DB_UID
                );
  
             SentID, ServerRequest, DBReferece/  DBCOde:Set,Get,Update,SQL / UID /  [key, type, val] or [SQL]
    */
    LexDBTable table = (LexDBTable)netMessage.GetNextInt();
    LexDBcode code = (LexDBcode)netMessage.GetNextInt();
    string userID = netMessage.GetNext();

    if (code == LexDBcode::LogIn) {
        DoLogIn(netMessage.sentActorNr, userID, netMessage.GetNext());
    }
    else if (table == LexDBTable::Events) {
        AddStat(DecodeTableCode(table), userID, netMessage.GetNext(), netMessage.GetNext()); //ALWAYS NEED TO BE STRING
    }else{
        switch (code) {
        case LexDBcode::Get:
            if (table == LexDBTable::Statistics) {
                GetStat(netMessage.sentActorNr, DecodeTableCode(table), userID, netMessage.GetNext());
            }
            else if (table == LexDBTable::Achievements) {

            }
            else if (table == LexDBTable::Leaderboards) {

            }
            break;
        case LexDBcode::Set:
            SetStat(DecodeTableCode(table), userID, netMessage.GetNext(), netMessage.GetNext()); //ALWAYS NEED TO BE STRING
            break;
        case LexDBcode::Append:
            AddStat(DecodeTableCode(table), userID, netMessage.GetNext(), netMessage.GetNext()); //ALWAYS NEED TO BE STRING
            break;
        case LexDBcode::SQL:
            ExecSQL(netMessage.GetNext());
            break;
        }
    }
}
