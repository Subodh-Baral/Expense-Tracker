#include "mainwindow.h"
#include "loginwidget.h"
#include "signupwidget.h"

MainWindow::MainWindow(QWidget* parent) : BackgroundWidget(parent) {
    setWindowTitle("ExpenseTracker");
    setMinimumSize(900, 600);
    resize(1200, 750);
    
    // Create stacked widget
    stackedWidget = new QStackedWidget(this);
    
    // Create pages
    loginWidget = new LoginWidget(this);
    signUpWidget = new SignUpWidget(this);
    
    // Add pages to stacked widget
    stackedWidget->addWidget(loginWidget);
    stackedWidget->addWidget(signUpWidget);
    
    // Set main layout
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(stackedWidget);
    
    // Connect signals
    connect(loginWidget, &LoginWidget::switchToSignUp, this, &MainWindow::showSignUp);
    connect(signUpWidget, &SignUpWidget::switchToLogin, this, &MainWindow::showLogin);
    
    // Start with login page
    showLogin();
}

void MainWindow::showLogin() {
    stackedWidget->setCurrentWidget(loginWidget);
}

void MainWindow::showSignUp() {
    stackedWidget->setCurrentWidget(signUpWidget);
}