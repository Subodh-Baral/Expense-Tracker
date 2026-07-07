#include "loginwidget.h"

LoginWidget::LoginWidget(QWidget* parent) : QWidget(parent) { setupUI(); }

void LoginWidget::setupUI() {
    // Full-window layout
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);

    BackgroundWidget *bg = new BackgroundWidget(this);
    QVBoxLayout *bgLay   = new QVBoxLayout(bg);
    bgLay->setAlignment(Qt::AlignCenter);
    bgLay->setContentsMargins(0,0,0,0);

    // Card
    CardWidget *card = new CardWidget();
    card->setFixedSize(400, 460);
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(80);
    shadow->setOffset(0, 16);
    shadow->setColor(QColor(0,0,0,180));
    card->setGraphicsEffect(shadow);

    QVBoxLayout *lay = new QVBoxLayout(card);
    lay->setContentsMargins(36, 36, 36, 32);
    lay->setSpacing(0);

    // Icon
    QHBoxLayout *iconRow = new QHBoxLayout();
    iconRow->addStretch();
    iconRow->addWidget(new AppIconWidget());
    iconRow->addStretch();
    lay->addLayout(iconRow);
    lay->addSpacing(16);

    // Title
    QLabel *title = new QLabel("Welcome Back");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color:white; font-size:24px; font-weight:700;");
    lay->addWidget(title);
    lay->addSpacing(4);

    QLabel *sub = new QLabel("Sign in to your account");
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet("color:#8A9BB0; font-size:13px;");
    lay->addWidget(sub);
    lay->addSpacing(24);

    // Email
    lay->addWidget(makeLabel("Email"));
    lay->addSpacing(4);
    emailEdit = makeField("you@example.com", false);
    lay->addWidget(emailEdit);
    lay->addSpacing(14);

    // Password
    lay->addWidget(makeLabel("Password"));
    lay->addSpacing(4);
    passwordEdit = makeField("", true);
    lay->addWidget(passwordEdit);
    lay->addSpacing(22);

    // Sign In button
    QPushButton *signInBtn = new QPushButton("Sign In");
    signInBtn->setFixedHeight(46);
    signInBtn->setCursor(Qt::PointingHandCursor);
    signInBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #6366f1, stop:1 #4f46e5);
            border:none; border-radius:10px; color:white;
            font-size:15px; font-weight:700;
        }
        QPushButton:hover   { background:#7c7ff5; }
        QPushButton:pressed { background:#4338ca; }
    )");
    lay->addWidget(signInBtn);
    lay->addSpacing(18);

    // Sign Up link
    QHBoxLayout *linkRow = new QHBoxLayout();
    linkRow->setAlignment(Qt::AlignCenter);
    linkRow->setSpacing(6);
    QLabel *noAcc = new QLabel("Don't have an account?");
    noAcc->setStyleSheet("color:#8A9BB0; font-size:13px;");
    QPushButton *signUpBtn = new QPushButton("Sign Up");
    signUpBtn->setCursor(Qt::PointingHandCursor);
    signUpBtn->setStyleSheet(R"(
        QPushButton { background:transparent; border:none;
            color:#6366f1; font-size:13px; font-weight:600; padding:0; }
        QPushButton:hover { color:#818cf8; }
    )");
    linkRow->addWidget(noAcc);
    linkRow->addWidget(signUpBtn);
    lay->addLayout(linkRow);

    bgLay->addWidget(card);
    root->addWidget(bg);

    connect(signUpBtn, &QPushButton::clicked, this, &LoginWidget::switchToSignUp);
    connect(signInBtn, &QPushButton::clicked, this, &LoginWidget::onSignInClicked);
}

void LoginWidget::onSignInClicked() {
    QString email    = emailEdit->text().trimmed();
    QString password = passwordEdit->text();

    if (email.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields."); return; }
    if (!isValidEmail(email)) {
        QMessageBox::warning(this, "Error", "Please enter a valid email address."); return; }

    Database &db = Database::instance();
    if (!db.validateLogin(email, password)) {
        QMessageBox::warning(this, "Error", "Invalid email or password."); return; }

    emit loginSuccessful(db.getUserName(email), email);
}

bool LoginWidget::isValidEmail(const QString &email) {
    QRegularExpression re("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    return re.match(email).hasMatch();
}
