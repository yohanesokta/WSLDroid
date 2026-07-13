#include <QApplication>
#include "mainwindow.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

int main(int argc, char *argv[]) {
#ifdef _WIN32
    WSADATA wsaData;
    int wsaRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaRet != 0) {
        qWarning("WSAStartup failed: %d", wsaRet);
    }
#endif

    QApplication app(argc, argv);
    
    
    app.setApplicationName("Custom Weston RDP AVD Emulator");
    app.setOrganizationName("CustomRdpEmulator");
    
    
    QString host = "127.0.0.1";
    int port = 3389;
    int width = 393;
    int height = 852;

    
    QStringList args = app.arguments();
    for (int i = 1; i < args.size(); ++i) {
        QString arg = args.at(i);
        if (arg.startsWith("--host=")) {
            host = arg.mid(7);
        } else if (arg.startsWith("--port=")) {
            bool ok = false;
            int parsedPort = arg.mid(7).toInt(&ok);
            if (ok) port = parsedPort;
        } else if (arg.startsWith("--width=")) {
            bool ok = false;
            int parsedWidth = arg.mid(8).toInt(&ok);
            if (ok) width = parsedWidth;
        } else if (arg.startsWith("--height=")) {
            bool ok = false;
            int parsedHeight = arg.mid(9).toInt(&ok);
            if (ok) height = parsedHeight;
        }
    }
    
    MainWindow w(host, port, width, height);
    w.show();
    
    int result = app.exec();

#ifdef _WIN32
    if (wsaRet == 0) {
        WSACleanup();
    }
#endif

    return result;
}
