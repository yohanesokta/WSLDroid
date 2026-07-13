#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPoint>

class RdpClient;
class RdpScreen;
class PhoneFrame;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(const QString& host, int port, int width, int height, QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    
    
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    
    void onConnected();
    void onDisconnected();
    void onConnectionFailed(const QString& reason);
    
    
    void onToolbarVolumeUp();
    void onToolbarVolumeDown();
    void onToolbarMinimize();
    void onToolbarCamera();
    void onToolbarBack();
    void onToolbarHome();
    void onToolbarRecents();
    void onToolbarInstallApk();
    void onToolbarHelp();

private:
    
    RdpClient* m_client;
    RdpScreen* m_screen;
    PhoneFrame* m_phoneFrame;

    
    QWidget* m_toolbarPanel;

    void createToolbar();

    
    QString m_host;
    int m_port;
    int m_width;
    int m_height;

    
    bool m_dragging = false;
    QPoint m_dragPosition;

    void runAdbCommand(int keyevent);
};

#endif 
