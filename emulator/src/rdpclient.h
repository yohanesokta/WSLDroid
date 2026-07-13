#ifndef RDPCLIENT_H
#define RDPCLIENT_H

#include <QObject>
#include <QImage>
#include <QMutex>
#include <QThread>
#include <atomic>
#include <string>


typedef struct rdp_freerdp freerdp;
typedef struct rdp_context rdpContext;

class RdpClient;


class RdpWorker : public QThread {
    Q_OBJECT
public:
    explicit RdpWorker(RdpClient* client);
    ~RdpWorker() override;

    void stop();


protected:
    void run() override;

private:
    RdpClient* m_client;
    std::atomic<bool> m_stop;
};


class RdpClient : public QObject {
    Q_OBJECT
    friend class RdpWorker;
public:
    explicit RdpClient(QObject* parent = nullptr);
    ~RdpClient() override;

    
    void setConnectionInfo(const QString& host, int port, const QString& username, const QString& password, int width = 393, int height = 852);

    
    void connectToServer();
    void disconnectFromServer();

    
    void sendMouseEvent(uint16_t flags, uint16_t x, uint16_t y);
    void sendKeyboardEvent(uint16_t flags, uint8_t scancode);
    void sendUnicodeKeyboardEvent(uint16_t flags, uint16_t code);

    
    QImage getFrame();
    bool isConnected() const;

    
    freerdp* instance() const { return m_instance; }
    void onEndPaint();
    void onDesktopResize(int width, int height);

signals:
    void connected();
    void disconnected();
    void connectionFailed(const QString& reason);
    void frameUpdated();
    void desktopResized(int width, int height);

private:
    
    freerdp* m_instance;
    RdpWorker* m_worker;

    
    QString m_host;
    int m_port;
    QString m_username;
    QString m_password;
    int m_width;
    int m_height;

    
    mutable QMutex m_stateMutex;
    bool m_connected;

    
    QMutex m_frameMutex;
    QImage m_frameImage;

    
    bool initializeFreeRDP();
    void cleanupFreeRDP();
};

#endif 
