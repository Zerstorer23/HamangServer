// Connecting_with_SQLConnect.cpp  
// compile with: user32.lib odbc32.lib  
#include <windows.h>  
#include <sqlext.h>  
#include <mbstring.h>  
#include <stdio.h>  
#include <iostream>

/*
SSMS설치 (ODBC구성)
MS SQL Dev 설치 (DB구성)

ODBC 구성에서 연결

*/

#define MAX_DATA 100  
#define MYSQLSUCCESS(rc) ((rc == SQL_SUCCESS) || (rc == SQL_SUCCESS_WITH_INFO) )  
using namespace std;
class direxec {
    RETCODE rc; // ODBC return code  
    HENV henv; // Environment     
    HDBC hdbc; // Connection handle  
    HSTMT hstmt; // Statement handle  

    unsigned char szData[MAX_DATA]; // Returned data storage  
    SQLLEN cbData; // Output length of data  

public:
    direxec(); // Constructor  
    void sqlconn(); // Allocate env, stat, and conn  
    void sqlexec(unsigned char*); // Execute SQL statement  
    void sqldisconn(); // Free pointers to env, stat, conn, and disconnect  
    void error_out(); // Displays errors  
};

// Constructor initializes the string chr_ds_name with the data source name.  
// "Northwind" is an ODBC data source (odbcad32.exe) name whose default is the Northwind database  
direxec::direxec() {
    rc = {};
    henv = {};
    hdbc = {};
    hstmt = {};
    cbData = {};
}

// Allocate environment handle and connection handle, connect to data source, and allocate statement handle.  
void direxec::sqlconn() {
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
void direxec::sqlexec(unsigned char* cmdstr) {
    rc = SQLExecDirect(hstmt, cmdstr, SQL_NTS);
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
}

// Free the statement handle, disconnect, free the connection handle, and free the environment handle.  
void direxec::sqldisconn() {
    SQLFreeStmt(hstmt, SQL_DROP);
    SQLDisconnect(hdbc);
    SQLFreeConnect(hdbc);
    SQLFreeEnv(henv);
}

// Display error message in a message box that has an OK button.  
void direxec::error_out() {
   

    unsigned char szSQLSTATE[10];
    SDWORD nErr;
    unsigned char msg[SQL_MAX_MESSAGE_LENGTH + 1];
    SWORD cbmsg;
    while (SQLError(0, 0, hstmt, szSQLSTATE, &nErr, msg, sizeof(msg), &cbmsg) == SQL_SUCCESS) {
        cout<<"Error: SQLSTATE="<< szSQLSTATE <<", Native error="<< nErr <<", msg="<< msg<<endl;
    //    MessageBox(NULL, (const char*)szData, "ODBC Error", MB_OK);
    }
}

int main() {
    direxec x;   // Declare an instance of the direxec object.  
    x.sqlconn();   // Allocate handles, and connect.  
    x.sqlexec((UCHAR FAR*)
        "USE [hamang]"
        "SELECT [ID], [Password], [Kills]"
        "FROM[dbo].[Statistics]"
        );   // Execute SQL command  
    x.sqldisconn();   // Free handles and disconnect  
}