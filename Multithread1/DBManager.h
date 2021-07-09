#pragma once
#include "Values.h"
class NetworkMessage;
class PlayerManager;
class DBManager
{
    //TODO Mutex?
public:
	DECLARE_SINGLE(DBManager)
public:
    typedef int SqlCallback(void*, int, char**, char**);
    sqlite3* db;
    string databaseName = "hamangDB.db";

    static int callback(
        void* NotUsed,
        int argc,
        char** argv,
        char** azColName);
    static int Callback_LogIn(
        void* NotUsed,
        int argc,
        char** argv,
        char** azColName);

    static int Callback_Stat(
        void* NotUsed,
        int argc,
        char** argv,
        char** azColName);

    void ExecSQL(string line);
    void ExecSQL(int actorID, string line, SqlCallback function);
    void SetStat(string table, string uid, string field, string value);
    void GetStat(int actorID, string table, string uid, string field);
    void AddStat(string table, string uid, string field, string value);
    void DoLogIn(int actorID, string uid, string password);
  

    void HandleRequest(NetworkMessage & cnetMessage);

    void ShowAll() {
        string update =
            "SELECT *"
            "FROM Statistics ;";
        ExecSQL(update);
    }
    void ShowID(string uid) {
        string update =
            "SELECT *"
            "FROM Statistics WHERE UID = '" + uid + "';";
        ExecSQL(update);
    }

    int OpenDatabase()
    {

        int rc = sqlite3_open(databaseName.c_str(), &db);

        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return 1;
        }

        return 0;
    }

    void CloseDatabase() {
        if(db)
        sqlite3_close(db);
    }
    string DecodeTableCode(LexDBTable table) {
        switch (table) {
        case LexDBTable::Achievements:
            return "Achievements";
        case LexDBTable::Statistics:
            return "Statistics";
        case LexDBTable::Leaderboards:
            return "Leaderboards";
        case LexDBTable::Events:
            return "Events";        
        }
    }

};