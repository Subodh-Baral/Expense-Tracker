#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include <QRegularExpression>
#include "widgets.h"

class LoginWidget : public QWidget {
    Q_OBJECT

public:
    explicit LoginWidget(QWidget* parent = nullptr);

signals:
    void switchToSignUp();
    void loginSuccessful();

private slots:
    void onSignInClicked();

private:
    QLineEdit* emailEdit    = nullptr;
    QLineEdit* passwordEdit = nullptr;
    
    bool isValidEmail(const QString& email);
};

#endif