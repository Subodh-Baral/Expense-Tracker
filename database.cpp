#include<iostream>
#include"database.h"
#include"sqlite3.h"
using namespace std;
database::database(string file){
int createDb=sqlite3_open(file.c_str(),&db);
if(createDb==SQLITE_OK){
cout<<"opened db sucessfully\n";
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