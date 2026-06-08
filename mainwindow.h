#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "widgets.h"
#include <QStackedWidget>

class LoginWidget;
class SignUpWidget;

class MainWindow : public BackgroundWidget {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void showLogin();
    void showSignUp();

private:
    QStackedWidget* stackedWidget;
    LoginWidget* loginWidget;
    SignUpWidget* signUpWidget;
};

#endif