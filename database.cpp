#include<iostream>
#include"database.h"
#include"sqlite3.h"
using namespace std;
database::database(string file){
int createDb=sqlite3_open(file.c_str(),&db);
if(createDb==SQLITE_OK){
cout<<"opened db sucessfully\n";

if(createUserTable()){
cout<<"user Tb created sucessfully";
}
}else{
    cerr<<"an error occoured while opening database"<<sqlite3_errmsg(db)<<endl;
    db=nullptr;
}
}
database::~database(){
    if(db!=nullptr){
sqlite3_close(db);
}
}
bool database::createUserTable(){
    if(db==nullptr){
return false;
    }

    string userTbSql="CREATE TABLE IF NOT EXISTS users("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "username TEXT NOT NULL UNIQUE,"
                     "email TEXT NOT NULL UNIQUE,"
                     "pass TEXT NOT NULL,"
                     "created_at DATETIME DEFAULT CURRENT_TIMESTAMP);"
    ;
    char* errMsg=nullptr;
    int res=sqlite3_exec(db,userTbSql.c_str(),nullptr,nullptr,&errMsg);
    if(res!=SQLITE_OK){
    cerr<<"an error occoured while creating a table"<<errMsg<<endl;
    sqlite3_free(errMsg);
    return false;
    }else{
    return true;
    }
}