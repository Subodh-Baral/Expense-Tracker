#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCryptographicHash>
#include <QDate>
#include <QCoreApplication>

const double Database::DEFAULT_BUDGET = 10000.0;

Database& Database::instance() {
    static Database inst;
    return inst;
}

bool Database::initialize() {
    db = QSqlDatabase::addDatabase("QSQLITE");

    // IMPORTANT: use an absolute path tied to the executable's own folder,
    // not a relative path. A relative "expense_tracker.db" resolves against
    // whatever directory the app happens to be launched FROM, so running the
    // binary from two different shells/working-dirs silently creates two
    // separate database files with different data. Pinning it to the
    // executable's directory guarantees every launch reads the same file.
    QString dbPath = QCoreApplication::applicationDirPath() + "/expense_tracker.db";
    db.setDatabaseName(dbPath);
    qDebug() << "Using database file:" << dbPath;

    if (!db.open()) {
        qDebug() << "Database error:" << db.lastError().text();
        return false;
    }

    QSqlQuery q(db);

    // Users table
    if (!q.exec("CREATE TABLE IF NOT EXISTS users ("
                "id           INTEGER PRIMARY KEY AUTOINCREMENT,"
                "name         TEXT    NOT NULL,"
                "email        TEXT    UNIQUE NOT NULL,"
                "password     TEXT    NOT NULL,"
                "created_date TEXT)")) {
        qDebug() << "users table error:" << q.lastError().text();
        return false;
    }
    // Migration: older databases created before this column existed won't
    // have it. ALTER TABLE fails harmlessly if the column is already there,
    // so we just ignore that specific error and move on.
    q.exec("ALTER TABLE users ADD COLUMN created_date TEXT");
    // Backfill any existing accounts that still have no created_date (either
    // from the migration above, or from before this feature existed at all)
    // with today's date. This is a reasonable floor: it means rollover for
    // those older accounts will only start accumulating from today forward,
    // rather than fabricating surplus for months before we know about.
    q.exec(QString("UPDATE users SET created_date='%1' WHERE created_date IS NULL OR created_date=''")
           .arg(QDate::currentDate().toString("yyyy-MM-dd")));

    // Expenses table
    if (!q.exec("CREATE TABLE IF NOT EXISTS expenses ("
                "id          INTEGER PRIMARY KEY AUTOINCREMENT,"
                "user_id     INTEGER NOT NULL,"
                "description TEXT    NOT NULL,"
                "category    TEXT    NOT NULL,"
                "amount      REAL    NOT NULL,"
                "date        TEXT    NOT NULL,"
                "is_income   INTEGER NOT NULL DEFAULT 0,"
                "FOREIGN KEY(user_id) REFERENCES users(id))")) {
        qDebug() << "expenses table error:" << q.lastError().text();
        return false;
    }

    // Budgets table
    // base_budget: what the user set for this month
    // rollover:    surplus carried forward from the previous month
    if (!q.exec("CREATE TABLE IF NOT EXISTS budgets ("
                "id          INTEGER PRIMARY KEY AUTOINCREMENT,"
                "user_id     INTEGER NOT NULL,"
                "year        INTEGER NOT NULL,"
                "month       INTEGER NOT NULL,"
                "base_budget REAL    NOT NULL DEFAULT 10000.0,"
                "rollover    REAL    NOT NULL DEFAULT 0.0,"
                "UNIQUE(user_id, year, month),"
                "FOREIGN KEY(user_id) REFERENCES users(id))")) {
        qDebug() << "budgets table error:" << q.lastError().text();
        return false;
    }

    qDebug() << "Database initialized successfully";
    return true;
}

QString Database::hashPassword(const QString &password) {
    return QString(QCryptographicHash::hash(
        password.toUtf8(), QCryptographicHash::Sha256).toHex());
}

// ── Users ─────────────────────────────────────────────────────────────────────
bool Database::registerUser(const QString &name, const QString &email, const QString &password) {
    QSqlQuery q(db);
    q.prepare("INSERT INTO users (name, email, password, created_date) VALUES (:n, :e, :p, :c)");
    q.bindValue(":n", name);
    q.bindValue(":e", email);
    q.bindValue(":p", hashPassword(password));
    q.bindValue(":c", QDate::currentDate().toString("yyyy-MM-dd"));
    if (!q.exec()) { qDebug() << "registerUser failed:" << q.lastError().text(); return false; }
    return true;
}

