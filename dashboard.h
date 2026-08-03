#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <QStackedWidget>
#include <QProgressBar>
#include <QPainter>
#include <QPainterPath>
#include <QMap>
#include <QList>
#include <QButtonGroup>
#include <QDate>
#include <QLocale>
#include <QtMath>
#include <QIcon>
#include <QPixmap>
#include "database.h"

// ── Vector line-icon rendering (Feather/Lucide style) ─────────────────────────
enum class IconKind { Home, TrendUp, Wallet, PieSlice, Gear, ArrowUpRight, ArrowDownRight, LogOut };

inline QPixmap renderIcon(IconKind type, const QColor &color, int size=22, qreal strokeW=2.0){
    qreal dpr=3.0;
    QPixmap pm(int(size*dpr), int(size*dpr));
    pm.fill(Qt::transparent);
    pm.setDevicePixelRatio(dpr);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    QPen pen(color, strokeW);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    QRectF r(strokeW, strokeW, size-2*strokeW, size-2*strokeW);
    qreal w=r.width(), h=r.height();
    switch(type){
    case IconKind::Home: {
        QPointF apex(r.left()+w*0.5, r.top());
        p.drawLine(apex, QPointF(r.left(), r.top()+h*0.45));
        p.drawLine(apex, QPointF(r.right(), r.top()+h*0.45));
        QRectF base(r.left()+w*0.16, r.top()+h*0.45, w*0.68, h*0.55);
        p.drawRoundedRect(base,1,1);
        QRectF door(r.left()+w*0.40, r.top()+h*0.66, w*0.20, h*0.34);
        p.drawRect(door);
        break; }
    case IconKind::TrendUp: {
        QPolygonF poly;
        poly << QPointF(r.left(), r.top()+h*0.78)
             << QPointF(r.left()+w*0.28, r.top()+h*0.46)
             << QPointF(r.left()+w*0.50, r.top()+h*0.64)
             << QPointF(r.right()-w*0.06, r.top()+h*0.14);
        p.drawPolyline(poly);
        QPolygonF head;
        head << QPointF(r.right()-w*0.32, r.top()+h*0.14)
             << QPointF(r.right()-w*0.02, r.top()+h*0.14)
             << QPointF(r.right()-w*0.02, r.top()+h*0.14+h*0.30);
        p.drawPolyline(head);
        break; }
    case IconKind::Wallet: {
        QRectF body(r.left(), r.top()+h*0.14, w, h*0.70);
        p.drawRoundedRect(body,3,3);
        QRectF flap(r.right()-w*0.36, r.top()+h*0.38, w*0.30, h*0.24);
        p.drawRoundedRect(flap,2,2);
        break; }
    case IconKind::PieSlice: {
        p.drawEllipse(r);
        QPointF c=r.center();
        p.drawLine(c, QPointF(c.x(), r.top()));
        qreal rad=r.width()/2.0, ang=300.0;
        QPointF edge(c.x()+rad*qCos(qDegreesToRadians(ang)), c.y()-rad*qSin(qDegreesToRadians(ang)));
        p.drawLine(c, edge);
        break; }
    case IconKind::Gear: {
        QPointF c=r.center();
        qreal rOuter=w*0.50, rInner=w*0.34, toothLen=w*0.14;
        for(int i=0;i<8;i++){
            qreal ang=qDegreesToRadians(i*45.0);
            QPointF p1(c.x()+rInner*qCos(ang), c.y()+rInner*qSin(ang));
            QPointF p2(c.x()+(rOuter+toothLen*0.3)*qCos(ang), c.y()+(rOuter+toothLen*0.3)*qSin(ang));
            p.drawLine(p1,p2);
        }
        p.drawEllipse(c, rInner, rInner);
        p.setBrush(color); p.drawEllipse(c, w*0.09, w*0.09); p.setBrush(Qt::NoBrush);
        break; }
    case IconKind::ArrowUpRight: {
        p.drawLine(QPointF(r.left(), r.bottom()), QPointF(r.right(), r.top()));
        QPolygonF head;
        head << QPointF(r.right()-w*0.42, r.top())
             << QPointF(r.right(), r.top())
             << QPointF(r.right(), r.top()+h*0.42);
        p.drawPolyline(head);
        break; }
    case IconKind::ArrowDownRight: {
        p.drawLine(QPointF(r.left(), r.top()), QPointF(r.right(), r.bottom()));
        QPolygonF head;
        head << QPointF(r.right()-w*0.42, r.bottom())
             << QPointF(r.right(), r.bottom())
             << QPointF(r.right(), r.bottom()-h*0.42);
        p.drawPolyline(head);
        break; }
    case IconKind::LogOut: {
        QRectF door(r.left(), r.top(), w*0.55, h);
        p.drawRoundedRect(door,2,2);
        p.drawLine(QPointF(r.left()+w*0.45, r.top()+h*0.5), QPointF(r.right(), r.top()+h*0.5));
        QPolygonF head;
        head << QPointF(r.right()-w*0.26, r.top()+h*0.30)
             << QPointF(r.right(), r.top()+h*0.5)
             << QPointF(r.right()-w*0.26, r.top()+h*0.70);
        p.drawPolyline(head);
        break; }
    }
    p.end();
    return pm;
}
inline QIcon makeQIcon(IconKind type, const QColor &color, int size=22){
    return QIcon(renderIcon(type,color,size));
}

