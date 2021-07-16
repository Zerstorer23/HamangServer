#include "DBMS_NI.h"
#include "NetworkMessage.h"
#include "PlayerManager.h"
#include "Player.h"
DEFINITION_SINGLE(DBMS_NI)
HSTMT DBMS_NI::hstmt; // Statement handle  
// static unsigned char szData[MAX_DATA]; // Returned data storage  
SQLLEN DBMS_NI::cbData; // Output length of data  

DBMS_NI::~DBMS_NI() {
    instance->CloseDatabase();
}
// Constructor initializes the string chr_ds_name with the data source name.  
// "Northwind" is an ODBC data source (odbcad32.exe) name whose default is the Northwind database  
DBMS_NI::DBMS_NI() {
    rc = {};
    db = {};
    henv = {};
    hdbc = {};
    hstmt = {};
    cbData = {};
}

// Allocate environment handle and connection handle, connect to data source, and allocate statement handle.  
void DBMS_NI::sqlconn() {
    SQLAllocEnv(&henv);
    SQLAllocConnect(henv, &hdbc);
    //rc = SQLConnect(hdbc, chr_ds_name, SQL_NTS, NULL, 0, NULL, 0);
    rc = SQLConnect(hdbc,
        (SQLCHAR*)"hamang", SQL_NTS,
        (SQLCHAR*)"admin", SQL_NTS,
        (SQLCHAR*)"1234", SQL_NTS);
    // Deallocate handles, display error message, and exit.  
    if (!MYSQLSUCCESS(rc)) {

        int iDiag;
        SQLRETURN ret;
        SQLINTEGER NativeError;
        SQLCHAR SqlState[6], Msg[255];
        SQLSMALLINT MsgLen;
        TCHAR Mes[1024];

        for (iDiag = 1;; iDiag++) {
            ret = SQLGetDiagRec(SQL_HANDLE_DBC, hdbc, iDiag, SqlState,
                &NativeError, Msg, sizeof(Msg), &MsgLen);
            if (ret == SQL_NO_DATA) {
                break;
            }
            wsprintf(Mes, "SQLSTATE:%s , NativeError : %d , 진단정보: %s",
                (LPCTSTR)SqlState, NativeError, (LPCTSTR)Msg
            );
            printf("%s\n", Mes);

        }
        std::cout << "Fail connect" << endl;
        SQLFreeConnect(henv);
        SQLFreeEnv(henv);
        SQLFreeConnect(hdbc);
        exit(-1);
    }

    rc = SQLAllocStmt(hdbc, &hstmt);
}

// Execute SQL command with SQLExecDirect() ODBC API.  
void DBMS_NI::sqlexec(int actorID, string line, SqlCallback function)
{
    rc = SQLExecDirect(hstmt, (SQLCHAR*)line.c_str(), SQL_NTS);
    if (!MYSQLSUCCESS(rc)) {  //Error  
        error_out();
        // Deallocate handles and disconnect.  
        SQLFreeStmt(hstmt, SQL_DROP);
        SQLDisconnect(hdbc);
        SQLFreeConnect(hdbc);
        SQLFreeEnv(henv);
        exit(-1);
    }
    else {
        function(actorID);
    }
}
void DBMS_NI::sqlexec(string line) {
    rc = SQLExecDirect(hstmt,(SQLCHAR*) line.c_str(), SQL_NTS);
    if (!MYSQLSUCCESS(rc)) {  //Error  
        error_out();
        // Deallocate handles and disconnect.  
        SQLFreeStmt(hstmt, SQL_DROP);
        SQLDisconnect(hdbc);
        SQLFreeConnect(hdbc);
        SQLFreeEnv(henv);
        exit(-1);
    }
    
}

// Free the statement handle, disconnect, free the connection handle, and free the environment handle.  
void DBMS_NI::sqldisconn() {
    SQLFreeStmt(hstmt, SQL_DROP);
    SQLDisconnect(hdbc);
    SQLFreeConnect(hdbc);
    SQLFreeEnv(henv);
}

// Display error message in a message box that has an OK button.  
void DBMS_NI::error_out() {


    unsigned char szSQLSTATE[10];
    SDWORD nErr;
    unsigned char msg[SQL_MAX_MESSAGE_LENGTH + 1];
    SWORD cbmsg;
    while (SQLError(0, 0, hstmt, szSQLSTATE, &nErr, msg, sizeof(msg), &cbmsg) == SQL_SUCCESS) {
        cout << "Error: SQLSTATE=" << szSQLSTATE << ", Native error=" << nErr << ", msg=" << msg << endl;
        //    MessageBox(NULL, (const char*)szData, "ODBC Error", MB_OK);
    }
}



