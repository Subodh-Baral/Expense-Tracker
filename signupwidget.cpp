#include "signupwidget.h"
#include "database.h"

SignupWidget::SignupWidget(QWidget* parent) : QWidget(parent) { setupUI(); }

void SignupWidget::setupUI() {
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);

    BackgroundWidget *bg = new BackgroundWidget(this);
    QVBoxLayout *bgLay   = new QVBoxLayout(bg);
    bgLay->setAlignment(Qt::AlignCenter);
    bgLay->setContentsMargins(0,0,0,0);

    CardWidget *card = new CardWidget();
    card->setFixedSize(400, 530);
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(80);
    shadow->setOffset(0, 16);
    shadow->setColor(QColor(0,0,0,180));
    card->setGraphicsEffect(shadow);

    QVBoxLayout *lay = new QVBoxLayout(card);
    lay->setContentsMargins(36, 32, 36, 28);
    lay->setSpacing(0);

    // Icon
    QHBoxLayout *iconRow = new QHBoxLayout();
    iconRow->addStretch();
    iconRow->addWidget(new AppIconWidget());
    iconRow->addStretch();
    lay->addLayout(iconRow);
    lay->addSpacing(14);

    QLabel *title = new QLabel("Create Account");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color:white; font-size:24px; font-weight:700;");
    lay->addWidget(title);
    lay->addSpacing(4);

    QLabel *sub = new QLabel("Start tracking your expenses today");
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet("color:#8A9BB0; font-size:13px;");
    lay->addWidget(sub);
    lay->addSpacing(20);

    lay->addWidget(makeLabel("Full Name"));
    lay->addSpacing(4);
    nameEdit = makeField("John Doe", false);
    lay->addWidget(nameEdit);
    lay->addSpacing(12);

    lay->addWidget(makeLabel("Email"));
    lay->addSpacing(4);
    emailEdit = makeField("you@example.com", false);
    lay->addWidget(emailEdit);
    lay->addSpacing(12);

    lay->addWidget(makeLabel("Password"));
    lay->addSpacing(4);
    passwordEdit = makeField("", true);
    lay->addWidget(passwordEdit);
    lay->addSpacing(12);

    lay->addWidget(makeLabel("Confirm Password"));
    lay->addSpacing(4);
    confirmPasswordEdit = makeField("", true);
    lay->addWidget(confirmPasswordEdit);
    lay->addSpacing(20);

    QPushButton *signUpBtn = new QPushButton("Create Account");
    signUpBtn->setFixedHeight(46);
    signUpBtn->setCursor(Qt::PointingHandCursor);
    signUpBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #6366f1, stop:1 #4f46e5);
            border:none; border-radius:10px; color:white;
            font-size:15px; font-weight:700;
        }
        QPushButton:hover   { background:#7c7ff5; }
        QPushButton:pressed { background:#4338ca; }
    )");
    lay->addWidget(signUpBtn);
    lay->addSpacing(16);

    QHBoxLayout *linkRow = new QHBoxLayout();
    linkRow->setAlignment(Qt::AlignCenter);
    linkRow->setSpacing(6);
    QLabel *haveAcc = new QLabel("Already have an account?");
    haveAcc->setStyleSheet("color:#8A9BB0; font-size:13px;");
    QPushButton *loginBtn = new QPushButton("Sign In");
    loginBtn->setCursor(Qt::PointingHandCursor);
    loginBtn->setStyleSheet(R"(
        QPushButton { background:transparent; border:none;
            color:#6366f1; font-size:13px; font-weight:600; padding:0; }
        QPushButton:hover { color:#818cf8; }
    )");
    linkRow->addWidget(haveAcc);
    linkRow->addWidget(loginBtn);
    lay->addLayout(linkRow);

    bgLay->addWidget(card);
    root->addWidget(bg);

    connect(loginBtn,  &QPushButton::clicked, this, &SignupWidget::switchToLogin);
    connect(signUpBtn, &QPushButton::clicked, this, &SignupWidget::onSignUpClicked);
}

void SignupWidget::onSignUpClicked() {
    QString name     = nameEdit->text().trimmed();
    QString email    = emailEdit->text().trimmed();
    QString password = passwordEdit->text();
    QString confirm  = confirmPasswordEdit->text();

    if (name.isEmpty() || email.isEmpty() || password.isEmpty() || confirm.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields."); return; }
    if (!isValidEmail(email)) {
        QMessageBox::warning(this, "Error", "Please enter a valid email address."); return; }
    if (Database::instance().emailExists(email)) {
        QMessageBox::warning(this, "Error",
            "An account with this email already exists. Please sign in instead."); return; }
    if (password != confirm) {
        QMessageBox::warning(this, "Error", "Passwords do not match."); return; }
    if (!isValidPassword(password)) {
        QMessageBox::warning(this, "Error",
            "Password must be at least 8 characters and contain at least one number."); return; }
    if (!Database::instance().registerUser(name, email, password)) {
        QMessageBox::critical(this, "Error", "Failed to create account. Please try again."); return; }

    emit signupSuccessful(name, email);
}

bool SignupWidget::isValidEmail(const QString &email) {
    QRegularExpression re("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    return re.match(email).hasMatch();
}

bool SignupWidget::isValidPassword(const QString &password) {
    if (password.length() < 8) return false;
    for (QChar ch : password) if (ch.isDigit()) return true;
    return false;
}