// ── Donut pie chart with slice labels ─────────────────────────────────────────
class PieChartWidget : public QWidget {
    Q_OBJECT
public:
    explicit PieChartWidget(QWidget *p=nullptr):QWidget(p){setMinimumSize(280,280);}
    void setData(const QMap<QString,double>&d,const QList<QColor>&c){m_data=d;m_col=c;update();}
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        if(m_data.isEmpty()){p.setPen(QColor("#94a3b8"));p.drawText(rect(),Qt::AlignCenter,"No data");return;}
        double total=0; for(double v:m_data)total+=v;
        int sz=qMin(width(),height())-60;
        QRectF r((width()-sz)/2.0,(height()-sz)/2.0,sz,sz);
        double angle=-90.0*16; int ci=0;
        QList<QPair<double,double>> midAngles; // for label positioning
        for(auto it=m_data.cbegin();it!=m_data.cend();++it,++ci){
            double span=it.value()/total*360.0*16;
            p.setBrush(m_col[ci%m_col.size()]); p.setPen(Qt::NoPen);
            p.drawPie(r,int(angle),int(span));
            midAngles.append({angle+span/2.0, it.value()/total*100.0});
            angle+=span;
        }
        // Hole
        int hole=int(sz*0.5);
        QRectF hr((width()-hole)/2.0,(height()-hole)/2.0,hole,hole);
        p.setBrush(Qt::white); p.setPen(Qt::NoPen); p.drawEllipse(hr);
        // Slice labels outside
        ci=0;
        for(auto it=m_data.cbegin();it!=m_data.cend();++it,++ci){
            double midRad=midAngles[ci].first/16.0*M_PI/180.0;
            double pct=midAngles[ci].second;
            if(pct<5.0){continue;} // skip tiny slices
            // Stagger the radius so adjacent thin slices (e.g. two 13% slices
            // next to each other) don't place their labels at the exact same
            // distance from center and collide.
            double radiusFactor = (ci%2==0) ? 0.60 : 0.74;
            double lx=width()/2.0+cos(midRad)*(sz*radiusFactor);
            double ly=height()/2.0+sin(midRad)*(sz*radiusFactor);
            p.setPen(m_col[ci%m_col.size()]);
            p.setFont(QFont("Segoe UI",9,QFont::Bold));
            QString lbl=QString("%1 %2%").arg(it.key()).arg(pct,0,'f',0);
            // Size the label box to the ACTUAL rendered text width instead of
            // a fixed box. A fixed-width box clips text that doesn't fit —
            // Qt's drawText(QRectF,...) clips by default — and because the
            // text is centered, clipping trims equal amounts off BOTH ends,
            // which is what was chopping the leading letter off labels like
            // "Entertainment" and "Education". Qt::TextDontClip is also
            // passed as a safety net in case any label is still wider than
            // the widget itself.
            QFontMetrics fm(p.font());
            int textW = fm.horizontalAdvance(lbl) + 12;
            int textH = fm.height() + 4;
            QRectF labelRect(lx-textW/2.0, ly-textH/2.0, textW, textH);
            // A correctly-sized box (above) stops Qt clipping text that's
            // wider than its own rect, but a label anchored near the
            // horizontal/vertical extremes (cos/sin near ±1 — e.g. a big
            // slice pointing due right) can still push part of that rect
            // past the WIDGET's own edge, which Qt always clips regardless
            // of drawText's rect. Nudge the rect back inside the widget so
            // it's never cut off — this is what was still chopping the
            // start of "Education" even after the box-sizing fix.
            const double margin=2.0;
            if(labelRect.left()<margin) labelRect.moveLeft(margin);
            if(labelRect.right()>width()-margin) labelRect.moveRight(width()-margin);
            if(labelRect.top()<margin) labelRect.moveTop(margin);
            if(labelRect.bottom()>height()-margin) labelRect.moveBottom(height()-margin);
            p.drawText(labelRect, Qt::AlignCenter | Qt::TextDontClip, lbl);
        }
    }
