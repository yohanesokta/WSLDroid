#include "toolbutton.h"
#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>

ToolButton::ToolButton(IconType type, QWidget* parent)
    : QAbstractButton(parent), m_type(type), m_hovered(false) {
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_Hover);
}

QSize ToolButton::sizeHint() const {
    return QSize(40, 40);
}

QSize ToolButton::minimumSizeHint() const {
    return QSize(32, 32);
}

void ToolButton::enterEvent(QEnterEvent* event) {
    m_hovered = true;
    update();
    QAbstractButton::enterEvent(event);
}

void ToolButton::leaveEvent(QEvent* event) {
    m_hovered = false;
    update();
    QAbstractButton::leaveEvent(event);
}

void ToolButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    
    QColor bgColor;
    if (m_hovered) {
        bgColor = QColor(255, 255, 255, 30); 
    } else {
        bgColor = Qt::transparent;
    }
    painter.setBrush(bgColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect().adjusted(2, 2, -2, -2), 6, 6);

    
    QColor iconColor = m_hovered ? QColor("#E3E3E3") : QColor("#8A8D90");
    if (!isEnabled()) {
        iconColor = QColor("#4A4B4D");
    }

    QPen pen(iconColor);
    pen.setWidth(2);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    int w = width();
    int h = height();
    int cx = w / 2;
    int cy = h / 2;

    switch (m_type) {
        case IconPower: {
            
            painter.drawArc(cx - 7, cy - 7, 14, 14, 45 * 16, 270 * 16);
            painter.drawLine(cx, cy - 10, cx, cy - 2);
            break;
        }
        case IconVolumeUp: {
            
            QPainterPath path;
            path.moveTo(cx - 7, cy - 3);
            path.lineTo(cx - 4, cy - 3);
            path.lineTo(cx - 1, cy - 6);
            path.lineTo(cx - 1, cy + 6);
            path.lineTo(cx - 4, cy + 3);
            path.lineTo(cx - 7, cy + 3);
            path.closeSubpath();
            painter.drawPath(path);
            
            
            painter.drawArc(cx - 3, cy - 6, 10, 12, -45 * 16, 90 * 16);
            painter.drawArc(cx - 5, cy - 9, 16, 18, -45 * 16, 90 * 16);
            break;
        }
        case IconVolumeDown: {
            
            QPainterPath path;
            path.moveTo(cx - 7, cy - 3);
            path.lineTo(cx - 4, cy - 3);
            path.lineTo(cx - 1, cy - 6);
            path.lineTo(cx - 1, cy + 6);
            path.lineTo(cx - 4, cy + 3);
            path.lineTo(cx - 7, cy + 3);
            path.closeSubpath();
            painter.drawPath(path);
            
            
            painter.drawArc(cx - 3, cy - 6, 10, 12, -45 * 16, 90 * 16);
            break;
        }

        case IconCamera: {
            
            painter.drawRoundedRect(cx - 8, cy - 5, 16, 10, 2, 2);
            painter.drawEllipse(cx - 3, cy - 2, 6, 6);
            
            painter.drawLine(cx - 5, cy - 5, cx - 3, cy - 5);
            break;
        }

        case IconBack: {
            
            painter.drawLine(cx - 5, cy, cx + 5, cy);
            painter.drawLine(cx - 5, cy, cx, cy - 5);
            painter.drawLine(cx - 5, cy, cx, cy + 5);
            break;
        }
        case IconHome: {
            
            QPainterPath path;
            path.moveTo(cx - 7, cy + 5);
            path.lineTo(cx - 7, cy - 2);
            path.lineTo(cx, cy - 8);
            path.lineTo(cx + 7, cy - 2);
            path.lineTo(cx + 7, cy + 5);
            path.closeSubpath();
            painter.drawPath(path);
            break;
        }
        case IconRecents: {
            
            painter.drawRoundedRect(cx - 6, cy - 6, 12, 12, 1, 1);
            break;
        }
        case IconHelp: {
            painter.drawEllipse(cx - 8, cy - 8, 16, 16);
            painter.setBrush(iconColor);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(cx - 1, cy + 3, 2, 2);
            QPen thinPen(iconColor);
            thinPen.setWidthF(1.5);
            painter.setPen(thinPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawArc(cx - 4, cy - 6, 8, 6, -30 * 16, 240 * 16);
            QPainterPath qPath;
            qPath.moveTo(cx + 2, cy - 3);
            qPath.quadTo(cx + 2, cy - 0.5, cx, cy + 1);
            painter.drawPath(qPath);
            break;
        }
        case IconMinimize: {
            painter.drawLine(cx - 5, cy + 4, cx + 5, cy + 4);
            break;
        }
        case IconClose: {
            painter.drawLine(cx - 5, cy - 5, cx + 5, cy + 5);
            painter.drawLine(cx - 5, cy + 5, cx + 5, cy - 5);
            break;
        }
        case IconInstallApk: {
            painter.drawPolyline(QPolygon({QPoint(cx - 6, cy + 2), QPoint(cx - 6, cy + 6), QPoint(cx + 6, cy + 6), QPoint(cx + 6, cy + 2)}));
            painter.drawLine(cx, cy - 6, cx, cy + 2);
            painter.drawLine(cx - 3, cy - 1, cx, cy + 2);
            painter.drawLine(cx + 3, cy - 1, cx, cy + 2);
            break;
        }
    }
}
