// Connecting_with_SQLConnect.cpp  
// compile with: user32.lib odbc32.lib  
#pragma once
#include "Values.h"
#define MAX_DATA 128  
#define MYSQLSUCCESS(rc) ((rc == SQL_SUCCESS) || (rc == SQL_SUCCESS_WITH_INFO) )  
/*
SSMS��ġ (ODBC����)
MS SQL Dev ��ġ (DB����)

ODBC �������� ����

*/
class NetworkMessage;
class Player;
class PlayerManager;
class DBMS_NI
{
    //TODO Mutex?
public:
    DECLARE_SINGLE(DBMS_NI)
public:
    typedef void SqlCallback(int);
    RETCODE rc; // ODBC return code  
    HENV henv; // Environment     
    HDBC hdbc; // Connection handle  
    //TODO ��Ƽ�������?
    static HSTMT hstmt; // Statement handle  
   // static unsigned char szData[MAX_DATA]; // Returned data storage  
    static SQLLEN cbData; // Output length of data  

    void sqlconn(); // Allocate env, stat, and conn  
    void sqlexec(int actorID, string line, SqlCallback function);
    void sqlexec(string); // Execute SQL statement  
    void sqldisconn(); // Free pointers to env, stat, conn, and disconnect  
    void error_out(); // Displays errors  
    sqlite3* db;
    string databaseName = "hamangDB.db";

    static void Callback_LogIn(int actorID);
    static void Callback_Stat(int actorID);
    static void Callback_Show(int actorID);

    void SetStat(string table, string uid, string field, string value);
    void GetStat(int actorID, string table, string uid, string field);
    void AddStat(string table, string uid, string field, string value);
    void DoLogIn(int actorID, string uid, string password);


    void HandleRequest(NetworkMessage& cnetMessage);

    void ShowAll() {
        string update =
            "USE [hamang]"
            "SELECT *"
            "FROM[dbo].[Statistics] ";// WHERE UID = '" + uid + "'; ";
        sqlexec(0,update, Callback_Show);
    }
    void ShowID(string uid) {
        string update =
            "USE [hamang]"
            "SELECT *"
            "FROM[dbo].[Statistics] WHERE UID = '" + uid + "';";
        sqlexec(update);
    }

    int OpenDatabase()
    {
        sqlconn();   // Allocate handles, and connect.  
        return 0;
    }

    void CloseDatabase() {
        if (db)
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