bool Database::validateLogin(const QString &email, const QString &password) {
    QSqlQuery q(db);
    q.prepare("SELECT id FROM users WHERE email=:e AND password=:p");
    q.bindValue(":e", email);
    q.bindValue(":p", hashPassword(password));
    if (!q.exec()) { qDebug() << "validateLogin failed:" << q.lastError().text(); return false; }
    return q.next();
}

bool Database::emailExists(const QString &email) {
    QSqlQuery q(db);
    q.prepare("SELECT id FROM users WHERE email=:e");
    q.bindValue(":e", email);
    if (!q.exec()) { qDebug() << "emailExists failed:" << q.lastError().text(); return false; }
    return q.next();
}

QString Database::getUserName(const QString &email) {
    QSqlQuery q(db);
    q.prepare("SELECT name FROM users WHERE email=:e");
    q.bindValue(":e", email);
    if (!q.exec() || !q.next()) return QString();
    return q.value(0).toString();
}

int Database::getUserId(const QString &email) {
    QSqlQuery q(db);
    q.prepare("SELECT id FROM users WHERE email=:e");
    q.bindValue(":e", email);
    if (!q.exec() || !q.next()) return -1;
    return q.value(0).toInt();
}

// Returns the account's signup date, used to stop rollover calculations
// from reaching further back than the account actually existed. Falls back
// to today if somehow missing (shouldn't happen given the migration in
// initialize(), but keeps this safe regardless).
QDate Database::getUserCreatedDate(int userId) {
    QSqlQuery q(db);
    q.prepare("SELECT created_date FROM users WHERE id=:uid");
    q.bindValue(":uid", userId);
    if (q.exec() && q.next()) {
        QDate d = QDate::fromString(q.value(0).toString(), "yyyy-MM-dd");
        if (d.isValid()) return d;
    }
    return QDate::currentDate();
}

// ── Expenses ──────────────────────────────────────────────────────────────────
bool Database::addExpense(int userId, const QString &description,
                          const QString &category, double amount,
                          const QString &date, bool isIncome) {
    QSqlQuery q(db);
    q.prepare("INSERT INTO expenses (user_id, description, category, amount, date, is_income)"
              " VALUES (:uid, :desc, :cat, :amt, :dt, :inc)");
    q.bindValue(":uid",  userId);
    q.bindValue(":desc", description);
    q.bindValue(":cat",  category);
    q.bindValue(":amt",  amount);
    q.bindValue(":dt",   date);
    q.bindValue(":inc",  isIncome ? 1 : 0);
    if (!q.exec()) { qDebug() << "addExpense failed:" << q.lastError().text(); return false; }
    return true;
}

QList<Expense> Database::getExpenses(int userId) {
    QList<Expense> list;
    QSqlQuery q(db);
    q.prepare("SELECT id, user_id, description, category, amount, date, is_income"
              " FROM expenses WHERE user_id=:uid ORDER BY date DESC, id DESC");
    q.bindValue(":uid", userId);
    if (!q.exec()) { qDebug() << "getExpenses failed:" << q.lastError().text(); return list; }
    while (q.next()) {
        Expense e;
        e.id          = q.value(0).toInt();
        e.userId      = q.value(1).toInt();
        e.description = q.value(2).toString();
        e.category    = q.value(3).toString();
        e.amount      = q.value(4).toDouble();
        e.date        = q.value(5).toString();
        e.isIncome    = q.value(6).toInt() != 0;
        list.append(e);
    }
    return list;
}

// If this user has never had an income entry recorded, seed one default
// income of $10,000 so the dashboard doesn't show $0.00 for new/existing
// accounts that never had a way to log income. Runs once per user — after
// the first income row exists (whatever its value), this becomes a no-op.
void Database::ensureDefaultIncome(int userId) {
    QSqlQuery check(db);
    check.prepare("SELECT COUNT(*) FROM expenses WHERE user_id=:uid AND is_income=1");
    check.bindValue(":uid", userId);
    if (!check.exec() || !check.next()) return;
    if (check.value(0).toInt() > 0) return; // already has income history

    QSqlQuery ins(db);
    ins.prepare("INSERT INTO expenses (user_id, description, category, amount, date, is_income)"
                " VALUES (:uid, :desc, :cat, :amt, :dt, 1)");
    ins.bindValue(":uid",  userId);
    ins.bindValue(":desc", "Initial Income");
    ins.bindValue(":cat",  "Income");
    ins.bindValue(":amt",  10000.0);
    ins.bindValue(":dt",   QDate::currentDate().toString("yyyy-MM-dd"));
    if (!ins.exec()) qDebug() << "ensureDefaultIncome failed:" << ins.lastError().text();
}

