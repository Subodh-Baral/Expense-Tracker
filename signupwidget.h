#ifndef SIGNUPWIDGET_H
#define SIGNUPWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QGraphicsDropShadowEffect>
#include "widgets.h"

class SignUpWidget : public QWidget {
    Q_OBJECT

public:
    explicit SignUpWidget(QWidget* parent = nullptr);

signals:
    void switchToLogin();

private:
    QLineEdit* nameEdit    = nullptr;
    QLineEdit* emailEdit   = nullptr;
    QLineEdit* passwordEdit = nullptr;
    QLineEdit* confirmEdit  = nullptr;
};

#endif