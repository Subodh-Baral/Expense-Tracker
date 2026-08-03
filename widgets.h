#ifndef WIDGETS_H
#define WIDGETS_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QDir>
#include <QCoreApplication>
#include <QLinearGradient>
#include <QRadialGradient>

// ── Rounded card widget ──────────────────────────────────────────────────────
class CardWidget : public QWidget {
public:
    explicit CardWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setAttribute(Qt::WA_TranslucentBackground);
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addRoundedRect(rect(), 16, 16);
        p.fillPath(path, QColor(38, 45, 55, 180));
    }
};

// ── Background widget ────────────────────────────────────────────────────────
class BackgroundWidget : public QWidget {
public:
    explicit BackgroundWidget(QWidget* parent = nullptr) : QWidget(parent) {
        // Look next to the executable, NOT the current working directory.
        // QDir::currentPath() depends on where the app was launched FROM,
        // so it silently failed to find these images unless you happened to
        // launch from the exact right folder. applicationDirPath() is
        // always the same regardless of how you start the app.
        QString dir = QCoreApplication::applicationDirPath();
        leftPixmap.load(dir + "/bg_right.jpg");
        rightPixmap.load(dir + "/bg_left.jpg");
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        p.setRenderHint(QPainter::Antialiasing);

        int W    = width();
        int H    = height();
        int half = W / 2;

        // ── Base dark background ──
        QLinearGradient base(0, 0, W, H);
        base.setColorAt(0.0, QColor(16, 21, 29));
        base.setColorAt(1.0, QColor(21, 27, 37));
        p.fillRect(rect(), base);

        bool haveImages = !leftPixmap.isNull() || !rightPixmap.isNull();

        if (haveImages) {
            // ── Left half image at 30% opacity ──
            if (!leftPixmap.isNull()) {
                QRect leftRect(0, 0, half, H);
                QPixmap scaled = leftPixmap.scaled(half, H,
                                                   Qt::KeepAspectRatioByExpanding,
                                                   Qt::SmoothTransformation);
                int ox = (half - scaled.width())  / 2;
                int oy = (H    - scaled.height()) / 2;
                p.setClipRect(leftRect);
                p.setOpacity(0.30);
                p.drawPixmap(ox, oy, scaled);
                p.setOpacity(1.0);
                p.setClipping(false);
            }
            // ── Right half image at 30% opacity ──
            if (!rightPixmap.isNull()) {
                QRect rightRect(half, 0, W - half, H);
                QPixmap scaled = rightPixmap.scaled(W - half, H,
                                                    Qt::KeepAspectRatioByExpanding,
                                                    Qt::SmoothTransformation);
                int ox = half + (W - half - scaled.width())  / 2;
                int oy =        (H        - scaled.height()) / 2;
                p.setClipRect(rightRect);
                p.setOpacity(0.30);
                p.drawPixmap(ox, oy, scaled);
                p.setOpacity(1.0);
                p.setClipping(false);
            }

            // ── Edge fades so the images blend into the base ──
            QLinearGradient bottomFade(0, H, 0, H * 0.4);
            bottomFade.setColorAt(0.0, QColor(10, 14, 20, 220));
            bottomFade.setColorAt(1.0, QColor(10, 14, 20,   0));
            p.fillRect(rect(), bottomFade);

            QLinearGradient topFade(0, 0, 0, H * 0.3);
            topFade.setColorAt(0.0, QColor(10, 14, 20, 180));
            topFade.setColorAt(1.0, QColor(10, 14, 20,   0));
            p.fillRect(rect(), topFade);

            QLinearGradient leftFade(0, 0, W * 0.18, 0);
            leftFade.setColorAt(0.0, QColor(10, 14, 20, 255));
            leftFade.setColorAt(1.0, QColor(10, 14, 20,   0));
            p.fillRect(rect(), leftFade);

            QLinearGradient rightFade(W, 0, W * 0.82, 0);
            rightFade.setColorAt(0.0, QColor(10, 14, 20, 255));
            rightFade.setColorAt(1.0, QColor(10, 14, 20,   0));
            p.fillRect(rect(), rightFade);
        } else {
            // ── No background images found: generate a premium look on the
            //    fly using soft brand-color glows, so the screen is never
            //    left looking flat/empty regardless of what files exist. ──
            QRadialGradient glowGreen(QPointF(W * 0.12, H * 0.15), W * 0.55);
            glowGreen.setColorAt(0.0, QColor(16, 185, 129, 60));
            glowGreen.setColorAt(1.0, QColor(16, 185, 129, 0));
            p.fillRect(rect(), glowGreen);

            QRadialGradient glowIndigo(QPointF(W * 0.9, H * 0.85), W * 0.6);
            glowIndigo.setColorAt(0.0, QColor(99, 102, 241, 55));
            glowIndigo.setColorAt(1.0, QColor(99, 102, 241, 0));
            p.fillRect(rect(), glowIndigo);

            QRadialGradient glowBlue(QPointF(W * 0.88, H * 0.08), W * 0.4);
            glowBlue.setColorAt(0.0, QColor(56, 189, 248, 35));
            glowBlue.setColorAt(1.0, QColor(56, 189, 248, 0));
            p.fillRect(rect(), glowBlue);

            // Subtle dot-grid texture for depth
            p.setPen(QPen(QColor(255, 255, 255, 12), 1));
            for (int gx = 0; gx < W; gx += 32)
                for (int gy = 0; gy < H; gy += 32)
                    p.drawPoint(gx, gy);
        }

        // ── Centre radial darkening so the card always reads clearly ──
        QRadialGradient vig(QPointF(W * 0.5, H * 0.5), W * 0.55);
        vig.setColorAt(0.0, QColor(10, 14, 20,  40));
        vig.setColorAt(1.0, QColor(10, 14, 20, 130));
        p.fillRect(rect(), vig);
    }

private:
    QPixmap leftPixmap;
    QPixmap rightPixmap;
};

