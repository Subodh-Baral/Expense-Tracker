#include<iostream>
#include "sqlite3.h"
using namespace std;
int main(){
sqlite3* db;
int rc=sqlite3_open("expense.db",&db);
if(rc==SQLITE_OK){
cout<<"database opened sucessfully";
}else{
    cout<<"an error occoured";
}
sqlite3_close(db);
    return 0;
}