double Database::getTotalIncome(int userId) {
    QSqlQuery q(db);
    q.prepare("SELECT COALESCE(SUM(amount),0) FROM expenses WHERE user_id=:uid AND is_income=1");
    q.bindValue(":uid", userId);
    if (!q.exec() || !q.next()) return 0.0;
    return q.value(0).toDouble();
}

double Database::getTotalExpenses(int userId) {
    QSqlQuery q(db);
    q.prepare("SELECT COALESCE(SUM(amount),0) FROM expenses WHERE user_id=:uid AND is_income=0");
    q.bindValue(":uid", userId);
    if (!q.exec() || !q.next()) return 0.0;
    return q.value(0).toDouble();
}

double Database::getMonthlyExpenses(int userId, int year, int month) {
    // dates stored as "yyyy-MM-dd" so LIKE 'yyyy-MM-%' works perfectly
    QString pattern = QString("%1-%2-%")
                          .arg(year)
                          .arg(month, 2, 10, QChar('0'));
    QSqlQuery q(db);
    q.prepare("SELECT COALESCE(SUM(amount),0) FROM expenses"
              " WHERE user_id=:uid AND is_income=0 AND date LIKE :pat");
    q.bindValue(":uid", userId);
    q.bindValue(":pat", pattern);
    if (!q.exec() || !q.next()) return 0.0;
    return q.value(0).toDouble();
}

double Database::getMonthlyIncome(int userId, int year, int month) {
    QString pattern = QString("%1-%2-%")
                          .arg(year)
                          .arg(month, 2, 10, QChar('0'));
    QSqlQuery q(db);
    q.prepare("SELECT COALESCE(SUM(amount),0) FROM expenses"
              " WHERE user_id=:uid AND is_income=1 AND date LIKE :pat");
    q.bindValue(":uid", userId);
    q.bindValue(":pat", pattern);
    if (!q.exec() || !q.next()) return 0.0;
    return q.value(0).toDouble();
}

// ── Budget ────────────────────────────────────────────────────────────────────

// Ensure a budget row exists for (userId, year, month).
// Returns the rollover stored there.
static double ensureBudgetRow(QSqlDatabase &db, int userId, int year, int month,
                               double defaultBudget = 10000.0)
{
    // Try insert with defaults; if already exists, do nothing
    QSqlQuery ins(db);
    ins.prepare("INSERT OR IGNORE INTO budgets (user_id, year, month, base_budget, rollover)"
                " VALUES (:uid, :y, :m, :b, 0.0)");
    ins.bindValue(":uid", userId);
    ins.bindValue(":y",   year);
    ins.bindValue(":m",   month);
    ins.bindValue(":b",   defaultBudget);
    ins.exec();

    QSqlQuery sel(db);
    sel.prepare("SELECT rollover FROM budgets WHERE user_id=:uid AND year=:y AND month=:m");
    sel.bindValue(":uid", userId);
    sel.bindValue(":y",   year);
    sel.bindValue(":m",   month);
    if (sel.exec() && sel.next()) return sel.value(0).toDouble();
    return 0.0;
}