// ── App icon widget ──────────────────────────────────────────────────────────
class AppIconWidget : public QWidget {
public:
    explicit AppIconWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setFixedSize(68, 68);
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        QLinearGradient grad(0, 0, 68, 68);
        grad.setColorAt(0, QColor(0, 210, 150));
        grad.setColorAt(1, QColor(0, 180, 100));
        QPainterPath path;
        path.addRoundedRect(rect(), 16, 16);
        p.fillPath(path, grad);

        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 255, 255, 55));
        p.drawEllipse(QPoint(34, 34), 18, 18);

        p.setPen(QPen(Qt::white, 2));
        p.setFont(QFont("Arial", 17, QFont::Bold));
        p.drawText(rect(), Qt::AlignCenter, QString::fromUtf8("Rs"));
    }
};

// ── Field builder helper ─────────────────────────────────────────────────────
static QLineEdit* makeField(const QString& placeholder,
                            bool password = false,
                            QWidget* parent = nullptr)
{
    auto* field = new QLineEdit(parent);
    field->setPlaceholderText(placeholder);
    field->setFixedHeight(46);
    if (password) {
        field->setEchoMode(QLineEdit::Password);
    }
    field->setStyleSheet(R"(
        QLineEdit {
            background:rgba(255,255,255,0.07);
            border:1.5px solid rgba(255,255,255,0.15);
            border-radius:10px; color:#C8D6E5;
            font-size:14px; font-family:'Segoe UI',sans-serif;
            padding:0 44px 0 14px;
        }
        QLineEdit:focus {
            border-color:#00D296;
            background:rgba(255,255,255,0.10);
        }
    )");
    return field;
}

static QLabel* makeLabel(const QString& text) {
    auto* lbl = new QLabel(text);
    lbl->setStyleSheet("color:#C8D6E5;font-size:13px;font-weight:600;"
                       "font-family:'Segoe UI',sans-serif;");
    return lbl;
}

#endif // WIDGETS_H
