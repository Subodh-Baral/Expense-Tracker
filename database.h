#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QSqlDatabase>

class Database {
public:
    static Database& instance();
    bool initialize();
    bool registerUser(const QString& name, const QString& email, const QString& password);
    bool validateLogin(const QString& email, const QString& password);
    bool emailExists(const QString& email);

private:
    Database() = default;
    QSqlDatabase db;
};

#endif