#include "rdpscreen.h"
#include "rdpclient.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QDebug>
#include <algorithm>


#define INPUT_PTR_FLAGS_MOVE 0x0800
#define INPUT_PTR_FLAGS_DOWN 0x8000
#define INPUT_PTR_FLAGS_BUTTON1 0x1000 
#define INPUT_PTR_FLAGS_BUTTON2 0x2000 
#define INPUT_PTR_FLAGS_BUTTON3 0x4000 
#define INPUT_PTR_FLAGS_WHEEL 0x0200
#define INPUT_PTR_FLAGS_WHEEL_NEGATIVE 0x0100
#define INPUT_WheelRotationMask 0x01FF

#define INPUT_KBD_FLAGS_EXTENDED 0x0100
#define INPUT_KBD_FLAGS_RELEASE 0x8000


static uint8_t getRdpScancode(int key, bool& extended) {
    extended = false;
    switch (key) {
        case Qt::Key_Escape: return 0x01;
        case Qt::Key_1: return 0x02;
        case Qt::Key_2: return 0x03;
        case Qt::Key_3: return 0x04;
        case Qt::Key_4: return 0x05;
        case Qt::Key_5: return 0x06;
        case Qt::Key_6: return 0x07;
        case Qt::Key_7: return 0x08;
        case Qt::Key_8: return 0x09;
        case Qt::Key_9: return 0x0A;
        case Qt::Key_0: return 0x0B;
        case Qt::Key_Minus: return 0x0C;
        case Qt::Key_Equal: return 0x0D;
        case Qt::Key_Backspace: return 0x0E;
        case Qt::Key_Tab: return 0x0F;
        case Qt::Key_Q: return 0x10;
        case Qt::Key_W: return 0x11;
        case Qt::Key_E: return 0x12;
        case Qt::Key_R: return 0x13;
        case Qt::Key_T: return 0x14;
        case Qt::Key_Y: return 0x15;
        case Qt::Key_U: return 0x16;
        case Qt::Key_I: return 0x17;
        case Qt::Key_O: return 0x18;
        case Qt::Key_P: return 0x19;
        case Qt::Key_BracketLeft: return 0x1A;
        case Qt::Key_BracketRight: return 0x1B;
        case Qt::Key_Return: return 0x1C;
        case Qt::Key_Control: return 0x1D; 
        case Qt::Key_A: return 0x1E;
        case Qt::Key_S: return 0x1F;
        case Qt::Key_D: return 0x20;
        case Qt::Key_F: return 0x21;
        case Qt::Key_G: return 0x22;
        case Qt::Key_H: return 0x23;
        case Qt::Key_J: return 0x24;
        case Qt::Key_K: return 0x25;
        case Qt::Key_L: return 0x26;
        case Qt::Key_Semicolon: return 0x27;
        case Qt::Key_Apostrophe: return 0x28;
        case Qt::Key_QuoteLeft: return 0x29;
        case Qt::Key_Shift: return 0x2A; 
        case Qt::Key_Backslash: return 0x2B;
        case Qt::Key_Z: return 0x2C;
        case Qt::Key_X: return 0x2D;
        case Qt::Key_C: return 0x2E;
        case Qt::Key_V: return 0x2F;
        case Qt::Key_B: return 0x30;
        case Qt::Key_N: return 0x31;
        case Qt::Key_M: return 0x32;
        case Qt::Key_Comma: return 0x33;
        case Qt::Key_Period: return 0x34;
        case Qt::Key_Slash: return 0x35;
        case Qt::Key_Alt: return 0x38; 
        case Qt::Key_Space: return 0x39;
        case Qt::Key_CapsLock: return 0x3A;
        case Qt::Key_F1: return 0x3B;
        case Qt::Key_F2: return 0x3C;
        case Qt::Key_F3: return 0x3D;
        case Qt::Key_F4: return 0x3E;
        case Qt::Key_F5: return 0x3F;
        case Qt::Key_F6: return 0x40;
        case Qt::Key_F7: return 0x41;
        case Qt::Key_F8: return 0x42;
        case Qt::Key_F9: return 0x43;
        case Qt::Key_F10: return 0x44;
        case Qt::Key_NumLock: return 0x45;
        case Qt::Key_ScrollLock: return 0x46;
        case Qt::Key_F11: return 0x57;
        case Qt::Key_F12: return 0x58;
        
        
        case Qt::Key_Left: extended = true; return 0x4B;
        case Qt::Key_Up: extended = true; return 0x48;
        case Qt::Key_Right: extended = true; return 0x4D;
        case Qt::Key_Down: extended = true; return 0x50;
        case Qt::Key_Home: extended = true; return 0x47;
        case Qt::Key_End: extended = true; return 0x4F;
        case Qt::Key_PageUp: extended = true; return 0x49;
        case Qt::Key_PageDown: extended = true; return 0x51;
        case Qt::Key_Insert: extended = true; return 0x52;
        case Qt::Key_Delete: extended = true; return 0x53;
        case Qt::Key_Meta: extended = true; return 0x5B; 
        default: return 0;
    }
}

