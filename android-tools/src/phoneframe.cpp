#include "phoneframe.h"
#include <QPainter>
#include <QPaintEvent>
#include <QVBoxLayout>
#include <QPainterPath>
#include <QDebug>

PhoneFrame::PhoneFrame(QWidget* screenWidget, int rdpWidth, int rdpHeight, QWidget* parent)
    : QWidget(parent), m_screenWidget(screenWidget), m_orientation(Qt::Vertical),
      m_rdpWidth(rdpWidth), m_rdpHeight(rdpHeight) {
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_screenWidget);
    
    updateMargins();
    
    
    int totalWidth = m_rdpWidth + 2 * BezelWidth;
    int totalHeight = m_rdpHeight + 2 * BezelWidth;
    setFixedSize(totalWidth, totalHeight);
}

void PhoneFrame::setOrientation(Qt::Orientation orientation) {
    if (m_orientation != orientation) {
        m_orientation = orientation;
        updateMargins();
        
        int totalWidth = m_rdpWidth + 2 * BezelWidth;
        int totalHeight = m_rdpHeight + 2 * BezelWidth;
        
        if (m_orientation == Qt::Vertical) {
            setFixedSize(totalWidth, totalHeight);
        } else {
            setFixedSize(totalHeight, totalWidth);
        }
        update();
    }
}

void PhoneFrame::updateMargins() {
    int left = BezelWidth;
    int top = BezelWidth;
    int right = BezelWidth;
    int bottom = BezelWidth;
    
    if (m_orientation == Qt::Horizontal) {
        
        left = BezelWidth + 20;
        top = BezelWidth;
        right = BezelWidth;
        bottom = BezelWidth;
    }
    
    layout()->setContentsMargins(left, top, right, bottom);
}

void PhoneFrame::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
}

void PhoneFrame::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect r = rect();
    
    
    QColor bodyColor("#0D1117");
    QColor borderColor("#30363D");
    painter.setBrush(bodyColor);
    painter.setPen(QPen(borderColor, 2));
    
    
    painter.drawRoundedRect(r.adjusted(1, 1, -1, -1), 28, 28);
    
    
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor("#21262D"), 1));
    painter.drawRoundedRect(r.adjusted(4, 4, -4, -4), 24, 24);

    
    QRect screenBoundary = m_screenWidget->geometry();
    painter.fillRect(screenBoundary, QColor("#161B22"));
    
    
    painter.setBrush(QColor("#161B22"));
    painter.setPen(Qt::NoPen);
    
    if (m_orientation == Qt::Vertical) {
        
        
        painter.setBrush(QColor("#21262D"));
        painter.drawRoundedRect(r.width() / 2 - 30, BezelWidth / 2 - 2, 60, 4, 2, 2);
        
        
        painter.setBrush(QColor("#000000"));
        painter.drawEllipse(r.width() / 2 - 6, BezelWidth / 2 + 2, 12, 12);
        painter.setBrush(QColor("#0F172A")); 
        painter.drawEllipse(r.width() / 2 - 3, BezelWidth / 2 + 5, 6, 6);
    } else {
        
        
        painter.setBrush(QColor("#21262D"));
        painter.drawRoundedRect(BezelWidth / 2 + 6, r.height() / 2 - 30, 4, 60, 2, 2);
        
        
        painter.setBrush(QColor("#000000"));
        painter.drawEllipse(BezelWidth + 10, r.height() / 2 - 6, 12, 12);
        painter.setBrush(QColor("#0F172A"));
        painter.drawEllipse(BezelWidth + 13, r.height() / 2 - 3, 6, 6);
    }
}
