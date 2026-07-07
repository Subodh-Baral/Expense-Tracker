#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QSqlDatabase>
#include <QList>
#include <QMap>

struct Expense {
    int     id;
    int     userId;
    QString description;
    QString category;
    double  amount;
    QString date;
    bool    isIncome;
};

struct BudgetInfo {
    double baseBudget;
    double rollover;
    double totalBudget;
    double spent;
    double remaining;
};

struct MonthlyTotal {
    int    year;
    int    month;
    double income;
    double expense;
};

class Database {
public:
    static Database& instance();
    bool initialize();

    // Users
    bool    registerUser(const QString &name, const QString &email, const QString &password);
    bool    validateLogin(const QString &email, const QString &password);
    bool    emailExists(const QString &email);
    QString getUserName(const QString &email);
    int     getUserId(const QString &email);

    // Expenses
    bool           addExpense(int userId, const QString &description,
                              const QString &category, double amount,
                              const QString &date, bool isIncome);
    QList<Expense> getExpenses(int userId);
    QList<Expense> getExpensesOnly(int userId);
    QList<Expense> getMonthlyExpensesOnly(int userId, int year, int month);
    double         getTotalIncome(int userId);
    double         getTotalExpenses(int userId);
    double         getMonthlyExpenses(int userId, int year, int month);
    QMap<QString,double> getMonthlyCategoryTotals(int userId, int year, int month);
    QMap<QString,double> getAllCategoryTotals(int userId);
    QList<MonthlyTotal>  getMonthlyTotals(int userId, int months);
    int                  getExpenseCount(int userId, const QString &category = QString());

    // Budget
    BudgetInfo getBudgetInfo(int userId, int year, int month);
    bool       setBaseBudget(int userId, int year, int month, double amount);
    void       processRollover(int userId, int year, int month);

private:
    Database() = default;
    QSqlDatabase db;
    QString hashPassword(const QString &password);
    static const double DEFAULT_BUDGET;
};

#endif
