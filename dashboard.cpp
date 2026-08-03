#include "dashboard.h"
#include <QDoubleValidator>
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QDateEdit>
#include <QFormLayout>
#include <QMessageBox>
#include <QDate>
#include <QSizePolicy>
#include <QGraphicsDropShadowEffect>
#include <QFont>
#include <QApplication>
#include <cmath>

// ── Currency formatter (USD) ──────────────────────────────────────────────────
static QString ru(double v){
    // Format as ₹1,234.56
    bool neg = v < 0;
    double abs_v = qAbs(v);
    QString s = QString::number(abs_v, 'f', 2);
    // Insert thousands separators
    int dot = s.indexOf('.');
    int insert = dot - 3;
    while(insert > 0){ s.insert(insert, ','); insert -= 3; }
    return (neg ? "-₹" : "₹") + s;
}

static QColor catColour(const QString &cat){
    static QMap<QString,QColor> m={
        {"Food",{"#3b82f6"}},{"Transport",{"#8b5cf6"}},{"Utilities",{"#f59e0b"}},
        {"Entertainment",{"#ec4899"}},{"Shopping",{"#10b981"}},{"Health",{"#f97316"}},
        {"Education",{"#6366f1"}},{"Rent",{"#06b6d4"}},{"Other",{"#94a3b8"}}
    };
    return m.value(cat, QColor("#6366f1"));
}

QList<QColor> Dashboard::pal(){
    return{{"#3b82f6"},{"#8b5cf6"},{"#f59e0b"},{"#ec4899"},
           {"#10b981"},{"#f97316"},{"#6366f1"},{"#06b6d4"}};
}

// ── Drop shadow helper ────────────────────────────────────────────────────────
static void addShadow(QWidget* w, int blur=18, int yOff=4, int alpha=18){
    auto* eff = new QGraphicsDropShadowEffect(w);
    eff->setBlurRadius(blur);
    eff->setOffset(0, yOff);
    eff->setColor(QColor(0,0,0,alpha));
    w->setGraphicsEffect(eff);
}

Dashboard::Dashboard(QWidget *parent):QWidget(parent){
    // Set application-wide font
    QFont appFont("Inter", 10);
    if(!appFont.exactMatch()) appFont.setFamily("Segoe UI");
    QApplication::setFont(appFont);
    setupUI();
}
Dashboard::~Dashboard(){}

void Dashboard::setUserInfo(const QString &name,const QString &email){
    m_userEmail=email;
    m_userId=Database::instance().getUserId(email);
    if(avatarLabel&&!name.isEmpty())avatarLabel->setText(name.at(0).toUpper());
    if(userNameLabel)userNameLabel->setText(name);
    if(userEmailLabel)userEmailLabel->setText(email);
    QDate t=QDate::currentDate();
    Database::instance().ensureDefaultIncome(m_userId);
    Database::instance().processRollover(m_userId,t.year(),t.month());
    refreshDashboard();
}

// ── refreshDashboard ──────────────────────────────────────────────────────────
void Dashboard::refreshDashboard(){
    if(m_userId<0)return;
    Database &db=Database::instance();
    double inc=db.getTotalIncome(m_userId);
    double exp=db.getTotalExpenses(m_userId);
    double bal=inc-exp;
    if(dash_balance)dash_balance->setText(ru(bal));
    if(dash_income) dash_income->setText(ru(inc));
    if(dash_expense)dash_expense->setText(ru(exp));
    if(dash_balance){
        QString col=bal<0?"#dc2626":"#0f172a";
        dash_balance->setStyleSheet(QString("font-size:32px;font-weight:700;color:%1;font-family:'Inter','Segoe UI';background:transparent;").arg(col));
    }
    QDate t=QDate::currentDate();
    BudgetInfo bi=db.getBudgetInfo(m_userId,t.year(),t.month());
    if(budgetWarning){
        if(bi.remaining>=0&&bi.remaining<=500){
            budgetWarning->setText(QString("⚠  Only %1 remaining this month!").arg(ru(bi.remaining)));
            budgetWarning->setVisible(true);
        }else if(bi.remaining<0){
            budgetWarning->setText("⛔  You have exceeded your budget!");
            budgetWarning->setVisible(true);
        }else{budgetWarning->setVisible(false);}
    }
    if(dash_txList){
        while(QLayoutItem*i=dash_txList->takeAt(0)){if(i->widget())i->widget()->deleteLater();delete i;}
        auto all=db.getExpenses(m_userId);
        int shown=qMin(all.size(),6);
        for(int i=0;i<shown;i++) dash_txList->addWidget(makeExpenseRow(all[i],false));
        if(all.isEmpty()){
            auto*e=new QLabel("No transactions yet");
            e->setStyleSheet("color:#94a3b8;padding:32px;font-size:13px;background:transparent;");
            e->setAlignment(Qt::AlignCenter); dash_txList->addWidget(e);
        }
        dash_txList->addStretch();
    }
    if(dash_catList){
        while(QLayoutItem*i=dash_catList->takeAt(0)){if(i->widget())i->widget()->deleteLater();delete i;}
        auto cats=db.getAllCategoryTotals(m_userId);
        double total=0; for(double v:cats)total+=v;
        for(auto it=cats.cbegin();it!=cats.cend();++it)
            dash_catList->addWidget(makeCategoryProgressRow(it.key(),it.value(),total,catColour(it.key())));
        if(cats.isEmpty()){
            auto*e=new QLabel("No expenses yet");
            e->setStyleSheet("color:#94a3b8;font-size:13px;font-family:'Inter','Segoe UI';background:transparent;"); dash_catList->addWidget(e);}
        dash_catList->addStretch();
    }
}