RdpScreen::RdpScreen(RdpClient* client, QWidget* parent)
    : QWidget(parent), m_client(client), m_scalingMode(FitAspectRatio) {
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    
    
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void RdpScreen::setScalingMode(ScalingMode mode) {
    if (m_scalingMode != mode) {
        m_scalingMode = mode;
        update();
    }
}

void RdpScreen::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    
    
    if (!m_client || !m_client->isConnected()) {
        painter.fillRect(rect(), QColor("#121214"));
        painter.setPen(QColor("#5F6368"));
        painter.setFont(QFont("Outfit", 12));
        painter.drawText(rect(), Qt::AlignCenter, "Not Connected\n\nEnter connection details and click Connect");
        return;
    }

    QImage frame = m_client->getFrame();
    if (frame.isNull()) {
        painter.fillRect(rect(), QColor("#121214"));
        painter.setPen(QColor("#80868B"));
        painter.drawText(rect(), Qt::AlignCenter, "Connected, waiting for first frame...");
        return;
    }

    if (m_scalingMode == Stretch) {
        painter.drawImage(rect(), frame);
    } else {
        
        painter.fillRect(rect(), Qt::black); 

        QSize imgSize = frame.size();
        QSize widgetSize = size();
        double imgRatio = (double)imgSize.width() / imgSize.height();
        double widgetRatio = (double)widgetSize.width() / widgetSize.height();

        int drawW, drawH, drawX, drawY;
        if (widgetRatio > imgRatio) {
            drawH = widgetSize.height();
            drawW = qRound(drawH * imgRatio);
            drawX = (widgetSize.width() - drawW) / 2;
            drawY = 0;
        } else {
            drawW = widgetSize.width();
            drawH = qRound(drawW / imgRatio);
            drawX = 0;
            drawY = (widgetSize.height() - drawH) / 2;
        }

        painter.drawImage(QRect(drawX, drawY, drawW, drawH), frame);
    }
}

bool RdpScreen::mapToRdpCoordinates(const QPoint& localPos, int& rdpX, int& rdpY) const {
    if (!m_client || !m_client->isConnected()) return false;

    QImage frame = m_client->getFrame();
    if (frame.isNull()) return false;

    QSize imgSize = frame.size();
    QSize widgetSize = size();

    if (m_scalingMode == Stretch) {
        rdpX = localPos.x() * imgSize.width() / widgetSize.width();
        rdpY = localPos.y() * imgSize.height() / widgetSize.height();
    } else {
        
        double imgRatio = (double)imgSize.width() / imgSize.height();
        double widgetRatio = (double)widgetSize.width() / widgetSize.height();

        int drawW, drawH, drawX, drawY;
        if (widgetRatio > imgRatio) {
            drawH = widgetSize.height();
            drawW = qRound(drawH * imgRatio);
            drawX = (widgetSize.width() - drawW) / 2;
            drawY = 0;
        } else {
            drawW = widgetSize.width();
            drawH = qRound(drawW / imgRatio);
            drawX = 0;
            drawY = (widgetSize.height() - drawH) / 2;
        }

        
        int lx = localPos.x() - drawX;
        int ly = localPos.y() - drawY;
        
        rdpX = lx * imgSize.width() / drawW;
        rdpY = ly * imgSize.height() / drawH;
    }

    
    rdpX = std::max(0, std::min(rdpX, imgSize.width() - 1));
    rdpY = std::max(0, std::min(rdpY, imgSize.height() - 1));

    return true;
}

