#include "mainwindow.h"
#include "rdpclient.h"
#include "rdpscreen.h"
#include "phoneframe.h"
#include "toolbutton.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QMessageBox>
#include <QDateTime>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QProcess>
#include <QFileDialog>
#include <QDir>
#include <QLabel>
#include <QPushButton>

MainWindow::MainWindow(const QString& host, int port, int width, int height, QWidget* parent)
    : QMainWindow(parent), m_host(host), m_port(port), m_width(width), m_height(height) {
    
    setWindowTitle("Custom Weston RDP AVD Emulator");
    
    
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_TranslucentBackground);
    
    
    m_client = new RdpClient(this);
    m_screen = new RdpScreen(m_client, this);
    m_phoneFrame = new PhoneFrame(m_screen, m_width, m_height, this);
    
    
    connect(m_client, &RdpClient::frameUpdated, m_screen, QOverload<>::of(&RdpScreen::update));
    connect(m_client, &RdpClient::connected, this, &MainWindow::onConnected);
    connect(m_client, &RdpClient::disconnected, this, &MainWindow::onDisconnected);
    connect(m_client, &RdpClient::connectionFailed, this, &MainWindow::onConnectionFailed);
    
    
    createToolbar();
    
    QWidget* central = new QWidget(this);
    central->setObjectName("central");
    central->setStyleSheet("QWidget#central { background: transparent; }");
    
    QFrame* controlBox = new QFrame(central);
    controlBox->setStyleSheet(
        "QFrame { background-color: rgba(22, 27, 34, 0.9); border: 1px solid rgba(48, 54, 61, 0.6); border-radius: 6px; }"
    );
    controlBox->setFixedSize(68, 30);
    // m_phoneFrame starts at x=30, y=30. Its width is m_width + 40.
    // To place controlBox fully above the device on the right:
    // x = 30 + m_width + 40 - 68 = m_width + 2
    // y = 30 - 30 = 0
    controlBox->move(m_width + 2, 0);

    QHBoxLayout* controlLayout = new QHBoxLayout(controlBox);
    controlLayout->setContentsMargins(6, 4, 6, 4);
    controlLayout->setSpacing(6);

    ToolButton* minBtn = new ToolButton(ToolButton::IconMinimize, controlBox);
    minBtn->setFixedSize(22, 22);
    minBtn->setToolTip("Minimize");
    connect(minBtn, &ToolButton::clicked, this, &MainWindow::onToolbarMinimize);

    ToolButton* closeBtn = new ToolButton(ToolButton::IconClose, controlBox);
    closeBtn->setFixedSize(22, 22);
    closeBtn->setToolTip("Close Emulator");
    connect(closeBtn, &ToolButton::clicked, this, &MainWindow::close);

    controlLayout->addWidget(minBtn);
    controlLayout->addWidget(closeBtn);
    central->setObjectName("central");
    central->setStyleSheet("QWidget#central { background: transparent; }");
    
    QHBoxLayout* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(30, 30, 30, 30); 
    mainLayout->setSpacing(20);
    
    
    mainLayout->addStretch(1);
    mainLayout->addWidget(m_phoneFrame, 0, Qt::AlignCenter);
    mainLayout->addWidget(m_toolbarPanel, 0, Qt::AlignVCenter);
    mainLayout->addStretch(1);
    
    setCentralWidget(central);
    
    
    int windowWidth = m_width + 180; 
    int windowHeight = m_height + 2 * 30 + 40; 
    resize(windowWidth, windowHeight);

    
    m_client->setConnectionInfo(m_host, m_port, "", "", m_width, m_height);
    qDebug() << "RDP: Auto-connecting to" << m_host << ":" << m_port << "with resolution" << m_width << "x" << m_height;
    m_client->connectToServer();
}

MainWindow::~MainWindow() {
    delete m_client;
}

void MainWindow::createToolbar() {
    m_toolbarPanel = new QWidget(this);
    
    m_toolbarPanel->setStyleSheet(
        "QWidget { background-color: rgba(22, 27, 34, 0.85); border: 1px solid rgba(48, 54, 61, 0.6); border-radius: 20px; }"
    );
    
    
    m_toolbarPanel->setGraphicsEffect(nullptr); 
    
    QVBoxLayout* layout = new QVBoxLayout(m_toolbarPanel);
    layout->setContentsMargins(6, 12, 6, 12);
    layout->setSpacing(8);
    
    
    auto addBtn = [this, layout](ToolButton::IconType type, const char* slotName, const QString& tooltip) {
        ToolButton* btn = new ToolButton(type, m_toolbarPanel);
        btn->setToolTip(tooltip);
        connect(btn, SIGNAL(clicked()), this, slotName);
        layout->addWidget(btn);
        return btn;
    };
    
    addBtn(ToolButton::IconVolumeUp, SLOT(onToolbarVolumeUp()), "Volume Up");
    addBtn(ToolButton::IconVolumeDown, SLOT(onToolbarVolumeDown()), "Volume Down");
    addBtn(ToolButton::IconCamera, SLOT(onToolbarCamera()), "Screenshot");
    
    
    QFrame* divider = new QFrame(m_toolbarPanel);
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("background-color: rgba(48, 54, 61, 0.6); max-height: 1px; margin: 4px 6px; border: none;");
    layout->addWidget(divider);
    
    addBtn(ToolButton::IconBack, SLOT(onToolbarBack()), "Back Button");
    addBtn(ToolButton::IconHome, SLOT(onToolbarHome()), "Home Button");
    addBtn(ToolButton::IconRecents, SLOT(onToolbarRecents()), "Recents Button");
    
    
    QFrame* divider2 = new QFrame(m_toolbarPanel);
    divider2->setFrameShape(QFrame::HLine);
    divider2->setStyleSheet("background-color: rgba(48, 54, 61, 0.6); max-height: 1px; margin: 4px 6px; border: none;");
    layout->addWidget(divider2);
    
    addBtn(ToolButton::IconInstallApk, SLOT(onToolbarInstallApk()), "Install APK");
    addBtn(ToolButton::IconHelp, SLOT(onToolbarHelp()), "About");
}



void MainWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent* event) {
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        event->accept();
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        close();
    } else {
        QMainWindow::keyPressEvent(event);
    }
}



void MainWindow::onConnected() {
    qDebug() << "RDP: Connected to remote server.";
}

void MainWindow::onDisconnected() {
    qDebug() << "RDP: Disconnected from remote server.";
    m_screen->update(); 
}

void MainWindow::onConnectionFailed(const QString& reason) {
    qWarning() << "RDP Connection Failed:" << reason;
    m_screen->update();
}



void MainWindow::runAdbCommand(int keyevent) {
    qDebug() << "Executing ADB keyevent:" << keyevent;
    QProcess::startDetached("adb", QStringList() << "shell" << "input" << "keyevent" << QString::number(keyevent));
}

void MainWindow::onToolbarMinimize() {
    showMinimized();
}



void MainWindow::onToolbarVolumeUp() {
    runAdbCommand(24);
}

void MainWindow::onToolbarVolumeDown() {
    runAdbCommand(25);
}

void MainWindow::onToolbarCamera() {
    qDebug() << "Toolbar: Camera / Screenshot Clicked";
    QImage img = m_client->getFrame();
    if (!img.isNull()) {
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
        QString fileName = QString("screenshot_%1.png").arg(timestamp);
        if (img.save(fileName)) {
            qDebug() << "Screenshot saved to" << fileName;
        } else {
            qWarning() << "Failed to save screenshot.";
        }
    } else {
        qWarning() << "Cannot take screenshot: No active RDP session.";
    }
}

void MainWindow::onToolbarBack() {
    runAdbCommand(4);
}

void MainWindow::onToolbarHome() {
    runAdbCommand(3);
}

void MainWindow::onToolbarRecents() {
    runAdbCommand(187);
}

void MainWindow::onToolbarInstallApk() {
    qDebug() << "Toolbar: Install APK Clicked";
    QString filePath = QFileDialog::getOpenFileName(this, "Select APK to Install", QDir::homePath(), "APK Files (*.apk)");
    if (!filePath.isEmpty()) {
        qDebug() << "Installing APK:" << filePath;
        QProcess::startDetached("adb", QStringList() << "install" << filePath);
    }
}

void MainWindow::onToolbarHelp() {
    qDebug() << "Toolbar: Help / About Window Triggered";
    
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("About AVD Emulator");
    dialog->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    dialog->setAttribute(Qt::WA_TranslucentBackground);

    QFrame* container = new QFrame(dialog);
    container->setStyleSheet(
        "QFrame { "
        "  background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1F242C, stop:1 #0D1117); "
        "  border: 2px solid #58A6FF; "
        "  border-radius: 20px; "
        "} "
        "QLabel { "
        "  color: #C9D1D9; "
        "  border: none; "
        "  font-family: 'Outfit', 'Segoe UI', Arial; "
        "} "
        "QPushButton { "
        "  background-color: #21262D; "
        "  color: #58A6FF; "
        "  border: 1px solid #30363D; "
        "  border-radius: 8px; "
        "  padding: 8px 24px; "
        "  font-family: 'Outfit'; "
        "  font-weight: bold; "
        "} "
        "QPushButton:hover { "
        "  background-color: #58A6FF; "
        "  color: #0D1117; "
        "  border-color: #58A6FF; "
        "}"
    );

    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(18);

    QLabel* title = new QLabel("AVD EMULATOR", container);
    title->setStyleSheet("font-size: 20px; font-weight: bold; color: #58A6FF; letter-spacing: 1px;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    QLabel* desc = new QLabel("High-Performance Weston RDP Controller", container);
    desc->setStyleSheet("font-size: 12px; color: #8A8D90; font-weight: 500;");
    desc->setAlignment(Qt::AlignCenter);
    layout->addWidget(desc);

    QLabel* copyright = new QLabel("© 2026 Yohanes Oktanio.\nAll rights reserved.", container);
    copyright->setStyleSheet("font-size: 11px; color: #8A8D90; line-height: 1.4;");
    copyright->setAlignment(Qt::AlignCenter);
    layout->addWidget(copyright);

    QLabel* link = new QLabel(container);
    link->setText("<a href=\"https://github.com/yohanesokta\" style=\"color: #58A6FF; text-decoration: none; font-weight: bold; font-size: 14px;\">github.com/yohanesokta</a>");
    link->setTextFormat(Qt::RichText);
    link->setOpenExternalLinks(true);
    link->setAlignment(Qt::AlignCenter);
    layout->addWidget(link);

    QPushButton* closeBtn = new QPushButton("Close", container);
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);
    layout->addWidget(closeBtn, 0, Qt::AlignCenter);

    QVBoxLayout* dialogLayout = new QVBoxLayout(dialog);
    dialogLayout->setContentsMargins(10, 10, 10, 10);
    dialogLayout->addWidget(container);

    dialog->resize(320, 260);
    dialog->exec();
    dialog->deleteLater();
}