// ── refreshExpenses ───────────────────────────────────────────────────────────
void Dashboard::refreshExpenses(const QString &filter){
    m_expFilter=filter;
    if(m_userId<0||!exp_listLayout)return;
    Database &db=Database::instance();
    if(exp_catCards){
        while(QLayoutItem*i=((QHBoxLayout*)exp_catCards->layout())->takeAt(0)){
            if(i->widget())i->widget()->deleteLater();delete i;}
        auto cats=db.getAllCategoryTotals(m_userId);
        for(auto it=cats.cbegin();it!=cats.cend();++it){
            int cnt=db.getExpenseCount(m_userId,it.key());
            QColor col=catColour(it.key());
            QFrame*card=new QFrame();
            bool active=(filter==it.key());
            card->setStyleSheet(active
                ? QString("QFrame{background:white;border-radius:14px;border:2px solid %1;}").arg(col.name())
                : "QFrame{background:white;border-radius:14px;border:1px solid #e2e8f0;}");
            card->setFixedSize(165,105);
            if(!active) addShadow(card,12,3,12);
            QVBoxLayout*vl=new QVBoxLayout(card); vl->setContentsMargins(14,14,14,14); vl->setSpacing(4);
            QLabel*dot=new QLabel(); dot->setFixedSize(12,12);
            dot->setStyleSheet(QString("background:%1;border-radius:6px;").arg(col.name())); vl->addWidget(dot);
            QLabel*nm=new QLabel(it.key()); nm->setStyleSheet("color:#64748b;font-size:12px;font-family:'Inter','Segoe UI';background:transparent;"); vl->addWidget(nm);
            QLabel*am=new QLabel(ru(it.value()));
            am->setStyleSheet("color:#0f172a;font-size:15px;font-weight:700;font-family:'Inter','Segoe UI';background:transparent;"); vl->addWidget(am);
            QLabel*cn=new QLabel(QString("%1 transactions").arg(cnt));
            cn->setStyleSheet("color:#94a3b8;font-size:11px;font-family:'Inter','Segoe UI';background:transparent;"); vl->addWidget(cn);
            card->setCursor(Qt::PointingHandCursor);
            QString catName=it.key();
            auto*btn=new QPushButton(card); btn->setGeometry(0,0,165,105);
            btn->setStyleSheet("QPushButton{background:transparent;border:none;}");
            connect(btn,&QPushButton::clicked,[=](){refreshExpenses(catName);});
            ((QHBoxLayout*)exp_catCards->layout())->addWidget(card);}
        ((QHBoxLayout*)exp_catCards->layout())->addStretch();}
    while(QLayoutItem*i=exp_listLayout->takeAt(0)){if(i->widget())i->widget()->deleteLater();delete i;}
    // getExpenses() returns BOTH income and expense rows — Transactions should
    // show everything, not just spending. getExpensesOnly() is still used
    // elsewhere (category summary cards, "Spending by Category") since those
    // are specifically about expense categories.
    auto all=db.getExpenses(m_userId);
    QList<Expense> shown;
    for(auto&e:all){
        bool match = (filter=="All")
                   || (filter=="Income" && e.isIncome)
                   || (!e.isIncome && e.category==filter);
        if(match) shown.append(e);
    }
    double incomeTotal=0, expenseTotal=0;
    for(auto&e:shown){ if(e.isIncome) incomeTotal+=e.amount; else expenseTotal+=e.amount; }
    QWidget*hdr=new QWidget(); hdr->setStyleSheet("background:white;border-radius:14px 14px 0 0;");
    QHBoxLayout*hl=new QHBoxLayout(hdr); hl->setContentsMargins(24,18,24,14);
    QString title;
    if(filter=="All") title="All Transactions";
    else if(filter=="Income") title="Income";
    else title=filter+" Expenses";
    QLabel*tl=new QLabel(QString("%1  (%2)").arg(title).arg(shown.size()));
    tl->setStyleSheet("color:#0f172a;font-size:15px;font-weight:700;font-family:'Inter','Segoe UI';background:transparent;"); hl->addWidget(tl); hl->addStretch();
    if(!shown.isEmpty()){
        if(incomeTotal>0 && expenseTotal>0){
            // Mixed view (e.g. "All"): a single total would blur together
            // money in and money out, so show both, clearly labeled.
            QLabel*incLbl=new QLabel(QString("Income: +%1").arg(ru(incomeTotal)));
            incLbl->setStyleSheet("color:#16a34a;font-size:13px;font-weight:700;font-family:'Inter','Segoe UI';background:transparent;"); hl->addWidget(incLbl);
            QLabel*sp=new QLabel("   "); sp->setStyleSheet("background:transparent;"); hl->addWidget(sp);
            QLabel*expLbl=new QLabel(QString("Expenses: -%1").arg(ru(expenseTotal)));
            expLbl->setStyleSheet("color:#ef4444;font-size:13px;font-weight:700;font-family:'Inter','Segoe UI';background:transparent;"); hl->addWidget(expLbl);
        }else if(incomeTotal>0){
            QLabel*tot=new QLabel("Total: "); tot->setStyleSheet("color:#64748b;font-size:13px;background:transparent;"); hl->addWidget(tot);
            QLabel*totAmt=new QLabel("+"+ru(incomeTotal));
            totAmt->setStyleSheet("color:#16a34a;font-size:14px;font-weight:700;font-family:'Inter','Segoe UI';background:transparent;"); hl->addWidget(totAmt);
        }else{
            QLabel*tot=new QLabel("Total: "); tot->setStyleSheet("color:#64748b;font-size:13px;background:transparent;"); hl->addWidget(tot);
            QLabel*totAmt=new QLabel(ru(expenseTotal));
            totAmt->setStyleSheet("color:#ef4444;font-size:14px;font-weight:700;font-family:'Inter','Segoe UI';background:transparent;"); hl->addWidget(totAmt);
        }
    }
    exp_listLayout->addWidget(hdr);
    QFrame*sep=new QFrame(); sep->setFrameShape(QFrame::HLine); sep->setStyleSheet("background:#f1f5f9;max-height:1px;border:none;"); exp_listLayout->addWidget(sep);
    if(shown.isEmpty()){
        QLabel*e=new QLabel("No transactions found."); e->setAlignment(Qt::AlignCenter);
        e->setStyleSheet("color:#94a3b8;font-size:14px;padding:48px;font-family:'Inter','Segoe UI';background:transparent;"); exp_listLayout->addWidget(e);
    }else{
        for(auto&e:shown) exp_listLayout->addWidget(makeExpenseRow(e,true));}
    exp_listLayout->addStretch();
}

// ── refreshAnalytics ──────────────────────────────────────────────────────────
void Dashboard::refreshAnalytics(){
    if(m_userId<0)return;
    Database &db=Database::instance();
    QDate t=QDate::currentDate();
    auto catMap=db.getMonthlyCategoryTotals(m_userId,t.year(),t.month());
    auto allCats=db.getAllCategoryTotals(m_userId);
    double totalSpent=db.getTotalExpenses(m_userId);
    auto monthly=db.getMonthlyTotals(m_userId,6);
    if(an_totalSpent) an_totalSpent->setText(ru(totalSpent));
    QString largestCat="-"; double largestAmt=0; double largestPct=0;
    for(auto it=allCats.cbegin();it!=allCats.cend();++it)
        if(it.value()>largestAmt){largestAmt=it.value();largestCat=it.key();}
    if(totalSpent>0)largestPct=largestAmt/totalSpent*100.0;
    if(an_largestCat) an_largestCat->setText(largestCat.isEmpty()?"-":largestCat);
    auto*largestSub=findChild<QLabel*>("an_largestSub");
    if(largestSub&&!largestCat.isEmpty())
        largestSub->setText(QString("%1  ·  %2% of total").arg(ru(largestAmt)).arg(largestPct,0,'f',0));
    double sumExp=0; for(auto&m:monthly)sumExp+=m.expense;
    double avg=monthly.isEmpty()?0:sumExp/monthly.size();
    if(an_avgMonthly) an_avgMonthly->setText(ru(avg));
    if(an_pie){
        QList<QColor> cols;
        for(auto it=catMap.cbegin();it!=catMap.cend();++it) cols.append(catColour(it.key()));
        an_pie->setData(catMap,cols);}
    if(an_line) an_line->setData(monthly);
    if(an_catBreakdown){
        while(QLayoutItem*i=an_catBreakdown->takeAt(0)){if(i->widget())i->widget()->deleteLater();delete i;}
        double tot=0; for(double v:allCats)tot+=v;
        for(auto it=allCats.cbegin();it!=allCats.cend();++it){
            QWidget*row=new QWidget(); row->setStyleSheet("background:white;");
            QHBoxLayout*hl=new QHBoxLayout(row); hl->setContentsMargins(0,10,0,10); hl->setSpacing(14);
            QColor col=catColour(it.key());
            QLabel*dot=new QLabel(); dot->setFixedSize(12,12);
            dot->setStyleSheet(QString("background:%1;border-radius:6px;").arg(col.name())); hl->addWidget(dot);
            QLabel*nm=new QLabel(it.key()); nm->setMinimumWidth(110);
            nm->setStyleSheet("color:#0f172a;font-size:13px;font-family:'Inter','Segoe UI';background:transparent;"); hl->addWidget(nm);
            QProgressBar*bar=new QProgressBar(); bar->setFixedHeight(8); bar->setRange(0,1000);
            bar->setValue(tot>0?int(it.value()/tot*1000):0); bar->setTextVisible(false);
            bar->setStyleSheet(QString(
                "QProgressBar{background:#f1f5f9;border-radius:4px;border:none;}"
                "QProgressBar::chunk{background:%1;border-radius:4px;}").arg(col.name()));
            hl->addWidget(bar,1);
            QLabel*al=new QLabel(ru(it.value())); al->setMinimumWidth(80); al->setAlignment(Qt::AlignRight);
            al->setStyleSheet("color:#0f172a;font-size:13px;font-weight:600;font-family:'Inter','Segoe UI';background:transparent;"); hl->addWidget(al);
            double pct=tot>0?it.value()/tot*100.0:0;
            QLabel*pl=new QLabel(QString("%1%").arg(pct,0,'f',0)); pl->setMinimumWidth(36); pl->setAlignment(Qt::AlignRight);
            pl->setStyleSheet("color:#94a3b8;font-size:12px;font-family:'Inter','Segoe UI';background:transparent;"); hl->addWidget(pl);
            an_catBreakdown->addWidget(row);
            QFrame*sep=new QFrame(); sep->setFrameShape(QFrame::HLine); sep->setStyleSheet("background:#f8fafc;max-height:1px;border:none;"); an_catBreakdown->addWidget(sep);}
        if(allCats.isEmpty()){QLabel*e=new QLabel("No expenses yet."); e->setStyleSheet("color:#94a3b8;font-size:13px;background:transparent;"); an_catBreakdown->addWidget(e);}
        an_catBreakdown->addStretch();}
}

