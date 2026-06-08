#include "loginwidget.h"

LoginWidget::LoginWidget(QWidget* parent) : QWidget(parent) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ── Card ──
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

    // Icon
    auto* icon = new AppIconWidget;
    auto* iconRow = new QHBoxLayout;
    iconRow->addStretch();
    iconRow->addWidget(icon);
    iconRow->addStretch();
    cardLayout->addLayout(iconRow);
    cardLayout->addSpacing(18);

    // Title
    auto* title = new QLabel("Welcome Back");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color:#FFFFFF;font-size:24px;font-weight:700;"
                         "font-family:'Segoe UI',sans-serif;");
    cardLayout->addWidget(title);
    cardLayout->addSpacing(6);

    // Subtitle
    auto* subtitle = new QLabel("Track your expenses with ease");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet("color:#8A9BB0;font-size:13px;"
                            "font-family:'Segoe UI',sans-serif;");
    cardLayout->addWidget(subtitle);
    cardLayout->addSpacing(24);

    // Email label
    auto* emailLabel = new QLabel("Email Address");
    emailLabel->setStyleSheet("color:#C8D6E5;font-size:13px;font-weight:600;"
                              "font-family:'Segoe UI',sans-serif;");
    cardLayout->addWidget(emailLabel);
    cardLayout->addSpacing(6);

    // Email input
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

    // Password label
    auto* passLabel = new QLabel("Password");
    passLabel->setStyleSheet("color:#C8D6E5;font-size:13px;font-weight:600;"
                             "font-family:'Segoe UI',sans-serif;");
    cardLayout->addWidget(passLabel);
    cardLayout->addSpacing(6);

    // Password input
    passwordEdit = new QLineEdit;
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setText("........");
    passwordEdit->setFixedHeight(46);
    passwordEdit->setStyleSheet(R"(
        QLineEdit {
            background:rgba(255,255,255,0.07); border:1.5px solid rgba(255,255,255,0.15);
            border-radius:10px; color:#C8D6E5;
            font-size:14px; font-family:'Segoe UI',sans-serif;
            padding:0 44px 0 14px;
        }
        QLineEdit:focus { border-color:#00D296; background:rgba(255,255,255,0.10); }
    )");

    // Eye toggle
    auto* toggleBtn = new QPushButton("👁", passwordEdit);
    toggleBtn->setFixedSize(34, 34);
    toggleBtn->setCursor(Qt::PointingHandCursor);
    toggleBtn->setStyleSheet(R"(
        QPushButton { background:transparent; border:none;
                      color:#8A9BB0; font-size:15px; }
        QPushButton:hover { color:#C8D6E5; }
    )");
    toggleBtn->move(passwordEdit->width() - 40, 6);

    bool* visible = new bool(false);
    connect(toggleBtn, &QPushButton::clicked, this, [=]() mutable {
        *visible = !*visible;
        passwordEdit->setEchoMode(*visible ? QLineEdit::Normal
                                           : QLineEdit::Password);
    });

    cardLayout->addWidget(passwordEdit);
    cardLayout->addSpacing(22);

    // Sign In button
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

    // Don't have an account? Sign Up
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

    // Footer
    auto* footer = new QLabel("Secure and encrypted expense tracking");
    footer->setAlignment(Qt::AlignCenter);
    footer->setStyleSheet("color:#3A4A5A;font-size:12px;"
                          "font-family:'Segoe UI',sans-serif;");
    mainLayout->addSpacing(14);
    mainLayout->addWidget(footer);
    mainLayout->setAlignment(footer, Qt::AlignHCenter);
    
    // Connect Sign Up button to emit signal
    connect(signUpBtn, &QPushButton::clicked, this, &LoginWidget::switchToSignUp);
}