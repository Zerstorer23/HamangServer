#include "sqlite3/sqlite3.h"
#include <stdio.h>
#include <string>

using namespace std; 
typedef int SqlCallback (void*,int,char**,char**) ;

sqlite3* db;
int callback(
    void* NotUsed,
    int argc,
    char** argv,
    char** azColName)
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

void ExecSQL(string line) {
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
void ExecSQL(string line, SqlCallback function) {
    char* err_msg = 0;
    int rc = sqlite3_exec(db, line.c_str(), function, 0, &err_msg);
    //rc = sqlite3_exec(db, sql, callback, 0, &err_msg);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        exit(-1);
    }
}
void SetStat(string uid, string field, int value) {
    string update = 
        "UPDATE Statistics "
        "SET "+field+" = "+to_string(value)+" "
        "WHERE UID='"+uid+"';";
    ExecSQL(update);
    /*
    UPDATE Statistics
    SET Kills = 2
    WHERE UID = 'TEST'
    */
}
void GetStat(string uid, string field) {
    string update =
        "SELECT "+field+" "
        "FROM Statistics "
        "WHERE UID ='" + uid + "'";
    ExecSQL(update);
}
void AddStat(string uid, string field, int value) {
    string update =
        "UPDATE Statistics "
        "SET " + field + " = "+field+" + " + to_string(value) + " "
        "WHERE UID='" + uid + "';";
    ExecSQL(update);
    /*
    UPDATE Statistics
SET Kills = Kills + 1
WHERE UID = 'TEST';
    */
}
void ShowAll() {
    string update =
        "SELECT *"
        "FROM Statistics ;";
    ExecSQL(update);
}
void ShowID(string uid) {
    string update =
        "SELECT *"
        "FROM Statistics WHERE UID = '"+uid+"';";
    ExecSQL(update);
}

int main(void)
{

    int rc = sqlite3_open("hamangDB.db", &db);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }
    SetStat("TEST", "Kills", 60);
    ShowID("TEST");
    GetStat("TEST", "Kills");
    sqlite3_close(db);

    return 0;
}