private:
    QMap<QString,double>m_data; QList<QColor>m_col;
};

// ── Line chart (Income vs Expenses) ──────────────────────────────────────────
class LineChartWidget : public QWidget {
    Q_OBJECT
public:
    explicit LineChartWidget(QWidget *p=nullptr):QWidget(p){setMinimumHeight(220);}
    void setData(const QList<MonthlyTotal>&d){m_data=d;update();}
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        if(m_data.isEmpty()){p.setPen(QColor("#94a3b8"));p.drawText(rect(),Qt::AlignCenter,"No data");return;}
        int pad=50,botPad=30,topPad=20;
        int W=width()-pad, H=height()-botPad-topPad;
        double maxV=0;
        for(auto&m:m_data){maxV=qMax(maxV,qMax(m.income,m.expense));}
        if(maxV==0)maxV=1;
        int n=m_data.size();
        // Grid lines
        p.setPen(QPen(QColor("#f1f5f9"),1));
        for(int i=0;i<=4;i++){
            int y=topPad+H-int(H*i/4.0);
            p.drawLine(pad,y,width()-10,y);
            p.setPen(QColor("#94a3b8")); p.setFont(QFont("Segoe UI",8));
            p.drawText(0,y-8,pad-5,16,Qt::AlignRight|Qt::AlignVCenter,
                       QString("₹%1").arg(int(maxV*i/4)));
            p.setPen(QPen(QColor("#f1f5f9"),1));
        }
        auto xOf=[&](int i){return pad+int(i*(W-pad)/(n>1?n-1:1));};
        auto yOf=[&](double v){return topPad+H-int(v/maxV*H);};
        // Draw expense shaded area
        QPolygonF expArea;
        expArea<<QPointF(xOf(0),topPad+H);
        for(int i=0;i<n;i++) expArea<<QPointF(xOf(i),yOf(m_data[i].expense));
        expArea<<QPointF(xOf(n-1),topPad+H);
        QLinearGradient grad(0,topPad,0,topPad+H);
        grad.setColorAt(0,QColor(239,68,68,60)); grad.setColorAt(1,QColor(239,68,68,5));
        p.setBrush(grad); p.setPen(Qt::NoPen); p.drawPolygon(expArea);
        // Draw income line
        QPen incPen(QColor("#10b981"),2); incPen.setJoinStyle(Qt::RoundJoin);
        p.setPen(incPen); p.setBrush(Qt::NoBrush);
        QPainterPath incPath;
        for(int i=0;i<n;i++){
            QPointF pt(xOf(i),yOf(m_data[i].income));
            i==0?incPath.moveTo(pt):incPath.lineTo(pt);
        }
        p.drawPath(incPath);
        // Income dots
        p.setBrush(QColor("#10b981")); p.setPen(Qt::NoPen);
        for(int i=0;i<n;i++) p.drawEllipse(QPointF(xOf(i),yOf(m_data[i].income)),4,4);
        // Draw expense line
        QPen expPen(QColor("#ef4444"),2); expPen.setJoinStyle(Qt::RoundJoin);
        p.setPen(expPen); p.setBrush(Qt::NoBrush);
        QPainterPath expPath;
        for(int i=0;i<n;i++){
            QPointF pt(xOf(i),yOf(m_data[i].expense));
            i==0?expPath.moveTo(pt):expPath.lineTo(pt);
        }
        p.drawPath(expPath);
        // Expense dots
        p.setBrush(QColor("#ef4444")); p.setPen(Qt::NoPen);
        for(int i=0;i<n;i++) p.drawEllipse(QPointF(xOf(i),yOf(m_data[i].expense)),4,4);
        // Month labels
        p.setPen(QColor("#94a3b8")); p.setFont(QFont("Segoe UI",8));
        QLocale loc;
        for(int i=0;i<n;i++){
            QString mo=QDate(m_data[i].year,m_data[i].month,1).toString("MMM");
            p.drawText(xOf(i)-20,topPad+H+5,40,20,Qt::AlignCenter,mo);
        }
    }
