#include "signupwidget.h"

SignUpWidget::SignUpWidget(QWidget* parent) : QWidget(parent) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ── Card ──
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

    // Icon
    auto* icon = new AppIconWidget;
    auto* iconRow = new QHBoxLayout;
    iconRow->addStretch();
    iconRow->addWidget(icon);
    iconRow->addStretch();
    cardLayout->addLayout(iconRow);
    cardLayout->addSpacing(16);

    // Title
    auto* title = new QLabel("Create Account");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color:#FFFFFF;font-size:24px;font-weight:700;"
                         "font-family:'Segoe UI',sans-serif;");
    cardLayout->addWidget(title);
    cardLayout->addSpacing(4);

    // Subtitle
    auto* subtitle = new QLabel("Start tracking your expenses today");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet("color:#8A9BB0;font-size:13px;"
                            "font-family:'Segoe UI',sans-serif;");
    cardLayout->addWidget(subtitle);
    cardLayout->addSpacing(22);

    // Full Name
    cardLayout->addWidget(makeLabel("Full Name"));
    cardLayout->addSpacing(6);
    nameEdit = makeField("John Doe");
    cardLayout->addWidget(nameEdit);
    cardLayout->addSpacing(14);

    // Email
    cardLayout->addWidget(makeLabel("Email Address"));
    cardLayout->addSpacing(6);
    emailEdit = makeField("you@example.com");
    cardLayout->addWidget(emailEdit);
    cardLayout->addSpacing(14);

    // Password
    cardLayout->addWidget(makeLabel("Password"));
    cardLayout->addSpacing(6);
    passwordEdit = makeField("", true, card);
    auto* passToggle = new QPushButton("👁", passwordEdit);
    passToggle->setFixedSize(34, 34);
    passToggle->setCursor(Qt::PointingHandCursor);
    passToggle->setStyleSheet("QPushButton{background:transparent;border:none;"
                              "color:#8A9BB0;font-size:15px;}"
                              "QPushButton:hover{color:#C8D6E5;}");
    passToggle->move(passwordEdit->width() - 40, 6);
    bool* pVis = new bool(false);
    connect(passToggle, &QPushButton::clicked, this, [=]() mutable {
        *pVis = !*pVis;
        passwordEdit->setEchoMode(*pVis ? QLineEdit::Normal : QLineEdit::Password);
    });
    cardLayout->addWidget(passwordEdit);
    cardLayout->addSpacing(14);

    // Confirm Password
    cardLayout->addWidget(makeLabel("Confirm Password"));
    cardLayout->addSpacing(6);
    confirmEdit = makeField("", true, card);
    auto* confToggle = new QPushButton("👁", confirmEdit);
    confToggle->setFixedSize(34, 34);
    confToggle->setCursor(Qt::PointingHandCursor);
    confToggle->setStyleSheet("QPushButton{background:transparent;border:none;"
                              "color:#8A9BB0;font-size:15px;}"
                              "QPushButton:hover{color:#C8D6E5;}");
    confToggle->move(confirmEdit->width() - 40, 6);
    bool* cVis = new bool(false);
    connect(confToggle, &QPushButton::clicked, this, [=]() mutable {
        *cVis = !*cVis;
        confirmEdit->setEchoMode(*cVis ? QLineEdit::Normal : QLineEdit::Password);
    });
    cardLayout->addWidget(confirmEdit);
    cardLayout->addSpacing(16);

    // Terms checkbox
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

    // Create Account button
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

    // Already have an account? Sign In
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

    // Footer
    auto* footer = new QLabel("Secure and encrypted expense tracking");
    footer->setAlignment(Qt::AlignCenter);
    footer->setStyleSheet("color:#3A4A5A;font-size:12px;"
                          "font-family:'Segoe UI',sans-serif;");
    mainLayout->addSpacing(14);
    mainLayout->addWidget(footer);
    mainLayout->setAlignment(footer, Qt::AlignHCenter);
    
    // Connect Sign In button to emit signal
    connect(signInBtn, &QPushButton::clicked, this, &SignUpWidget::switchToLogin);
}