BudgetInfo Database::getBudgetInfo(int userId, int year, int month)
{
    ensureBudgetRow(db, userId, year, month);

    QSqlQuery q(db);
    q.prepare("SELECT base_budget, rollover FROM budgets"
              " WHERE user_id=:uid AND year=:y AND month=:m");
    q.bindValue(":uid", userId);
    q.bindValue(":y",   year);
    q.bindValue(":m",   month);

    BudgetInfo info{DEFAULT_BUDGET, 0.0, 0.0, DEFAULT_BUDGET, 0.0, DEFAULT_BUDGET};
    if (q.exec() && q.next()) {
        info.baseBudget  = q.value(0).toDouble();
        info.rollover    = q.value(1).toDouble();
    }
    // Any income logged in this specific month increases what's available to
    // spend that month — e.g. a ₹10,000 base budget plus a ₹5,000 income
    // entry this month means ₹15,000 is available this month. Computed
    // fresh from the actual income rows each call (same "derive from real
    // data, don't store a mutable copy" approach as the rollover fix), so
    // it can never drift out of sync with what was actually logged.
    info.monthlyIncome = getMonthlyIncome(userId, year, month);
    info.totalBudget   = info.baseBudget + info.rollover + info.monthlyIncome;
    info.spent         = getMonthlyExpenses(userId, year, month);
    info.remaining     = info.totalBudget - info.spent;
    return info;
}

bool Database::setBaseBudget(int userId, int year, int month, double amount)
{
    ensureBudgetRow(db, userId, year, month);

    QSqlQuery q(db);
    q.prepare("UPDATE budgets SET base_budget=:b"
              " WHERE user_id=:uid AND year=:y AND month=:m");
    q.bindValue(":b",   amount);
    q.bindValue(":uid", userId);
    q.bindValue(":y",   year);
    q.bindValue(":m",   month);
    if (!q.exec()) { qDebug() << "setBaseBudget failed:" << q.lastError().text(); return false; }
    return true;
}

// Called on login. Recomputes the rollover chain from the account's signup
// month (or 24 months back, whichever is later) up through the given month,
// walking forward in chronological order.
//
// The previous version only ever looked at a single month-to-month step
// and stopped as soon as it applied one rollover. That meant if the account
// had accumulated surplus several months back but the very next link in the
// chain hadn't been recalculated yet, the surplus got stranded partway and
// never reached the current month — so the dashboard would show this
// month's budget as if no rollover had happened at all.
//
// This version always recomputes the full chain in one pass, in order, so
// each month's rollover is derived from the ACTUAL leftover of the month
// before it (which itself already reflects its own correctly-cascaded
// rollover computed earlier in this same loop). It's idempotent — safe to
// call on every login — and self-heals even if a backdated expense is
// added after the fact.
void Database::processRollover(int userId, int year, int month)
{
    constexpr int LOOKBACK_MONTHS = 24;
    QDate target(year, month, 1);
    QDate floor = target.addMonths(-LOOKBACK_MONTHS);

    QDate signup = getUserCreatedDate(userId);
    QDate signupMonth(signup.year(), signup.month(), 1);
    QDate start = (signupMonth > floor) ? signupMonth : floor;

    QDate cursor = start;
    while (cursor < target) {
        QDate nextMonth = cursor.addMonths(1);

        ensureBudgetRow(db, userId, cursor.year(), cursor.month());
        BudgetInfo info = getBudgetInfo(userId, cursor.year(), cursor.month());
        double surplus  = qMax(info.remaining, 0.0);

        ensureBudgetRow(db, userId, nextMonth.year(), nextMonth.month());
        QSqlQuery upd(db);
        upd.prepare("UPDATE budgets SET rollover=:r"
                    " WHERE user_id=:uid AND year=:y AND month=:m");
        upd.bindValue(":r",   surplus);
        upd.bindValue(":uid", userId);
        upd.bindValue(":y",   nextMonth.year());
        upd.bindValue(":m",   nextMonth.month());
        upd.exec();

        cursor = nextMonth;
    }
}

QList<Expense> Database::getExpensesOnly(int userId) {
    QList<Expense> list;
    QSqlQuery q(db);
    q.prepare("SELECT id, user_id, description, category, amount, date, is_income"
              " FROM expenses WHERE user_id=:uid AND is_income=0 ORDER BY date DESC, id DESC");
    q.bindValue(":uid", userId);
    if (!q.exec()) { qDebug() << "getExpensesOnly failed:" << q.lastError().text(); return list; }
    while (q.next()) {
        Expense e;
        e.id          = q.value(0).toInt();
        e.userId      = q.value(1).toInt();
        e.description = q.value(2).toString();
        e.category    = q.value(3).toString();
        e.amount      = q.value(4).toDouble();
        e.date        = q.value(5).toString();
        e.isIncome    = false;
        list.append(e);
    }
    return list;
}

