#include "mainwindow.h"
#include "loginwidget.h"
#include "signupwidget.h"
#include <QScreen>
#include <QGuiApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    setWindowTitle("Expense Tracker");
    setMinimumSize(1200, 800);

    QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    move(screenGeometry.center() - rect().center());

    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    loginWidget  = new LoginWidget(this);
    signupWidget = new SignupWidget(this);
    dashboardWidget = new Dashboard(this);

    stackedWidget->addWidget(loginWidget);
    stackedWidget->addWidget(signupWidget);
    stackedWidget->addWidget(dashboardWidget);

    connect(loginWidget,  &LoginWidget::loginSuccessful,
            this, &MainWindow::onLoginSuccessful);
    connect(loginWidget,  &LoginWidget::switchToSignUp,
            this, &MainWindow::onSwitchToSignUp);
    connect(signupWidget, &SignupWidget::switchToLogin,
            this, &MainWindow::onSwitchToLogin);
    connect(signupWidget, &SignupWidget::signupSuccessful,
            this, &MainWindow::onLoginSuccessful);
    connect(dashboardWidget, &Dashboard::logoutRequested,
            this, &MainWindow::onLogoutRequested);

    showLogin();
}

void MainWindow::showLogin()
{
    stackedWidget->setCurrentIndex(0);
    resize(400, 600);
    QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    move(screenGeometry.center() - rect().center());
}

void MainWindow::showDashboard(const QString &name, const QString &email)
{
    dashboardWidget->setUserInfo(name, email);
    stackedWidget->setCurrentIndex(2);
    resize(1200, 800);
    QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    move(screenGeometry.center() - rect().center());
}

void MainWindow::onLoginSuccessful(const QString &name, const QString &email)
{
    showDashboard(name, email);
}

void MainWindow::onLogoutRequested()  { showLogin(); }
void MainWindow::onSwitchToSignUp()   { stackedWidget->setCurrentIndex(1); }
void MainWindow::onSwitchToLogin()    { stackedWidget->setCurrentIndex(0); }
