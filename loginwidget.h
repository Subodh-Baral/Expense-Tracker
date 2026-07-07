#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

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
#include "database.h"

class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr);

signals:
    void loginSuccessful(const QString &name, const QString &email);
    void switchToSignUp();

private:
    void setupUI();
    bool isValidEmail(const QString& email);

    QLineEdit *emailEdit;
    QLineEdit *passwordEdit;

private slots:
    void onSignInClicked();
};

#endif
