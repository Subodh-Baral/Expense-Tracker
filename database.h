#ifndef DATABASE_H
#define DATABASE_H
#include<iostream>
#include"sqlite3.h"
using namespace std;
class database{
    private:
    sqlite3 *db;
    bool createUserTable();
    public:
    database(string file);
    ~database();
};
#endif