#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include "widgets.h"

class SignUpWindow;

class LoginWindow : public BackgroundWidget {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget* parent = nullptr);

private slots:
    void navigateToSignUp();

private:
    QLineEdit* emailEdit    = nullptr;
    QLineEdit* passwordEdit = nullptr;
};

#endif // LOGIN_H