void RdpScreen::sendMouseEvent(QMouseEvent* event, bool isDown) {
    int rx, ry;
    if (!mapToRdpCoordinates(event->pos(), rx, ry)) return;

    uint16_t flags = INPUT_PTR_FLAGS_MOVE;
    
    if (event->button() == Qt::LeftButton) {
        flags |= INPUT_PTR_FLAGS_BUTTON1;
    } else if (event->button() == Qt::RightButton) {
        flags |= INPUT_PTR_FLAGS_BUTTON2;
    } else if (event->button() == Qt::MiddleButton) {
        flags |= INPUT_PTR_FLAGS_BUTTON3;
    } else {
        
        m_client->sendMouseEvent(flags, rx, ry);
        return;
    }

    if (isDown) {
        flags |= INPUT_PTR_FLAGS_DOWN;
    }

    m_client->sendMouseEvent(flags, rx, ry);
}

void RdpScreen::mousePressEvent(QMouseEvent* event) {
    sendMouseEvent(event, true);
    setFocus();
}

void RdpScreen::mouseReleaseEvent(QMouseEvent* event) {
    sendMouseEvent(event, false);
}

void RdpScreen::mouseMoveEvent(QMouseEvent* event) {
    int rx, ry;
    if (mapToRdpCoordinates(event->pos(), rx, ry)) {
        uint16_t flags = INPUT_PTR_FLAGS_MOVE;
        
        if (event->buttons() & Qt::LeftButton) {
            flags |= INPUT_PTR_FLAGS_BUTTON1 | INPUT_PTR_FLAGS_DOWN;
        }
        if (event->buttons() & Qt::RightButton) {
            flags |= INPUT_PTR_FLAGS_BUTTON2 | INPUT_PTR_FLAGS_DOWN;
        }
        if (event->buttons() & Qt::MiddleButton) {
            flags |= INPUT_PTR_FLAGS_BUTTON3 | INPUT_PTR_FLAGS_DOWN;
        }
        m_client->sendMouseEvent(flags, rx, ry);
    }
}

void RdpScreen::wheelEvent(QWheelEvent* event) {
    int rx, ry;
    if (mapToRdpCoordinates(event->position().toPoint(), rx, ry)) {
        int delta = event->angleDelta().y();
        uint16_t flags = INPUT_PTR_FLAGS_WHEEL;
        
        if (delta < 0) {
            flags |= INPUT_PTR_FLAGS_WHEEL_NEGATIVE;
            delta = -delta;
        }
        
        
        flags |= (0x0078 & INPUT_WheelRotationMask);
        m_client->sendMouseEvent(flags, rx, ry);
    }
}

void RdpScreen::keyPressEvent(QKeyEvent* event) {
    if (!m_client || !m_client->isConnected()) return;

    bool extended = false;
    uint8_t scancode = getRdpScancode(event->key(), extended);

    if (scancode != 0) {
        uint16_t flags = 0;
        if (extended) {
            flags |= INPUT_KBD_FLAGS_EXTENDED;
        }
        m_client->sendKeyboardEvent(flags, scancode);
    } else {
        
        QString text = event->text();
        if (!text.isEmpty()) {
            uint16_t unicode = text.at(0).unicode();
            m_client->sendUnicodeKeyboardEvent(0, unicode);
        }
    }
    event->accept();
}

void RdpScreen::keyReleaseEvent(QKeyEvent* event) {
    if (!m_client || !m_client->isConnected()) return;

    bool extended = false;
    uint8_t scancode = getRdpScancode(event->key(), extended);

    if (scancode != 0) {
        uint16_t flags = INPUT_KBD_FLAGS_RELEASE;
        if (extended) {
            flags |= INPUT_KBD_FLAGS_EXTENDED;
        }
        m_client->sendKeyboardEvent(flags, scancode);
    } else {
        QString text = event->text();
        if (!text.isEmpty()) {
            uint16_t unicode = text.at(0).unicode();
            m_client->sendUnicodeKeyboardEvent(INPUT_KBD_FLAGS_RELEASE, unicode);
        }
    }
    event->accept();
}
