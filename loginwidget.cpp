#include "loginwidget.h"
#include "database.h"

LoginWidget::LoginWidget(QWidget* parent) : QWidget(parent) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto* card = new CardWidget(this);
    card->setFixedSize(340, 480);

    auto* shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(60);
    shadow->setOffset(0, 12);
    shadow->setColor(QColor(0, 0, 0, 160));
    card->setGraphicsEffect(shadow);

    mainLayout->addWidget(card);

    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(36, 36, 36, 36);
    cardLayout->setSpacing(0);

    auto* icon = new AppIconWidget;
    auto* iconRow = new QHBoxLayout;
    iconRow->addStretch();
    iconRow->addWidget(icon);
    iconRow->addStretch();
    cardLayout->addLayout(iconRow);
    cardLayout->addSpacing(18);

    auto* title = new QLabel("Welcome Back");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color:#FFFFFF;font-size:24px;font-weight:700;"
                         "font-family:'Segoe UI',sans-serif;");
    cardLayout->addWidget(title);
    cardLayout->addSpacing(6);

    auto* subtitle = new QLabel("Track your expenses with ease");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet("color:#8A9BB0;font-size:13px;"
                            "font-family:'Segoe UI',sans-serif;");
    cardLayout->addWidget(subtitle);
    cardLayout->addSpacing(24);

    auto* emailLabel = new QLabel("Email Address");
    emailLabel->setStyleSheet("color:#C8D6E5;font-size:13px;font-weight:600;"
                              "font-family:'Segoe UI',sans-serif;");
    cardLayout->addWidget(emailLabel);
    cardLayout->addSpacing(6);

    emailEdit = new QLineEdit;
    emailEdit->setPlaceholderText("you@example.com");
    emailEdit->setFixedHeight(46);
    emailEdit->setStyleSheet(R"(
        QLineEdit {
            background:rgba(255,255,255,0.07); border:1.5px solid rgba(255,255,255,0.15);
            border-radius:10px; color:#C8D6E5;
            font-size:14px; font-family:'Segoe UI',sans-serif;
            padding:0 14px;
        }
        QLineEdit:focus { border-color:#00D296; background:rgba(255,255,255,0.10); }
    )");
    cardLayout->addWidget(emailEdit);
    cardLayout->addSpacing(14);

    auto* passLabel = new QLabel("Password");
    passLabel->setStyleSheet("color:#C8D6E5;font-size:13px;font-weight:600;"
                             "font-family:'Segoe UI',sans-serif;");
    cardLayout->addWidget(passLabel);
    cardLayout->addSpacing(6);

    passwordEdit = new QLineEdit;
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setFixedHeight(46);
    passwordEdit->setStyleSheet(R"(
        QLineEdit {
            background:rgba(255,255,255,0.07); border:1.5px solid rgba(255,255,255,0.15);
            border-radius:10px; color:#C8D6E5;
            font-size:14px; font-family:'Segoe UI',sans-serif;
            padding:0 42px 0 14px;
        }
        QLineEdit:focus { border-color:#00D296; background:rgba(255,255,255,0.10); }
    )");

    auto* toggleBtn = new QPushButton("👁");
    toggleBtn->setFixedSize(28, 28);
    toggleBtn->setCursor(Qt::PointingHandCursor);
    toggleBtn->setStyleSheet("QPushButton{background:transparent;border:none;"
                             "color:#8A9BB0;font-size:13px;}"
                             "QPushButton:hover{color:#C8D6E5;}");
    toggleBtn->setParent(passwordEdit);
    toggleBtn->move(234, 9);
    toggleBtn->show();

    bool* visible = new bool(false);
    connect(toggleBtn, &QPushButton::clicked, this, [=]() mutable {
        *visible = !*visible;
        passwordEdit->setEchoMode(*visible ? QLineEdit::Normal
                                           : QLineEdit::Password);
        toggleBtn->setText(*visible ? "🙈" : "👁");
    });

    cardLayout->addWidget(passwordEdit);
    cardLayout->addSpacing(22);

    auto* signInBtn = new QPushButton("Sign In");
    signInBtn->setFixedHeight(48);
    signInBtn->setCursor(Qt::PointingHandCursor);
    signInBtn->setStyleSheet(R"(
        QPushButton {
            background:qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #00D296,stop:1 #00B578);
            border:none; border-radius:10px; color:white;
            font-size:15px; font-weight:700;
            font-family:'Segoe UI',sans-serif;
        }
        QPushButton:hover {
            background:qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #00E5A8,stop:1 #00C984);
        }
        QPushButton:pressed {
            background:qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #00B578,stop:1 #009E66);
        }
    )");
    cardLayout->addWidget(signInBtn);
    cardLayout->addSpacing(16);

    auto* bottomRow = new QHBoxLayout;
    bottomRow->setAlignment(Qt::AlignCenter);
    bottomRow->setSpacing(8);

    auto* noAccountLabel = new QLabel("Don't have an account?");
    noAccountLabel->setStyleSheet("color:#8A9BB0;font-size:13px;"
                                 "font-family:'Segoe UI',sans-serif;");

    auto* signUpBtn = new QPushButton("Sign Up");
    signUpBtn->setCursor(Qt::PointingHandCursor);
    signUpBtn->setFixedSize(80, 32);
    signUpBtn->setStyleSheet(R"(
        QPushButton {
            background:#3B7DDD; border:none; border-radius:8px;
            color:white; font-size:13px; font-weight:600;
            font-family:'Segoe UI',sans-serif;
        }
        QPushButton:hover { background:#5591E8; }
        QPushButton:pressed { background:#2E6BC4; }
    )");

    bottomRow->addWidget(noAccountLabel);
    bottomRow->addWidget(signUpBtn);
    cardLayout->addLayout(bottomRow);

    auto* footer = new QLabel("Secure and encrypted expense tracking");
    footer->setAlignment(Qt::AlignCenter);
    footer->setStyleSheet("color:#3A4A5A;font-size:12px;"
                          "font-family:'Segoe UI',sans-serif;");
    mainLayout->addSpacing(14);
    mainLayout->addWidget(footer);
    mainLayout->setAlignment(footer, Qt::AlignHCenter);
    
    connect(signUpBtn, &QPushButton::clicked, this, &LoginWidget::switchToSignUp);
    connect(signInBtn, &QPushButton::clicked, this, &LoginWidget::onSignInClicked);
}

void LoginWidget::onSignInClicked() {
    QString email = emailEdit->text().trimmed();
    QString password = passwordEdit->text();

    if (email.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields.");
        return;
    }

    if (!isValidEmail(email)) {
        QMessageBox::warning(this, "Error", "Please enter a valid email address.");
        return;
    }

    if (!Database::instance().emailExists(email)) {
        QMessageBox::warning(this, "Error", "No account found with this email.");
        return;
    }

    if (Database::instance().validateLogin(email, password)) {
        emit loginSuccessful();
    } else {
        QMessageBox::warning(this, "Error", "Incorrect password.");
    }
}

bool LoginWidget::isValidEmail(const QString& email) {
    QRegularExpression regex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    return regex.match(email).hasMatch();
}