QList<Expense> Database::getMonthlyExpensesOnly(int userId, int year, int month) {
    QList<Expense> list;
    QString pattern = QString("%1-%2-%").arg(year).arg(month, 2, 10, QChar('0'));
    QSqlQuery q(db);
    q.prepare("SELECT id, user_id, description, category, amount, date, is_income"
              " FROM expenses WHERE user_id=:uid AND is_income=0 AND date LIKE :pat"
              " ORDER BY date DESC, id DESC");
    q.bindValue(":uid", userId);
    q.bindValue(":pat", pattern);
    if (!q.exec()) { qDebug() << "getMonthlyExpensesOnly failed:" << q.lastError().text(); return list; }
    while (q.next()) {
        Expense e;
        e.id          = q.value(0).toInt();
        e.userId      = q.value(1).toInt();
        e.description = q.value(2).toString();
        e.category    = q.value(3).toString();
        e.amount      = q.value(4).toDouble();
        e.date        = q.value(5).toString();
        e.isIncome    = false;
        list.append(e);
    }
    return list;
}

QMap<QString,double> Database::getMonthlyCategoryTotals(int userId, int year, int month) {
    QMap<QString,double> map;
    QString pattern = QString("%1-%2-%").arg(year).arg(month, 2, 10, QChar('0'));
    QSqlQuery q(db);
    q.prepare("SELECT category, SUM(amount) FROM expenses"
              " WHERE user_id=:uid AND is_income=0 AND date LIKE :pat"
              " GROUP BY category ORDER BY SUM(amount) DESC");
    q.bindValue(":uid", userId);
    q.bindValue(":pat", pattern);
    if (!q.exec()) { qDebug() << "getMonthlyCategoryTotals failed:" << q.lastError().text(); return map; }
    while (q.next())
        map[q.value(0).toString()] = q.value(1).toDouble();
    return map;
}

QMap<QString,double> Database::getAllCategoryTotals(int userId) {
    QMap<QString,double> map;
    QSqlQuery q(db);
    q.prepare("SELECT category, SUM(amount) FROM expenses"
              " WHERE user_id=:uid AND is_income=0"
              " GROUP BY category ORDER BY SUM(amount) DESC");
    q.bindValue(":uid", userId);
    if (!q.exec()) return map;
    while (q.next())
        map[q.value(0).toString()] = q.value(1).toDouble();
    return map;
}

// Returns monthly totals for the last N months: list of {year,month,income,expense}
QList<MonthlyTotal> Database::getMonthlyTotals(int userId, int months) {
    QList<MonthlyTotal> result;
    QDate today = QDate::currentDate();
    for (int i = months-1; i >= 0; i--) {
        QDate d = today.addMonths(-i);
        QString pat = QString("%1-%2-%").arg(d.year()).arg(d.month(),2,10,QChar('0'));
        QSqlQuery qi(db), qe(db);
        qi.prepare("SELECT COALESCE(SUM(amount),0) FROM expenses"
                   " WHERE user_id=:uid AND is_income=1 AND date LIKE :pat");
        qi.bindValue(":uid", userId); qi.bindValue(":pat", pat);
        qe.prepare("SELECT COALESCE(SUM(amount),0) FROM expenses"
                   " WHERE user_id=:uid AND is_income=0 AND date LIKE :pat");
        qe.bindValue(":uid", userId); qe.bindValue(":pat", pat);
        MonthlyTotal mt;
        mt.year   = d.year(); mt.month = d.month();
        mt.income = (qi.exec() && qi.next()) ? qi.value(0).toDouble() : 0.0;
        mt.expense= (qe.exec() && qe.next()) ? qe.value(0).toDouble() : 0.0;
        result.append(mt);
    }
    return result;
}

int Database::getExpenseCount(int userId, const QString &category) {
    QSqlQuery q(db);
    if (category.isEmpty() || category == "All") {
        q.prepare("SELECT COUNT(*) FROM expenses WHERE user_id=:uid AND is_income=0");
        q.bindValue(":uid", userId);
    } else {
        q.prepare("SELECT COUNT(*) FROM expenses WHERE user_id=:uid AND is_income=0 AND category=:cat");
        q.bindValue(":uid", userId); q.bindValue(":cat", category);
    }
    if (!q.exec() || !q.next()) return 0;
    return q.value(0).toInt();
}
