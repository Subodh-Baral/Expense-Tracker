#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include "widgets.h"
#include "loginwidget.h"
#include "signupwidget.h"

class MainWindow : public BackgroundWidget {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void showLogin();
    void showSignUp();
    void onLoginSuccessful();

private:
    QStackedWidget* stackedWidget;
    LoginWidget* loginWidget;
    SignUpWidget* signUpWidget;
};

#endif