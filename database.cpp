#include<iostream>
#include"database.h"
#include"sqlite3.h"
#include<argon2.h>
#include<random>
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
string hashPass(const string &pass){
constexpr uint32_t t_cost=3;
constexpr uint32_t m_cost=65536;
constexpr uint32_t p=1;
constexpr size_t salt_length=16;
constexpr size_t hash_length=32;
unsigned char salt[salt_length];
random_device randNum;
for(size_t i=0;i<salt_length;i++){
salt[i]=static_cast<unsigned char>(randNum());
}
char encoded[256];
int res=argon2id_hash_encoded(t_cost,m_cost,p,pass.c_str(),pass.size(),salt,salt_length,hash_length,encoded,sizeof(encoded));
if(res!=ARGON2_OK){
throw runtime_error(argon2_error_message(res));
}else{
return string(encoded);
}
}
bool database::registerUser(const string& username,const string& email,const string &password){ 
    sqlite3_stmt* stmt; string sql="INSERT INTO users (username,email,pass) VALUES (?,?,?)"; 
    int res=sqlite3_prepare_v2(db,sql.c_str(),-1,&stmt,nullptr); 
    if(res!=SQLITE_OK){ 
        cerr<<"An error occoured:"<<sqlite3_errmsg(db)<<endl;
        sqlite3_finalize(stmt); 
        return false; 
    }
    else{ 
    string hashedPass=hashPass(password);
    sqlite3_bind_text(stmt,1,username.c_str(),-1,SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt,2,email.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,3,hashedPass.c_str(),-1,SQLITE_TRANSIENT); 
    bool success=(sqlite3_step(stmt)==SQLITE_DONE); 
    sqlite3_finalize(stmt); 
     return success; 
    } 
    }