void DBMS_NI::Callback_LogIn(int actorID)
{
    RETCODE rc;
    SQLINTEGER   numLogins;
    SQLGetData(hstmt, 1, SQL_C_ULONG, &numLogins, 0, &numLogins);

    //TODO
    numLogins = 1;
    NetworkMessage netMessage;
    netMessage.Append(0);
    netMessage.Append(((int)MessageInfo::ServerCallbacks));
    netMessage.Append((int)LexCallback::DB_Received);
    netMessage.Append((int)LexDBTable::Statistics);
    netMessage.Append((int)LexDBcode::LogIn);
    netMessage.Append(numLogins);
    PlayerManager::GetInst()->playerHash[actorID]->Send(netMessage.BuildNewSignedMessage(), true);

}
void DBMS_NI::Callback_Stat(int actorID)
{
    //TODO get column name, params
    //https://docs.microsoft.com/ko-kr/sql/odbc/reference/syntax/sqlgetdata-function?view=sql-server-ver15
    //string key = azColName[0];
    //SQLCHAR buffer[128];
    //SQLINTEGER    cbValue;
    //SQLGetData(hstmt, 2, SQL_C_CHAR, buffer, 128, &cbValue);
    //string value = (char *)(buffer);
    //NetworkMessage netMessage;
    //netMessage.Append(0);
    //netMessage.Append(((int)MessageInfo::ServerCallbacks));
    //netMessage.Append((int)LexCallback::DB_Received);
    //netMessage.Append((int)LexDBTable::Statistics);
    //netMessage.Append((int)LexDBcode::Get);
    //netMessage.Append(key);
    //netMessage.Append(value);
    //PlayerManager::GetInst()->playerHash[actorID]->Send(netMessage.BuildNewSignedMessage(), true);
}

void DBMS_NI::Callback_Show(int actorID)
{
    RETCODE rc;
    unsigned char szData[MAX_DATA]; // Returned data storage  
    SQLLEN cbData; // Output length of data  
    for (rc = SQLFetch(hstmt); rc == SQL_SUCCESS; rc = SQLFetch(hstmt)) {
        SQLGetData(hstmt, 1, SQL_C_CHAR, szData, sizeof(szData), &cbData);
        // In this example, the data is sent to the console; SQLBindCol() could be called to bind   
        // individual rows of data and assign for a rowset.  
        printf("%s\n", (const char*)szData);

        SQLGetData(hstmt, 2, SQL_C_CHAR, szData, sizeof(szData), &cbData);
        // In this example, the data is sent to the console; SQLBindCol() could be called to bind   
        // individual rows of data and assign for a rowset.  
        printf("%s\n", (const char*)szData);
    }
}



void DBMS_NI::SetStat(string table, string uid, string field, string value)
{
    string update =
        "UPDATE [dbo].[" + table + "] "
        "SET [" + field + "] = " + (value)+" "
        "WHERE [UID]='" + uid + "';";
    sqlexec(update);

    /*
    UPDATE Statistics
    SET Kills = 2
    WHERE UID = 'TEST'
    */
}

void DBMS_NI::GetStat(int actorID, string table, string uid, string field)
{
    string command =
        "SELECT [" + field + "] "
        "FROM " + table + " "
        "WHERE [UID] ='" + uid + "'";
    sqlexec(actorID, command, Callback_Stat);
}

void DBMS_NI::AddStat(string table, string uid, string field, string value)
{
    string command =
        "UPDATE dbo].[" + table + "] "
        "SET [" + field + "] = [" + field + "] + " + (value)+" "
        "WHERE [UID]='" + uid + "';";
    sqlexec(command);
    /*
    UPDATE Statistics
SET Kills = Kills + 1
WHERE UID = 'TEST';
    */
}

void DBMS_NI::DoLogIn(int actorID, string uid, string password)
{
    cout << "Login id " << uid << " password " << password << " actor " << actorID << endl;
    string command =
        "SELECT COUNT(*) "
        "UPDATE [dbo].[Statistics] "
        "WHERE [UID] ='" + uid + "' AND [Password] ='" + password + "'";
    sqlexec(actorID, command, Callback_LogIn);
}

void DBMS_NI::HandleRequest(NetworkMessage& netMessage)
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
    }
    else {
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
            sqlexec(netMessage.GetNext());
            break;
        }
    }
}