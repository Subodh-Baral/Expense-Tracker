#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QRegularExpression>

Database& Database::instance() {
    static Database inst;
    return inst;
}

bool Database::initialize() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("expense_tracker.db");

    if (!db.open()) {
        qDebug() << "Database error:" << db.lastError().text();
        return false;
    }

    QSqlQuery query(db);
    bool success = query.exec(
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL, "
        "email TEXT UNIQUE NOT NULL, "
        "password TEXT NOT NULL)"
    );

    if (!success) {
        qDebug() << "Table creation error:" << query.lastError().text();
        return false;
    }

    qDebug() << "Database initialized successfully";
    return true;
}

bool Database::registerUser(const QString& name, const QString& email, const QString& password) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO users (name, email, password) VALUES (:name, :email, :password)");
    query.bindValue(":name", name);
    query.bindValue(":email", email);
    query.bindValue(":password", password);

    if (!query.exec()) {
        qDebug() << "Registration failed:" << query.lastError().text();
        return false;
    }
    return true;
}

bool Database::validateLogin(const QString& email, const QString& password) {
    QSqlQuery query(db);
    query.prepare("SELECT id FROM users WHERE email = :email AND password = :password");
    query.bindValue(":email", email);
    query.bindValue(":password", password);

    if (!query.exec()) {
        qDebug() << "Login query failed:" << query.lastError().text();
        return false;
    }

    return query.next();
}

bool Database::emailExists(const QString& email) {
    QSqlQuery query(db);
    query.prepare("SELECT id FROM users WHERE email = :email");
    query.bindValue(":email", email);

    if (!query.exec()) {
        qDebug() << "Email check failed:" << query.lastError().text();
        return false;
    }

    return query.next();
}