#include "signupwidget.h"
#include "database.h"

SignUpWidget::SignUpWidget(QWidget* parent) : QWidget(parent) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto* card = new CardWidget(this);
    card->setFixedSize(390, 720);

    auto* shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(60);
    shadow->setOffset(0, 12);
    shadow->setColor(QColor(0, 0, 0, 160));
    card->setGraphicsEffect(shadow);

    mainLayout->addWidget(card);

    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(36, 32, 36, 32);
    cardLayout->setSpacing(0);

    auto* icon = new AppIconWidget;
    auto* iconRow = new QHBoxLayout;
    iconRow->addStretch();
    iconRow->addWidget(icon);
    iconRow->addStretch();
    cardLayout->addLayout(iconRow);
    cardLayout->addSpacing(16);

    auto* title = new QLabel("Create Account");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color:#FFFFFF;font-size:24px;font-weight:700;"
                         "font-family:'Segoe UI',sans-serif;");
    cardLayout->addWidget(title);
    cardLayout->addSpacing(4);

    auto* subtitle = new QLabel("Start tracking your expenses today");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet("color:#8A9BB0;font-size:13px;"
                            "font-family:'Segoe UI',sans-serif;");
    cardLayout->addWidget(subtitle);
    cardLayout->addSpacing(22);

    cardLayout->addWidget(makeLabel("Full Name"));
    cardLayout->addSpacing(6);
    nameEdit = makeField("John Doe");
    cardLayout->addWidget(nameEdit);
    cardLayout->addSpacing(14);

    cardLayout->addWidget(makeLabel("Email Address"));
    cardLayout->addSpacing(6);
    emailEdit = makeField("you@example.com");
    cardLayout->addWidget(emailEdit);
    cardLayout->addSpacing(14);

    cardLayout->addWidget(makeLabel("Password"));
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

    auto* passToggle = new QPushButton("👁");
    passToggle->setFixedSize(28, 28);
    passToggle->setCursor(Qt::PointingHandCursor);
    passToggle->setStyleSheet("QPushButton{background:transparent;border:none;"
                              "color:#8A9BB0;font-size:13px;}"
                              "QPushButton:hover{color:#C8D6E5;}");
    passToggle->setParent(passwordEdit);
    passToggle->move(284, 9);
    passToggle->show();

    bool* pVis = new bool(false);
    connect(passToggle, &QPushButton::clicked, this, [=]() mutable {
        *pVis = !*pVis;
        passwordEdit->setEchoMode(*pVis ? QLineEdit::Normal : QLineEdit::Password);
        passToggle->setText(*pVis ? "🙈" : "👁");
    });

    cardLayout->addWidget(passwordEdit);
    cardLayout->addSpacing(14);

    cardLayout->addWidget(makeLabel("Confirm Password"));
    cardLayout->addSpacing(6);

    confirmEdit = new QLineEdit;
    confirmEdit->setEchoMode(QLineEdit::Password);
    confirmEdit->setFixedHeight(46);
    confirmEdit->setStyleSheet(R"(
        QLineEdit {
            background:rgba(255,255,255,0.07); border:1.5px solid rgba(255,255,255,0.15);
            border-radius:10px; color:#C8D6E5;
            font-size:14px; font-family:'Segoe UI',sans-serif;
            padding:0 42px 0 14px;
        }
        QLineEdit:focus { border-color:#00D296; background:rgba(255,255,255,0.10); }
    )");

    auto* confToggle = new QPushButton("👁");
    confToggle->setFixedSize(28, 28);
    confToggle->setCursor(Qt::PointingHandCursor);
    confToggle->setStyleSheet("QPushButton{background:transparent;border:none;"
                              "color:#8A9BB0;font-size:13px;}"
                              "QPushButton:hover{color:#C8D6E5;}");
    confToggle->setParent(confirmEdit);
    confToggle->move(284, 9);
    confToggle->show();

    bool* cVis = new bool(false);
    connect(confToggle, &QPushButton::clicked, this, [=]() mutable {
        *cVis = !*cVis;
        confirmEdit->setEchoMode(*cVis ? QLineEdit::Normal : QLineEdit::Password);
        confToggle->setText(*cVis ? "🙈" : "👁");
    });

    cardLayout->addWidget(confirmEdit);
    cardLayout->addSpacing(16);

    auto* termsRow = new QHBoxLayout;
    termsRow->setSpacing(8);
    auto* termsCheck = new QCheckBox;
    termsCheck->setStyleSheet(R"(
        QCheckBox::indicator {
            width:16px; height:16px;
            border:1.5px solid rgba(255,255,255,0.3);
            border-radius:4px;
            background:rgba(255,255,255,0.07);
        }
        QCheckBox::indicator:checked {
            background:#00D296;
            border-color:#00D296;
        }
    )");

    auto* termsLabel = new QLabel(
        "I agree to the <a href='#' style='color:#00D296;text-decoration:none;'>"
        "Terms of Service</a> and "
        "<a href='#' style='color:#3B7DDD;text-decoration:none;'>Privacy Policy</a>");
    termsLabel->setOpenExternalLinks(false);
    termsLabel->setTextFormat(Qt::RichText);
    termsLabel->setStyleSheet("color:#8A9BB0;font-size:13px;"
                              "font-family:'Segoe UI',sans-serif;");
    termsLabel->setWordWrap(true);

    termsRow->addWidget(termsCheck);
    termsRow->addWidget(termsLabel, 1);
    cardLayout->addLayout(termsRow);
    cardLayout->addSpacing(20);

    auto* createBtn = new QPushButton("Create Account");
    createBtn->setFixedHeight(48);
    createBtn->setCursor(Qt::PointingHandCursor);
    createBtn->setStyleSheet(R"(
        QPushButton {
            background:#3B7DDD;
            border:none; border-radius:10px; color:white;
            font-size:15px; font-weight:700;
            font-family:'Segoe UI',sans-serif;
        }
        QPushButton:hover { background:#5591E8; }
        QPushButton:pressed { background:#2E6BC4; }
    )");
    cardLayout->addWidget(createBtn);
    cardLayout->addSpacing(16);

    auto* bottomRow = new QHBoxLayout;
    bottomRow->setAlignment(Qt::AlignCenter);
    bottomRow->setSpacing(8);

    auto* alreadyLabel = new QLabel("Already have an account?");
    alreadyLabel->setStyleSheet("color:#8A9BB0;font-size:13px;"
                                "font-family:'Segoe UI',sans-serif;");

    auto* signInBtn = new QPushButton("Sign In");
    signInBtn->setCursor(Qt::PointingHandCursor);
    signInBtn->setFixedSize(80, 32);
    signInBtn->setStyleSheet(R"(
        QPushButton {
            background:#3B7DDD; border:none; border-radius:8px;
            color:white; font-size:13px; font-weight:600;
            font-family:'Segoe UI',sans-serif;
        }
        QPushButton:hover { background:#5591E8; }
        QPushButton:pressed { background:#2E6BC4; }
    )");

    bottomRow->addWidget(alreadyLabel);
    bottomRow->addWidget(signInBtn);
    cardLayout->addLayout(bottomRow);

    auto* footer = new QLabel("Secure and encrypted expense tracking");
    footer->setAlignment(Qt::AlignCenter);
    footer->setStyleSheet("color:#3A4A5A;font-size:12px;"
                          "font-family:'Segoe UI',sans-serif;");
    mainLayout->addSpacing(14);
    mainLayout->addWidget(footer);
    mainLayout->setAlignment(footer, Qt::AlignHCenter);
    
    connect(signInBtn, &QPushButton::clicked, this, &SignUpWidget::switchToLogin);
    connect(createBtn, &QPushButton::clicked, this, &SignUpWidget::onSignUpClicked);
}

void SignUpWidget::onSignUpClicked() {
    QString name = nameEdit->text().trimmed();
    QString email = emailEdit->text().trimmed();
    QString password = passwordEdit->text();
    QString confirm = confirmEdit->text();

    if (name.isEmpty() || email.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields.");
        return;
    }

    if (password != confirm) {
        QMessageBox::warning(this, "Error", "Passwords do not match.");
        return;
    }

    if (password.length() < 6) {
        QMessageBox::warning(this, "Error", "Password must be at least 6 characters.");
        return;
    }

    if (Database::instance().registerUser(name, email, password)) {
        QMessageBox::information(this, "Success", "Account created successfully!");
        emit accountCreated();
        emit switchToLogin();
    } else {
        QMessageBox::warning(this, "Error", "Registration failed. Email may already exist.");
    }
}