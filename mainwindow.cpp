#include "mainwindow.h"
#include "loginwidget.h"
#include "signupwidget.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent) : BackgroundWidget(parent) {
    setWindowTitle("ExpenseTracker");
    setMinimumSize(900, 600);
    resize(1200, 750);
    
    stackedWidget = new QStackedWidget(this);
    
    loginWidget = new LoginWidget(this);
    signUpWidget = new SignUpWidget(this);
    
    stackedWidget->addWidget(loginWidget);
    stackedWidget->addWidget(signUpWidget);
    
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(stackedWidget);
    
    connect(loginWidget, &LoginWidget::switchToSignUp, this, &MainWindow::showSignUp);
    connect(signUpWidget, &SignUpWidget::switchToLogin, this, &MainWindow::showLogin);
    connect(loginWidget, &LoginWidget::loginSuccessful, this, &MainWindow::onLoginSuccessful);
    
    showLogin();
}

void MainWindow::showLogin() {
    stackedWidget->setCurrentWidget(loginWidget);
}

void MainWindow::showSignUp() {
    stackedWidget->setCurrentWidget(signUpWidget);
}

void MainWindow::onLoginSuccessful() {
    QMessageBox::information(this, "Success", "Login successful!");
}