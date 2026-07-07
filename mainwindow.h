#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include "dashboard.h"

class LoginWidget;
class SignupWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setupUI();
    void showLogin();
    void showDashboard(const QString &name, const QString &email);

    QStackedWidget *stackedWidget;
    LoginWidget *loginWidget;
    SignupWidget *signupWidget;
    Dashboard *dashboardWidget;

private slots:
    void onLoginSuccessful(const QString &name, const QString &email);
    void onLogoutRequested();
    void onSwitchToSignUp();
    void onSwitchToLogin();
};

#endif