private:
    QList<MonthlyTotal> m_data;
};

// ── Dashboard class ───────────────────────────────────────────────────────────
class Dashboard : public QWidget {
    Q_OBJECT
public:
    explicit Dashboard(QWidget *parent=nullptr);
    ~Dashboard();
    void setUserInfo(const QString &name, const QString &email);
signals:
    void logoutRequested();
private slots:
    void onAddExpenseClicked();
    void onDashboardClicked();
    void onExpensesClicked();
    void onAnalyticsClicked();
    void onSettingsClicked();
    void onLogoutClicked();
private:
    void setupUI();
    void createSidebar();
    void createDashboardPage();
    void createExpensesPage();
    void createAnalyticsPage();
    void createSettingsPage();
    void refreshDashboard();
    void refreshExpenses(const QString &filter="All");
    void refreshAnalytics();
    void updateNavIcons();

    // helpers
    QWidget* makeExpenseRow(const Expense &e, bool showBadge=false);
    QWidget* makeCategoryProgressRow(const QString &name,double amount,
                                     double total,const QColor &col);
    static QList<QColor> pal();

    int     m_userId=-1;
    QString m_userEmail;
    QString m_expFilter="All";

    // Sidebar
    QWidget     *sidebarWidget;
    QPushButton *navDashboard,*navExpenses,*navAnalytics,*navSettings,*navBudget=nullptr;
    QLabel      *avatarLabel=nullptr,*userNameLabel=nullptr,*userEmailLabel=nullptr;

    // Pages
    QWidget        *contentWidget;
    QStackedWidget *pageStack;

    // Dashboard page live widgets
    QLabel       *dash_balance=nullptr,*dash_income=nullptr,*dash_expense=nullptr;
    QLabel       *budgetWarning=nullptr;
    QVBoxLayout  *dash_txList=nullptr, *dash_catList=nullptr;

    // Expenses page
    QVBoxLayout  *exp_listLayout=nullptr;
    QWidget      *exp_catCards=nullptr;  // HBox of category summary cards

    // Analytics page
    QLabel         *an_totalSpent=nullptr,*an_largestCat=nullptr,*an_avgMonthly=nullptr;
    PieChartWidget *an_pie=nullptr;
    LineChartWidget*an_line=nullptr;
    QVBoxLayout    *an_catBreakdown=nullptr;

    // Settings
    QLabel *set_base=nullptr,*set_rollover=nullptr,*set_income=nullptr,*set_total=nullptr;
};
#endif
