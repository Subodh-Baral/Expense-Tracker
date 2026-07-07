#ifndef SIGNUPWIDGET_H
#define SIGNUPWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QRegularExpression>
#include <QGraphicsDropShadowEffect>
#include "widgets.h"

class SignupWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SignupWidget(QWidget *parent = nullptr);

signals:
    void switchToLogin();
    void signupSuccessful(const QString &name, const QString &email);

private:
    void setupUI();
    bool isValidEmail(const QString& email);
    bool isValidPassword(const QString& password);

    QLineEdit *nameEdit;
    QLineEdit *emailEdit;
    QLineEdit *passwordEdit;
    QLineEdit *confirmPasswordEdit;

private slots:
    void onSignUpClicked();
};

#endif