// ── onAddExpenseClicked ───────────────────────────────────────────────────────
void Dashboard::onAddExpenseClicked(){
    QDialog dlg(this); dlg.setWindowTitle("Add Transaction"); dlg.setFixedWidth(440);
    dlg.setStyleSheet(R"(
        QDialog{background:#ffffff;border-radius:16px;}
        QLabel{color:#0f172a;font-size:13px;font-weight:500;font-family:'Inter','Segoe UI';}
        QLineEdit,QComboBox,QDateEdit{
            background:#f8fafc;color:#0f172a;
            border:1.5px solid #e2e8f0;border-radius:10px;
            padding:10px 14px;font-size:13px;font-family:'Inter','Segoe UI';}
        QLineEdit:focus,QComboBox:focus,QDateEdit:focus{border-color:#10b981;background:#ffffff;}
        QPushButton#ok{background:#10b981;color:white;border:none;border-radius:10px;
            padding:11px 32px;font-weight:700;font-size:14px;font-family:'Inter','Segoe UI';}
        QPushButton#ok:hover{background:#059669;}
        QPushButton#cancel{background:#f1f5f9;color:#64748b;border:none;
            border-radius:10px;padding:11px 22px;font-size:14px;}
        QPushButton#cancel:hover{background:#e2e8f0;})");
    QVBoxLayout*vl=new QVBoxLayout(&dlg); vl->setSpacing(18); vl->setContentsMargins(28,28,28,28);
    QLabel*h=new QLabel("Add Transaction");
    h->setStyleSheet("font-size:20px;font-weight:700;color:#0f172a;font-family:'Inter','Segoe UI';background:transparent;"); vl->addWidget(h);
    QLabel*sub=new QLabel("Record a new expense or income entry");
    sub->setStyleSheet("color:#94a3b8;font-size:13px;margin-bottom:4px;background:transparent;"); vl->addWidget(sub);
    QFrame*divider=new QFrame(); divider->setFrameShape(QFrame::HLine); divider->setStyleSheet("background:#f1f5f9;"); vl->addWidget(divider);

    // ── Expense / Income segmented toggle ───────────────────────────────────
    QWidget*toggleWrap=new QWidget();
    toggleWrap->setStyleSheet("background:#f1f5f9;border-radius:12px;");
    QHBoxLayout*toggleLay=new QHBoxLayout(toggleWrap);
    toggleLay->setContentsMargins(4,4,4,4); toggleLay->setSpacing(4);
    QPushButton*typeExpense=new QPushButton("Expense");
    QPushButton*typeIncome =new QPushButton("Income");
    QString typeOnExpense = "QPushButton{background:white;color:#dc2626;border:none;border-radius:9px;"
                             "padding:10px;font-size:13px;font-weight:700;font-family:'Inter','Segoe UI';}";
    QString typeOnIncome   = "QPushButton{background:white;color:#16a34a;border:none;border-radius:9px;"
                             "padding:10px;font-size:13px;font-weight:700;font-family:'Inter','Segoe UI';}";
    QString typeOff        = "QPushButton{background:transparent;color:#64748b;border:none;border-radius:9px;"
                             "padding:10px;font-size:13px;font-weight:600;font-family:'Inter','Segoe UI';}";
    typeExpense->setCheckable(true); typeIncome->setCheckable(true);
    typeExpense->setChecked(true); typeExpense->setStyleSheet(typeOnExpense); typeIncome->setStyleSheet(typeOff);
    typeExpense->setCursor(Qt::PointingHandCursor); typeIncome->setCursor(Qt::PointingHandCursor);
    toggleLay->addWidget(typeExpense); toggleLay->addWidget(typeIncome);
    vl->addWidget(toggleWrap);

    QFormLayout*form=new QFormLayout(); form->setSpacing(14); form->setLabelAlignment(Qt::AlignLeft);
    QLineEdit*desc=new QLineEdit(); desc->setPlaceholderText("e.g. Grocery Shopping"); form->addRow("Description",desc);
    QComboBox*cat=new QComboBox();
    QStringList expenseCats={"Food","Transport","Utilities","Entertainment","Shopping","Health","Education","Rent","Other"};
    QStringList incomeCats ={"Salary","Freelance","Business","Investment","Gift","Other Income"};
    cat->addItems(expenseCats);
    form->addRow("Category",cat);
    QLineEdit*amt=new QLineEdit(); amt->setPlaceholderText("0.00");
    amt->setValidator(new QDoubleValidator(0.01,9999999.99,2,amt)); form->addRow("Amount (₹)",amt);
    QDateEdit*de=new QDateEdit(QDate::currentDate()); de->setCalendarPopup(true); de->setDisplayFormat("yyyy-MM-dd");
    form->addRow("Date",de); vl->addLayout(form);

    // Swap category list + button styling based on selected type
    connect(typeExpense,&QPushButton::clicked,[=](){
        typeExpense->setChecked(true); typeIncome->setChecked(false);
        typeExpense->setStyleSheet(typeOnExpense); typeIncome->setStyleSheet(typeOff);
        cat->clear(); cat->addItems(expenseCats);});
    connect(typeIncome,&QPushButton::clicked,[=](){
        typeIncome->setChecked(true); typeExpense->setChecked(false);
        typeIncome->setStyleSheet(typeOnIncome); typeExpense->setStyleSheet(typeOff);
        cat->clear(); cat->addItems(incomeCats);});

    QHBoxLayout*br=new QHBoxLayout(); br->addStretch();
    QPushButton*cancel=new QPushButton("Cancel"); cancel->setObjectName("cancel");
    QPushButton*ok=new QPushButton("Save"); ok->setObjectName("ok");
    cancel->setCursor(Qt::PointingHandCursor); ok->setCursor(Qt::PointingHandCursor);
    br->addWidget(cancel); br->addWidget(ok); vl->addLayout(br);
    connect(cancel,&QPushButton::clicked,&dlg,&QDialog::reject);
    connect(ok,&QPushButton::clicked,[&](){
        QString d=desc->text().trimmed(); bool ok2;
        double a=amt->text().trimmed().toDouble(&ok2);
        bool isIncome=typeIncome->isChecked();
        if(d.isEmpty()){QMessageBox::warning(&dlg,"Error","Please enter a description.");return;}
        if(!ok2||a<=0){QMessageBox::warning(&dlg,"Error","Please enter a valid amount.");return;}
        // Budget-exceeded check only applies to expenses — adding income
        // should never be blocked by the budget limit.
        if(!isIncome){
            QDate t2=QDate::currentDate();
            BudgetInfo bi=Database::instance().getBudgetInfo(m_userId,t2.year(),t2.month());
            if((bi.spent+a)>(bi.totalBudget+500.0)){
                QMessageBox::warning(&dlg,"Over Budget",
                    QString("This would exceed your monthly budget.\nBudget: %1  |  Spent: %2")
                    .arg(ru(bi.totalBudget),ru(bi.spent)));
                return;}
        }
        if(!Database::instance().addExpense(m_userId,d,cat->currentText(),a,de->date().toString("yyyy-MM-dd"),isIncome)){
            QMessageBox::critical(&dlg,"Error","Failed to save."); return;}
        dlg.accept();});
    if(dlg.exec()==QDialog::Accepted){
        refreshDashboard(); refreshExpenses(m_expFilter);
        if(pageStack->currentIndex()==2)refreshAnalytics();}
}

// ── makeExpenseRow ────────────────────────────────────────────────────────────
QWidget* Dashboard::makeExpenseRow(const Expense &e, bool /*showBadge*/){
    QWidget*row=new QWidget();
    row->setStyleSheet(
        "QWidget{background:white;border-bottom:1px solid #f8fafc;}"
        "QWidget:hover{background:#fafafa;}");
    QHBoxLayout*h=new QHBoxLayout(row); h->setContentsMargins(24,16,24,16); h->setSpacing(16);

    // Icon badge
    QLabel*ico=new QLabel(); ico->setFixedSize(44,44); ico->setAlignment(Qt::AlignCenter);
    if(e.isIncome){
        ico->setPixmap(renderIcon(IconKind::ArrowUpRight,QColor("#16a34a"),18));
        ico->setStyleSheet("background:#dcfce7;border-radius:12px;");
    }else{
        ico->setPixmap(renderIcon(IconKind::ArrowDownRight,QColor("#dc2626"),18));
        ico->setStyleSheet("background:#fee2e2;border-radius:12px;");
    }
    h->addWidget(ico);

    // Description + category
    QVBoxLayout*mid=new QVBoxLayout(); mid->setSpacing(3);
    QLabel*dl=new QLabel(e.description);
    dl->setStyleSheet("color:#0f172a;font-size:14px;font-weight:600;font-family:'Inter','Segoe UI';background:transparent;border:none;");
    QLabel*catLabel=new QLabel(e.isIncome?"Income":e.category);
    catLabel->setStyleSheet("color:#94a3b8;font-size:12px;font-family:'Inter','Segoe UI';background:transparent;border:none;");
    mid->addWidget(dl); mid->addWidget(catLabel);
    h->addLayout(mid); h->addStretch();

    // Amount + date
    QVBoxLayout*right=new QVBoxLayout(); right->setSpacing(3); right->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    QString amtText=e.isIncome?"+"+ru(e.amount):"-"+ru(e.amount);
    QString amtCol =e.isIncome?"#16a34a":"#ef4444";
    QLabel*al=new QLabel(amtText);
    al->setStyleSheet(QString("color:%1;font-size:14px;font-weight:700;font-family:'Inter','Segoe UI';background:transparent;border:none;").arg(amtCol));
    al->setAlignment(Qt::AlignRight);
    QLabel*dt=new QLabel(e.date);
    dt->setStyleSheet("color:#94a3b8;font-size:12px;font-family:'Inter','Segoe UI';background:transparent;border:none;");
    dt->setAlignment(Qt::AlignRight);
    right->addWidget(al); right->addWidget(dt); h->addLayout(right);
    return row;
}

// ── makeCategoryProgressRow ───────────────────────────────────────────────────
QWidget* Dashboard::makeCategoryProgressRow(const QString &name,double amount,double total,const QColor &col){
    QWidget*w=new QWidget(); w->setStyleSheet("background:transparent;");
    QVBoxLayout*vl=new QVBoxLayout(w); vl->setContentsMargins(0,6,0,6); vl->setSpacing(6);
    QHBoxLayout*top=new QHBoxLayout();
    QLabel*nm=new QLabel(name);
    nm->setStyleSheet("color:#0f172a;font-size:13px;font-weight:500;font-family:'Inter','Segoe UI';background:transparent;");
    top->addWidget(nm); top->addStretch();
    QLabel*al=new QLabel(ru(amount));
    al->setStyleSheet("color:#0f172a;font-size:13px;font-weight:700;font-family:'Inter','Segoe UI';background:transparent;");
    top->addWidget(al); vl->addLayout(top);
    QProgressBar*bar=new QProgressBar(); bar->setFixedHeight(7); bar->setRange(0,1000);
    bar->setValue(total>0?int(amount/total*1000):0); bar->setTextVisible(false);
    bar->setStyleSheet(QString(
        "QProgressBar{background:#f1f5f9;border-radius:4px;border:none;}"
        "QProgressBar::chunk{background:%1;border-radius:4px;}").arg(col.name()));
    vl->addWidget(bar); return w;
}

// ── setupUI ───────────────────────────────────────────────────────────────────
void Dashboard::setupUI(){
    setMinimumSize(1200,800);
    // Main canvas: premium light gray
    setStyleSheet("QWidget#dashRoot{background:#f1f5f9;}");
    setObjectName("dashRoot");
    QHBoxLayout*root=new QHBoxLayout(this); root->setContentsMargins(0,0,0,0); root->setSpacing(0);
    createSidebar();
    contentWidget=new QWidget(); contentWidget->setStyleSheet("background:#f1f5f9;");
    QVBoxLayout*cl=new QVBoxLayout(contentWidget); cl->setContentsMargins(0,0,0,0); cl->setSpacing(0);
    pageStack=new QStackedWidget(); pageStack->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    createDashboardPage();
    createExpensesPage();
    createAnalyticsPage();
    createSettingsPage();
    cl->addWidget(pageStack);
    root->addWidget(sidebarWidget); root->addWidget(contentWidget,1);
}

// ── createSidebar ─────────────────────────────────────────────────────────────
void Dashboard::createSidebar(){
    sidebarWidget=new QWidget(); sidebarWidget->setFixedWidth(256);
    sidebarWidget->setObjectName("sidebar");
    sidebarWidget->setStyleSheet("#sidebar{background:#ffffff;border-right:1px solid #e2e8f0;}");
    QVBoxLayout*lay=new QVBoxLayout(sidebarWidget); lay->setContentsMargins(0,0,0,0); lay->setSpacing(0);

    // ── Profile area ──────────────────────────────────────────────────────────
    QWidget*prof=new QWidget();
    prof->setStyleSheet("background:white;");
    prof->setFixedHeight(88);
    QHBoxLayout*pl=new QHBoxLayout(prof); pl->setContentsMargins(20,20,20,20); pl->setSpacing(14);

    // Perfect circle avatar
    avatarLabel=new QLabel("U"); avatarLabel->setFixedSize(46,46);
    avatarLabel->setAlignment(Qt::AlignCenter);
    avatarLabel->setStyleSheet(
        "background:#10b981;color:white;border-radius:23px;"
        "font-weight:700;font-size:19px;font-family:'Inter','Segoe UI';");
    pl->addWidget(avatarLabel);

    QVBoxLayout*nc=new QVBoxLayout(); nc->setSpacing(2);
    userNameLabel=new QLabel("User");
    userNameLabel->setStyleSheet("color:#0f172a;font-size:14px;font-weight:700;font-family:'Inter','Segoe UI';background:transparent;");
    userEmailLabel=new QLabel("");
    userEmailLabel->setStyleSheet("color:#94a3b8;font-size:11px;font-family:'Inter','Segoe UI';background:transparent;");
    userEmailLabel->setWordWrap(true);
    nc->addWidget(userNameLabel); nc->addWidget(userEmailLabel);
    pl->addLayout(nc,1);
    lay->addWidget(prof);

    QFrame*topDiv=new QFrame(); topDiv->setFrameShape(QFrame::HLine);
    topDiv->setStyleSheet("background:#f1f5f9;max-height:1px;border:none;"); lay->addWidget(topDiv);

    // ── Nav buttons ───────────────────────────────────────────────────────────
    QWidget*nav=new QWidget(); nav->setStyleSheet("background:white;");
    QVBoxLayout*nl=new QVBoxLayout(nav); nl->setContentsMargins(14,24,14,24); nl->setSpacing(4);

    QString bs=R"(
        QPushButton{
            background:transparent;color:#64748b;border:none;
            padding:12px 16px;text-align:left;
            font-size:14px;border-radius:10px;
            font-family:'Inter','Segoe UI';
        }
        QPushButton:hover{background:#f8fafc;color:#0f172a;}
        QPushButton:checked{background:#f0fdf4;color:#059669;font-weight:700;}
    )";

    // Order matches reference: Dashboard, Transactions, Budget, Analytics, Settings
    navDashboard = new QPushButton(" Dashboard");
    navExpenses  = new QPushButton(" Transactions");
    navBudget    = new QPushButton(" Budget");
    navAnalytics = new QPushButton(" Analytics");
    navSettings  = new QPushButton(" Settings");

    for(auto*b:{navDashboard,navExpenses,navBudget,navAnalytics,navSettings}){
        b->setCheckable(true); b->setStyleSheet(bs);
        b->setIconSize(QSize(19,19));
        b->setCursor(Qt::PointingHandCursor); nl->addWidget(b);}
    navDashboard->setChecked(true);
    lay->addWidget(nav); lay->addStretch();

    // ── Logout ────────────────────────────────────────────────────────────────
    QFrame*botDiv=new QFrame(); botDiv->setFrameShape(QFrame::HLine);
    botDiv->setStyleSheet("background:#f1f5f9;max-height:1px;border:none;"); lay->addWidget(botDiv);
    QWidget*bot=new QWidget(); bot->setStyleSheet("background:white;");
    QVBoxLayout*bl=new QVBoxLayout(bot); bl->setContentsMargins(14,14,14,20);
    QPushButton*logout=new QPushButton(" Logout");
    logout->setStyleSheet(
        "QPushButton{background:transparent;color:#ef4444;border:none;"
        "padding:12px 16px;text-align:left;font-size:14px;"
        "border-radius:10px;font-weight:600;font-family:'Inter','Segoe UI';}"
        "QPushButton:hover{background:#fff1f2;}");
    logout->setIcon(makeQIcon(IconKind::LogOut,QColor("#ef4444"),19));
    logout->setIconSize(QSize(19,19));
    logout->setCursor(Qt::PointingHandCursor); bl->addWidget(logout); lay->addWidget(bot);

    connect(logout,      &QPushButton::clicked,this,&Dashboard::onLogoutClicked);
    connect(navDashboard,&QPushButton::clicked,this,&Dashboard::onDashboardClicked);
    connect(navExpenses, &QPushButton::clicked,this,&Dashboard::onExpensesClicked);
    connect(navBudget,&QPushButton::clicked,[=](){
        for(auto*b:{navDashboard,navExpenses,navBudget,navAnalytics,navSettings})b->setChecked(false);
        navBudget->setChecked(true); pageStack->setCurrentIndex(3); updateNavIcons();});
    connect(navAnalytics,&QPushButton::clicked,this,&Dashboard::onAnalyticsClicked);
    connect(navSettings, &QPushButton::clicked,this,&Dashboard::onSettingsClicked);

    updateNavIcons();
}

// ── updateNavIcons ────────────────────────────────────────────────────────────
// Repaints each sidebar icon in green when its button is checked, gray otherwise.
void Dashboard::updateNavIcons(){
    auto apply=[&](QPushButton*b, IconKind k){
        if(!b)return;
        QColor c = b->isChecked() ? QColor("#059669") : QColor("#64748b");
        b->setIcon(makeQIcon(k,c,19));
    };
    apply(navDashboard, IconKind::Home);
    apply(navExpenses,  IconKind::TrendUp);
    apply(navBudget,    IconKind::Wallet);
    apply(navAnalytics, IconKind::PieSlice);
    apply(navSettings,  IconKind::Gear);
}

// ── shared top header bar ─────────────────────────────────────────────────────
static QWidget* makeTopBar(const QString &title,const QString &sub,
                            const QString &btnLabel,QObject *recv,const char *slot){
    QWidget*bar=new QWidget(); bar->setFixedHeight(80);
    bar->setStyleSheet("background:white;border-bottom:1px solid #e2e8f0;");
    QHBoxLayout*l=new QHBoxLayout(bar); l->setContentsMargins(36,0,36,0);
    QVBoxLayout*tc=new QVBoxLayout(); tc->setSpacing(2);
    QLabel*tl=new QLabel(title);
    tl->setStyleSheet("font-size:22px;font-weight:700;color:#0f172a;font-family:'Inter','Segoe UI';background:transparent;");
    QLabel*sl=new QLabel(sub);
    sl->setStyleSheet("font-size:13px;color:#94a3b8;font-family:'Inter','Segoe UI';background:transparent;");
    tc->addWidget(tl); tc->addWidget(sl); l->addLayout(tc); l->addStretch();
    if(!btnLabel.isEmpty()){
        QPushButton*btn=new QPushButton(btnLabel); btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(
            "QPushButton{background:#10b981;color:white;border:none;"
            "border-radius:10px;padding:12px 26px;"
            "font-size:14px;font-weight:700;font-family:'Inter','Segoe UI';}"
            "QPushButton:hover{background:#059669;}");
        QObject::connect(btn,SIGNAL(clicked()),recv,slot); l->addWidget(btn);}
    return bar;
}

// ── createDashboardPage ───────────────────────────────────────────────────────
void Dashboard::createDashboardPage(){
    QWidget*page=new QWidget(); page->setStyleSheet("background:#f1f5f9;");
    QVBoxLayout*outer=new QVBoxLayout(page); outer->setContentsMargins(0,0,0,0); outer->setSpacing(0);
    outer->addWidget(makeTopBar("Dashboard","Track and manage your expenses","+ Add Expense",this,SLOT(onAddExpenseClicked())));

    // Budget warning banner
    budgetWarning=new QLabel(""); budgetWarning->setAlignment(Qt::AlignCenter);
    budgetWarning->setStyleSheet(
        "background:#fff7ed;color:#c2410c;font-size:13px;font-weight:600;"
        "padding:10px;border-bottom:1px solid #fed7aa;font-family:'Inter','Segoe UI';");
    budgetWarning->setVisible(false); outer->addWidget(budgetWarning);

    QScrollArea*scroll=new QScrollArea(); scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame); scroll->setStyleSheet("QScrollArea{background:#f1f5f9;border:none;}");
    QWidget*content=new QWidget(); content->setStyleSheet("background:#f1f5f9;");
    QVBoxLayout*cl=new QVBoxLayout(content); cl->setContentsMargins(32,28,32,32); cl->setSpacing(24);

    // ── 3 stat cards ─────────────────────────────────────────────────────────
    QHBoxLayout*cr=new QHBoxLayout(); cr->setSpacing(22);

    auto makeStatCard=[&](IconKind iconKind,const QString &iconFg,const QString &iconBg,
                          const QString &titleText,const QString &amtCol,
                          QLabel*&amtOut,
                          const QString &trendText,const QString &trendCol)->QFrame*{
        QFrame*c=new QFrame(); c->setObjectName("sc");
        c->setStyleSheet("QFrame#sc{background:white;border-radius:18px;border:none;}");
        c->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
        c->setMinimumHeight(165);
        addShadow(c,20,5,16);
        QVBoxLayout*vl=new QVBoxLayout(c); vl->setContentsMargins(26,24,26,22); vl->setSpacing(0);

        // Top row: icon badge + "This Month"
        QHBoxLayout*top=new QHBoxLayout(); top->setSpacing(0);
        QLabel*ico=new QLabel(); ico->setFixedSize(48,48); ico->setAlignment(Qt::AlignCenter);
        ico->setPixmap(renderIcon(iconKind,QColor(iconFg),22));
        ico->setStyleSheet(QString("background:%1;border-radius:14px;").arg(iconBg));
        top->addWidget(ico); top->addStretch();
        QLabel*per=new QLabel("This Month");
        per->setStyleSheet("color:#94a3b8;font-size:12px;font-family:'Inter','Segoe UI';background:transparent;");
        top->addWidget(per);
        vl->addLayout(top);
        vl->addSpacing(14);

        // Amount
        amtOut=new QLabel("₹0.00");
        amtOut->setStyleSheet(QString(
            "font-size:32px;font-weight:700;color:%1;font-family:'Inter','Segoe UI';background:transparent;").arg(amtCol));
        vl->addWidget(amtOut);
        vl->addSpacing(4);

        // Label
        QLabel*tl=new QLabel(titleText);
        tl->setStyleSheet("color:#64748b;font-size:13px;font-weight:500;font-family:'Inter','Segoe UI';background:transparent;");
        vl->addWidget(tl);
        vl->addSpacing(12);

        // Trend indicator
        if(!trendText.isEmpty()){
            QLabel*trend=new QLabel(trendText);
            trend->setStyleSheet(QString("color:%1;font-size:12px;font-weight:600;font-family:'Inter','Segoe UI';background:transparent;").arg(trendCol));
            vl->addWidget(trend);}
        return c;};

    cr->addWidget(makeStatCard(IconKind::Wallet,"#3b82f6","#dbeafe","Total Balance","#0f172a",dash_balance,"↗ 12.5% from last month","#16a34a"),1);
    cr->addWidget(makeStatCard(IconKind::ArrowUpRight,"#16a34a","#dcfce7","Total Income","#16a34a",dash_income,"↗ 8.2% from last month","#16a34a"),1);
    cr->addWidget(makeStatCard(IconKind::ArrowDownRight,"#dc2626","#fee2e2","Total Expenses","#dc2626",dash_expense,"↘ 3.1% from last month","#ef4444"),1);
    cl->addLayout(cr);

    // ── Bottom: transactions + category ──────────────────────────────────────
    QHBoxLayout*mr=new QHBoxLayout(); mr->setSpacing(22);

    // Recent Transactions card
    QFrame*txCard=new QFrame(); txCard->setObjectName("txCard");
    txCard->setStyleSheet("QFrame#txCard{background:white;border-radius:18px;border:none;}");
    txCard->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    addShadow(txCard,20,5,14);
    QVBoxLayout*txl=new QVBoxLayout(txCard); txl->setContentsMargins(0,0,0,0); txl->setSpacing(0);

    QHBoxLayout*txh=new QHBoxLayout(); txh->setContentsMargins(26,20,26,16);
    QLabel*txt=new QLabel("Recent Transactions");
    txt->setStyleSheet("font-size:16px;font-weight:700;color:#0f172a;font-family:'Inter','Segoe UI';background:transparent;");
    txh->addWidget(txt); txh->addStretch();
    QPushButton*va=new QPushButton("View All");
    va->setStyleSheet(
        "QPushButton{background:transparent;color:#10b981;border:none;"
        "font-size:13px;font-weight:600;font-family:'Inter','Segoe UI';}"
        "QPushButton:hover{color:#059669;}");
    va->setCursor(Qt::PointingHandCursor);
    connect(va,&QPushButton::clicked,this,&Dashboard::onExpensesClicked); txh->addWidget(va);
    txl->addLayout(txh);
    QFrame*s=new QFrame(); s->setFrameShape(QFrame::HLine); s->setStyleSheet("background:#f8fafc;max-height:1px;border:none;"); txl->addWidget(s);
    QWidget*txw=new QWidget(); txw->setStyleSheet("background:white;border-radius:18px;");
    dash_txList=new QVBoxLayout(txw); dash_txList->setContentsMargins(0,0,0,0); dash_txList->setSpacing(0);
    txl->addWidget(txw,1);
    mr->addWidget(txCard,3);

    // Spending by Category card
    QFrame*catCard=new QFrame(); catCard->setObjectName("catCard");
    catCard->setStyleSheet("QFrame#catCard{background:white;border-radius:18px;border:none;}");
    catCard->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    addShadow(catCard,20,5,14);
    QVBoxLayout*catl=new QVBoxLayout(catCard); catl->setContentsMargins(26,22,26,20); catl->setSpacing(12);
    QLabel*catt=new QLabel("Spending by Category");
    catt->setStyleSheet("font-size:16px;font-weight:700;color:#0f172a;font-family:'Inter','Segoe UI';background:transparent;"); catl->addWidget(catt);
    QFrame*catDiv=new QFrame(); catDiv->setFrameShape(QFrame::HLine); catDiv->setStyleSheet("background:#f8fafc;max-height:1px;border:none;"); catl->addWidget(catDiv);
    QWidget*catw=new QWidget(); catw->setStyleSheet("background:white;");
    dash_catList=new QVBoxLayout(catw); dash_catList->setContentsMargins(0,4,0,4); dash_catList->setSpacing(8);
    catl->addWidget(catw,1);

    // "View Detailed Report" footer link
    QFrame*catBot=new QFrame(); catBot->setFrameShape(QFrame::HLine); catBot->setStyleSheet("background:#f8fafc;max-height:1px;border:none;"); catl->addWidget(catBot);
    QPushButton*vdr=new QPushButton("View Detailed Report");
    vdr->setStyleSheet(
        "QPushButton{background:transparent;color:#10b981;border:none;"
        "font-size:13px;font-weight:600;padding:4px 0;"
        "font-family:'Inter','Segoe UI';}"
        "QPushButton:hover{color:#059669;}");
    vdr->setCursor(Qt::PointingHandCursor); vdr->setFixedHeight(32);
    connect(vdr,&QPushButton::clicked,this,&Dashboard::onAnalyticsClicked); catl->addWidget(vdr,0,Qt::AlignCenter);
    mr->addWidget(catCard,2);

    cl->addLayout(mr,1);
    scroll->setWidget(content); outer->addWidget(scroll,1);
    pageStack->addWidget(page); // index 0
}

// ── createExpensesPage ────────────────────────────────────────────────────────
void Dashboard::createExpensesPage(){
    QWidget*page=new QWidget(); page->setStyleSheet("background:#f1f5f9;");
    QVBoxLayout*outer=new QVBoxLayout(page); outer->setContentsMargins(0,0,0,0); outer->setSpacing(0);
    outer->addWidget(makeTopBar("Transactions","All your recorded income and expenses","+ Add Expense",this,SLOT(onAddExpenseClicked())));

    QFrame*filterCard=new QFrame(); filterCard->setStyleSheet("QFrame{background:white;border-bottom:1px solid #e2e8f0;}");
    QHBoxLayout*fl=new QHBoxLayout(filterCard); fl->setContentsMargins(28,14,28,14); fl->setSpacing(8);
    QStringList cats={"All","Income","Food","Transport","Utilities","Entertainment","Shopping","Health","Education","Rent","Other"};
    QString tabOn="QPushButton{background:#10b981;color:white;border:none;border-radius:18px;padding:6px 18px;font-size:12px;font-weight:700;font-family:'Inter','Segoe UI';}";
    QString tabOff="QPushButton{background:transparent;color:#64748b;border:1.5px solid #e2e8f0;border-radius:18px;padding:6px 16px;font-size:12px;font-family:'Inter','Segoe UI';}"
                   "QPushButton:hover{background:#f8fafc;color:#0f172a;}";
    QButtonGroup*grp=new QButtonGroup(filterCard); grp->setExclusive(true);
    for(const QString&cat:cats){
        QPushButton*btn=new QPushButton(cat); btn->setCheckable(true); btn->setStyleSheet(cat=="All"?tabOn:tabOff);
        btn->setCursor(Qt::PointingHandCursor); grp->addButton(btn); fl->addWidget(btn);
        connect(btn,&QPushButton::clicked,[=](){for(auto*b:grp->buttons())b->setStyleSheet(b==btn?tabOn:tabOff); refreshExpenses(cat);});}
    fl->addStretch(); outer->addWidget(filterCard);

    QWidget*body=new QWidget(); body->setStyleSheet("background:#f1f5f9;");
    QVBoxLayout*bl=new QVBoxLayout(body); bl->setContentsMargins(28,22,28,22); bl->setSpacing(18);
    exp_catCards=new QWidget(); exp_catCards->setStyleSheet("background:transparent;");
    new QHBoxLayout(exp_catCards); ((QHBoxLayout*)exp_catCards->layout())->setContentsMargins(0,0,0,0); ((QHBoxLayout*)exp_catCards->layout())->setSpacing(14);
    bl->addWidget(exp_catCards);
    QFrame*listCard=new QFrame(); listCard->setObjectName("lCard");
    listCard->setStyleSheet("QFrame#lCard{background:white;border-radius:18px;border:none;}");
    listCard->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    addShadow(listCard,16,4,12);
    QScrollArea*scroll2=new QScrollArea(); scroll2->setWidgetResizable(true); scroll2->setFrameShape(QFrame::NoFrame); scroll2->setStyleSheet("QScrollArea{background:white;border:none;}");
    QWidget*listW=new QWidget(); listW->setStyleSheet("background:white;");
    exp_listLayout=new QVBoxLayout(listW); exp_listLayout->setContentsMargins(0,0,0,0); exp_listLayout->setSpacing(0); scroll2->setWidget(listW);
    QVBoxLayout*lcl=new QVBoxLayout(listCard); lcl->setContentsMargins(0,0,0,0); lcl->addWidget(scroll2);
    bl->addWidget(listCard,1); outer->addWidget(body,1);
    pageStack->addWidget(page); // index 1
}

// ── createAnalyticsPage ───────────────────────────────────────────────────────
void Dashboard::createAnalyticsPage(){
    QWidget*page=new QWidget(); page->setStyleSheet("background:#f1f5f9;");
    QVBoxLayout*outer=new QVBoxLayout(page); outer->setContentsMargins(0,0,0,0); outer->setSpacing(0);
    outer->addWidget(makeTopBar("Analytics & Reports","Track your spending patterns","+ Add Expense",this,SLOT(onAddExpenseClicked())));
    QScrollArea*scroll=new QScrollArea(); scroll->setWidgetResizable(true); scroll->setFrameShape(QFrame::NoFrame); scroll->setStyleSheet("QScrollArea{background:#f1f5f9;border:none;}");
    QWidget*inner=new QWidget(); inner->setStyleSheet("background:#f1f5f9;");
    QVBoxLayout*cl=new QVBoxLayout(inner); cl->setContentsMargins(32,28,32,32); cl->setSpacing(22);

    // Top stat cards
    QHBoxLayout*sr=new QHBoxLayout(); sr->setSpacing(22);
    auto makeAnCard=[&](const QString &label,QLabel*&val,QLabel**sub=nullptr)->QFrame*{
        QFrame*c=new QFrame(); c->setObjectName("ac");
        c->setStyleSheet("QFrame#ac{background:white;border-radius:18px;border:none;}");
        addShadow(c,16,4,14);
        QVBoxLayout*vl=new QVBoxLayout(c); vl->setContentsMargins(26,22,26,22); vl->setSpacing(6);
        QLabel*lbl=new QLabel(label.toUpper());
        lbl->setStyleSheet("color:#94a3b8;font-size:10px;font-weight:700;letter-spacing:1.2px;font-family:'Inter','Segoe UI';background:transparent;"); vl->addWidget(lbl);
        val=new QLabel("₹0.00"); val->setStyleSheet("font-size:28px;font-weight:700;color:#0f172a;font-family:'Inter','Segoe UI';background:transparent;"); vl->addWidget(val);
        if(sub){*sub=new QLabel(""); (*sub)->setStyleSheet("color:#94a3b8;font-size:12px;font-family:'Inter','Segoe UI';background:transparent;"); vl->addWidget(*sub);} return c;};
    QLabel*largestSub=nullptr;
    sr->addWidget(makeAnCard("Total Spent",an_totalSpent),1);
    QFrame*lcCard=makeAnCard("Largest Category",an_largestCat,&largestSub);
    if(largestSub)largestSub->setObjectName("an_largestSub"); sr->addWidget(lcCard,1);
    QLabel*avgSub=nullptr; QFrame*avgCard=makeAnCard("Avg Monthly",an_avgMonthly,&avgSub);
    if(avgSub)avgSub->setText("Across 6 months"); sr->addWidget(avgCard,1); cl->addLayout(sr);

    // Charts row
    QHBoxLayout*chartRow=new QHBoxLayout(); chartRow->setSpacing(22);
    QFrame*pieCard=new QFrame(); pieCard->setObjectName("pc");
    pieCard->setStyleSheet("QFrame#pc{background:white;border-radius:18px;border:none;}");
    pieCard->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    addShadow(pieCard,16,4,12);
    QVBoxLayout*pl=new QVBoxLayout(pieCard); pl->setContentsMargins(26,22,26,22); pl->setSpacing(8);
    QLabel*pt=new QLabel("Spending by Category"); pt->setStyleSheet("font-size:16px;font-weight:700;color:#0f172a;font-family:'Inter','Segoe UI';background:transparent;"); pl->addWidget(pt);
    QLabel*ps=new QLabel(QDate::currentDate().toString("MMMM yyyy")+" breakdown"); ps->setStyleSheet("color:#94a3b8;font-size:12px;font-family:'Inter','Segoe UI';background:transparent;"); pl->addWidget(ps);
    an_pie=new PieChartWidget(); an_pie->setMinimumSize(300,300); pl->addWidget(an_pie,1); chartRow->addWidget(pieCard,1);
    QFrame*lineCard=new QFrame(); lineCard->setObjectName("lc");
    lineCard->setStyleSheet("QFrame#lc{background:white;border-radius:18px;border:none;}");
    lineCard->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    addShadow(lineCard,16,4,12);
    QVBoxLayout*ll=new QVBoxLayout(lineCard); ll->setContentsMargins(26,22,26,22); ll->setSpacing(8);
    QLabel*lt=new QLabel("Income vs Expenses"); lt->setStyleSheet("font-size:16px;font-weight:700;color:#0f172a;font-family:'Inter','Segoe UI';background:transparent;"); ll->addWidget(lt);
    QDate s6=QDate::currentDate().addMonths(-5);
    QLabel*ls=new QLabel(s6.toString("MMM")+" – "+QDate::currentDate().toString("MMM yyyy")); ls->setStyleSheet("color:#94a3b8;font-size:12px;font-family:'Inter','Segoe UI';background:transparent;"); ll->addWidget(ls);
    an_line=new LineChartWidget(); an_line->setMinimumHeight(260); ll->addWidget(an_line,1);
    QHBoxLayout*leg=new QHBoxLayout(); leg->setSpacing(20);
    auto addLeg=[&](const QString&label,const QString&col){
        QWidget*lw=new QWidget(); QHBoxLayout*lh=new QHBoxLayout(lw); lh->setContentsMargins(0,0,0,0); lh->setSpacing(6);
        QLabel*dot=new QLabel(); dot->setFixedSize(10,10); dot->setStyleSheet(QString("background:%1;border-radius:5px;").arg(col)); lh->addWidget(dot);
        QLabel*lb=new QLabel(label); lb->setStyleSheet(QString("color:#64748b;font-size:12px;font-family:'Inter','Segoe UI';background:transparent;")); lh->addWidget(lb); leg->addWidget(lw);};
    addLeg("Income","#10b981"); addLeg("Expenses","#ef4444"); leg->addStretch();
    ll->addLayout(leg); chartRow->addWidget(lineCard,1); cl->addLayout(chartRow);

    // Category breakdown card
    QFrame*bkCard=new QFrame(); bkCard->setObjectName("bkc");
    bkCard->setStyleSheet("QFrame#bkc{background:white;border-radius:18px;border:none;}");
    addShadow(bkCard,16,4,12);
    QVBoxLayout*bkl=new QVBoxLayout(bkCard); bkl->setContentsMargins(26,22,26,22); bkl->setSpacing(0);
    QLabel*bkt=new QLabel("Category Breakdown");
    bkt->setStyleSheet("font-size:16px;font-weight:700;color:#0f172a;margin-bottom:14px;font-family:'Inter','Segoe UI';background:transparent;"); bkl->addWidget(bkt);
    QWidget*bkw=new QWidget(); bkw->setStyleSheet("background:white;");
    an_catBreakdown=new QVBoxLayout(bkw); an_catBreakdown->setContentsMargins(0,0,0,0); an_catBreakdown->setSpacing(0);
    bkl->addWidget(bkw); cl->addWidget(bkCard);
    scroll->setWidget(inner); outer->addWidget(scroll,1);
    pageStack->addWidget(page); // index 2
}

// ── createSettingsPage ────────────────────────────────────────────────────────
void Dashboard::createSettingsPage(){
    QWidget*page=new QWidget(); page->setStyleSheet("background:#f1f5f9;");
    QVBoxLayout*outer=new QVBoxLayout(page); outer->setContentsMargins(0,0,0,0); outer->setSpacing(0);
    outer->addWidget(makeTopBar("Settings","Manage your preferences","",this,SLOT(onAddExpenseClicked())));
    QWidget*content=new QWidget(); content->setStyleSheet("background:#f1f5f9;");
    QVBoxLayout*cl=new QVBoxLayout(content); cl->setContentsMargins(32,32,32,32); cl->setSpacing(22);
    QFrame*sec=new QFrame(); sec->setObjectName("stCard");
    sec->setStyleSheet("QFrame#stCard{background:white;border-radius:18px;border:none;}");
    addShadow(sec,16,4,12);
    QVBoxLayout*sl=new QVBoxLayout(sec); sl->setContentsMargins(32,32,32,32); sl->setSpacing(18);
    QLabel*st=new QLabel("Monthly Budget"); st->setStyleSheet("font-size:18px;font-weight:700;color:#0f172a;font-family:'Inter','Segoe UI';background:transparent;"); sl->addWidget(st);
    QLabel*sd=new QLabel("Set your monthly spending limit. Any unspent balance rolls forward automatically.");
    sd->setWordWrap(true); sd->setStyleSheet("color:#64748b;font-size:13px;font-family:'Inter','Segoe UI';background:transparent;"); sl->addWidget(sd);
    QFrame*dv=new QFrame(); dv->setFrameShape(QFrame::HLine); dv->setStyleSheet("background:#f1f5f9;"); sl->addWidget(dv);
    auto addRow=[&](const QString&lbl,QLabel*&out,const QString&col){
        QHBoxLayout*rl=new QHBoxLayout();
        QLabel*ll=new QLabel(lbl); ll->setStyleSheet("color:#0f172a;font-size:14px;font-weight:500;font-family:'Inter','Segoe UI';background:transparent;"); rl->addWidget(ll); rl->addStretch();
        out=new QLabel("—"); out->setStyleSheet(QString("color:%1;font-size:16px;font-weight:700;font-family:'Inter','Segoe UI';background:transparent;").arg(col)); rl->addWidget(out); sl->addLayout(rl);};
    addRow("Current base budget:",set_base,"#6366f1");
    addRow("Rolled over from last month:",set_rollover,"#10b981");
    addRow("Income added this month:",set_income,"#16a34a");
    addRow("Total available this month:",set_total,"#0f172a");
    QFrame*dv2=new QFrame(); dv2->setFrameShape(QFrame::HLine); dv2->setStyleSheet("background:#f1f5f9;"); sl->addWidget(dv2);
    QLabel*nl=new QLabel("Set New Monthly Budget (₹):"); nl->setStyleSheet("color:#0f172a;font-size:14px;font-weight:600;font-family:'Inter','Segoe UI';background:transparent;"); sl->addWidget(nl);
    QHBoxLayout*ir=new QHBoxLayout(); ir->setSpacing(14);
    QLineEdit*inp=new QLineEdit(); inp->setPlaceholderText("e.g. 5000.00"); inp->setValidator(new QDoubleValidator(1,99999999,2,inp));
    inp->setStyleSheet("QLineEdit{background:#f8fafc;color:#0f172a;border:1.5px solid #e2e8f0;border-radius:10px;padding:10px 14px;font-size:14px;font-family:'Inter','Segoe UI';}"
                       "QLineEdit:focus{border-color:#10b981;background:white;}"); ir->addWidget(inp,1);
    QPushButton*save=new QPushButton("Save Budget"); save->setCursor(Qt::PointingHandCursor);
    save->setStyleSheet("QPushButton{background:#10b981;color:white;border:none;border-radius:10px;padding:10px 22px;font-size:14px;font-weight:700;font-family:'Inter','Segoe UI';}"
                        "QPushButton:hover{background:#059669;}"); ir->addWidget(save); sl->addLayout(ir);
    QLabel*status=new QLabel(""); status->setStyleSheet("font-size:13px;font-family:'Inter','Segoe UI';background:transparent;"); sl->addWidget(status);
    cl->addWidget(sec); cl->addStretch(); outer->addWidget(content,1);
    auto refresh=[=](){
        if(m_userId<0)return;
        QDate t=QDate::currentDate();
        BudgetInfo bi=Database::instance().getBudgetInfo(m_userId,t.year(),t.month());
        if(set_base)set_base->setText(ru(bi.baseBudget));
        if(set_rollover)set_rollover->setText(bi.rollover>0?ru(bi.rollover):"None");
        if(set_income)set_income->setText(bi.monthlyIncome>0?ru(bi.monthlyIncome):"None");
        if(set_total)set_total->setText(ru(bi.totalBudget));};
    connect(navSettings,&QPushButton::clicked,refresh);
    connect(save,&QPushButton::clicked,[=](){
        bool ok; double a=inp->text().trimmed().toDouble(&ok);
        if(!ok||a<=0){status->setStyleSheet("color:#ef4444;font-size:13px;background:transparent;"); status->setText("Please enter a valid amount.");return;}
        QDate t=QDate::currentDate();
        if(Database::instance().setBaseBudget(m_userId,t.year(),t.month(),a)){
            status->setStyleSheet("color:#16a34a;font-size:13px;font-weight:600;background:transparent;"); status->setText("✓ Budget updated successfully!"); inp->clear();
            refresh(); refreshDashboard();
        }else{status->setStyleSheet("color:#ef4444;font-size:13px;background:transparent;"); status->setText("Failed to save.");}});
    pageStack->addWidget(page); // index 3
}

// ── nav slots ─────────────────────────────────────────────────────────────────
void Dashboard::onLogoutClicked(){emit logoutRequested();}
void Dashboard::onDashboardClicked(){
    for(auto*b:{navDashboard,navExpenses,navBudget,navAnalytics,navSettings})b->setChecked(false);
    navDashboard->setChecked(true); pageStack->setCurrentIndex(0); updateNavIcons(); refreshDashboard();}
void Dashboard::onExpensesClicked(){
    for(auto*b:{navDashboard,navExpenses,navBudget,navAnalytics,navSettings})b->setChecked(false);
    navExpenses->setChecked(true); pageStack->setCurrentIndex(1); updateNavIcons(); refreshExpenses(m_expFilter);}
void Dashboard::onAnalyticsClicked(){
    for(auto*b:{navDashboard,navExpenses,navBudget,navAnalytics,navSettings})b->setChecked(false);
    navAnalytics->setChecked(true); pageStack->setCurrentIndex(2); updateNavIcons(); refreshAnalytics();}
void Dashboard::onSettingsClicked(){
    for(auto*b:{navDashboard,navExpenses,navBudget,navAnalytics,navSettings})b->setChecked(false);
    navSettings->setChecked(true); pageStack->setCurrentIndex(3); updateNavIcons();}
