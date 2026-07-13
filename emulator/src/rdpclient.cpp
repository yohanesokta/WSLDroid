#include "rdpclient.h"
#include <winpr/environment.h>

#include <freerdp/freerdp.h>
#include <freerdp/gdi/gdi.h>
#include <freerdp/client.h>
#include <freerdp/channels/channels.h>
#include <freerdp/settings.h>
#include <winpr/synch.h>
#include <winpr/error.h>
#include <QDebug>
#include <QMutexLocker>

#ifndef PERF_DISABLE_WALLPAPER
#define PERF_DISABLE_WALLPAPER 0x00000001
#endif
#ifndef PERF_DISABLE_FULLWINDOWDRAG
#define PERF_DISABLE_FULLWINDOWDRAG 0x00000002
#endif
#ifndef PERF_DISABLE_MENUANIMATIONS
#define PERF_DISABLE_MENUANIMATIONS 0x00000004
#endif
#ifndef PERF_DISABLE_THEMES
#define PERF_DISABLE_THEMES 0x00000008
#endif
#ifndef PERF_DISABLE_CURSOR_SHADOW
#define PERF_DISABLE_CURSOR_SHADOW 0x00000020
#endif
#ifndef PERF_DISABLE_FONT_SMOOTHING
#define PERF_DISABLE_FONT_SMOOTHING 0x00000080
#endif

struct MyRdpContext {
    rdpClientContext context;
    RdpClient* client;
};

static char* allocate_string(const char* src) {
    if (!src) src = "";
    size_t len = strlen(src);
    char* dst = (char*)malloc(len + 1);
    if (dst) {
        strcpy(dst, src);
    }
    return dst;
}



RdpWorker::RdpWorker(RdpClient* client)
    : m_client(client), m_stop(false) {}

RdpWorker::~RdpWorker() {
    stop();
}

void RdpWorker::stop() {
    m_stop = true;
    wait();
}

void RdpWorker::run() {
    freerdp* instance = m_client->instance();
    if (!instance) {
        emit m_client->connectionFailed("FreeRDP instance not initialized.");
        return;
    }

    qDebug() << "Connecting to RDP server...";
    if (!freerdp_connect(instance)) {
        emit m_client->connectionFailed("Failed to connect to RDP server. Please check host, port, or if Weston RDP is running.");
        return;
    }

    {
        QMutexLocker stateLocker(&m_client->m_stateMutex);
        m_client->m_connected = true;
    }
    emit m_client->connected();

    qDebug() << "RDP connected. Running event loop...";

    rdpContext* context = instance->context;
    while (!m_stop && !freerdp_shall_disconnect_context(context)) {
        HANDLE handles[64];
        DWORD nCount = freerdp_get_event_handles(context, handles, 60);
        
        
        HANDLE channelEvent = freerdp_channels_get_event_handle(instance);
        if (channelEvent) {
            handles[nCount++] = channelEvent;
        }
        
        
        DWORD status = WaitForMultipleObjects(nCount, handles, FALSE, 100);
        Q_UNUSED(status);
        
        
        if (!freerdp_check_event_handles(context)) {
            if (freerdp_get_last_error(context) != FREERDP_ERROR_SUCCESS) {
                qWarning() << "FreeRDP check event handles failed, error code:" << freerdp_get_last_error(context);
                break;
            }
        }

        
        if (context->channels) {
            if (!freerdp_channels_check_fds(context->channels, instance)) {
                qWarning() << "FreeRDP channels check fds failed";
                break;
            }
        }
    }

    qDebug() << "Disconnecting RDP...";
    freerdp_disconnect(instance);

    {
        QMutexLocker stateLocker(&m_client->m_stateMutex);
        m_client->m_connected = false;
    }
    emit m_client->disconnected();
}



RdpClient::RdpClient(QObject* parent)
    : QObject(parent), m_instance(nullptr), m_worker(nullptr), m_port(3389), m_connected(false) {}

RdpClient::~RdpClient() {
    disconnectFromServer();
    cleanupFreeRDP();
}

void RdpClient::setConnectionInfo(const QString& host, int port, const QString& username, const QString& password, int width, int height) {
    m_host = host;
    m_port = port;
    m_username = username;
    m_password = password;
    m_width = width;
    m_height = height;
}

void RdpClient::connectToServer() {
    if (m_worker && m_worker->isRunning()) {
        return;
    }



    cleanupFreeRDP();

    
    const char* proxy_vars[] = {
        "http_proxy", "HTTP_PROXY",
        "https_proxy", "HTTPS_PROXY",
        "all_proxy", "ALL_PROXY",
        "socks_proxy", "SOCKS_PROXY",
        "socks5_proxy", "SOCKS5_PROXY",
        "socks_server", "SOCKS_SERVER",
        "no_proxy", "NO_PROXY"
    };
    for (const char* var : proxy_vars) {
        SetEnvironmentVariableA(var, nullptr);
#ifdef _WIN32
        _putenv_s(var, "");
#endif
    }

    if (!initializeFreeRDP()) {
        emit connectionFailed("Failed to initialize FreeRDP library.");
        return;
    }

    
    rdpSettings* settings = m_instance->context->settings;
    freerdp_settings_set_string(settings, FreeRDP_ServerHostname, m_host.toUtf8().constData());
    freerdp_settings_set_string(settings, FreeRDP_UserSpecifiedServerName, m_host.toUtf8().constData());
    freerdp_settings_set_uint32(settings, FreeRDP_ServerPort, m_port);
    freerdp_settings_set_string(settings, FreeRDP_Username, m_username.toUtf8().constData());
    freerdp_settings_set_string(settings, FreeRDP_Password, m_password.toUtf8().constData());

    
    freerdp_settings_set_uint32(settings, FreeRDP_DesktopWidth, m_width);
    freerdp_settings_set_uint32(settings, FreeRDP_DesktopHeight, m_height);

    qDebug() << "RdpClient: Host input =" << m_host;
    qDebug() << "RdpClient: ServerHostname setting =" << freerdp_settings_get_string(settings, FreeRDP_ServerHostname);
    qDebug() << "RdpClient: UserSpecifiedServerName setting =" << freerdp_settings_get_string(settings, FreeRDP_UserSpecifiedServerName);
    qDebug() << "RdpClient: ServerPort setting =" << freerdp_settings_get_uint32(settings, FreeRDP_ServerPort);
    
    
    freerdp_settings_set_uint32(settings, FreeRDP_ProxyType, 0);
    freerdp_settings_set_string(settings, FreeRDP_ProxyHostname, nullptr);
    freerdp_settings_set_uint16(settings, FreeRDP_ProxyPort, 0);
    freerdp_settings_set_string(settings, FreeRDP_ProxyUsername, nullptr);
    freerdp_settings_set_string(settings, FreeRDP_ProxyPassword, nullptr);

    
    freerdp_settings_set_bool(settings, FreeRDP_IgnoreCertificate, TRUE);
    freerdp_settings_set_bool(settings, FreeRDP_FastPathInput, TRUE);
    freerdp_settings_set_bool(settings, FreeRDP_FastPathOutput, TRUE);
    freerdp_settings_set_uint32(settings, FreeRDP_PerformanceFlags,
        PERF_DISABLE_WALLPAPER | PERF_DISABLE_FULLWINDOWDRAG |
        PERF_DISABLE_MENUANIMATIONS | PERF_DISABLE_THEMES |
        PERF_DISABLE_CURSOR_SHADOW | PERF_DISABLE_FONT_SMOOTHING
    );

    
    freerdp_settings_set_bool(settings, FreeRDP_RedirectClipboard, TRUE);
    freerdp_settings_set_bool(settings, FreeRDP_RedirectDrives, FALSE);

    
    if (!freerdp_client_load_addins(m_instance->context->channels, settings)) {
        qWarning() << "RdpClient: Failed to load client add-ins (e.g. cliprdr)";
    }

    m_worker = new RdpWorker(this);
    m_worker->start();
}

void RdpClient::disconnectFromServer() {
    if (m_worker) {
        m_worker->stop();
        delete m_worker;
        m_worker = nullptr;
    }
    cleanupFreeRDP();
}

bool RdpClient::initializeFreeRDP() {
    RDP_CLIENT_ENTRY_POINTS entryPoints;
    ZeroMemory(&entryPoints, sizeof(RDP_CLIENT_ENTRY_POINTS));
    entryPoints.Size = sizeof(RDP_CLIENT_ENTRY_POINTS);
    entryPoints.Version = RDP_CLIENT_INTERFACE_VERSION;
    entryPoints.ContextSize = sizeof(MyRdpContext);
    entryPoints.ClientNew = [](freerdp* instance, rdpContext* context) -> BOOL {
        MyRdpContext* my_ctx = (MyRdpContext*)context;
        my_ctx->client = nullptr;
        return TRUE;
    };
    entryPoints.ClientFree = [](freerdp* instance, rdpContext* context) {
        Q_UNUSED(instance);
        Q_UNUSED(context);
    };

    rdpContext* context = freerdp_client_context_new(&entryPoints);
    if (!context) {
        qWarning() << "Failed to create FreeRDP client context";
        return false;
    }

    m_instance = context->instance;
    MyRdpContext* my_ctx = (MyRdpContext*)context;
    my_ctx->client = this;

    
    m_instance->PreConnect = [](freerdp* instance) -> BOOL {
        if (!gdi_init(instance, PIXEL_FORMAT_BGRA32)) {
            qWarning() << "Failed to initialize FreeRDP GDI";
            return FALSE;
        }
        return TRUE;
    };

    m_instance->PostConnect = [](freerdp* instance) -> BOOL {
        instance->context->update->EndPaint = [](rdpContext* context) -> BOOL {
            MyRdpContext* my_ctx = (MyRdpContext*)context;
            if (my_ctx && my_ctx->client) {
                my_ctx->client->onEndPaint();
            }
            return TRUE;
        };

        instance->context->update->DesktopResize = [](rdpContext* context) -> BOOL {
            UINT32 w = freerdp_settings_get_uint32(context->settings, FreeRDP_DesktopWidth);
            UINT32 h = freerdp_settings_get_uint32(context->settings, FreeRDP_DesktopHeight);
            
            if (!gdi_resize(context->gdi, w, h)) {
                qWarning() << "Failed to resize FreeRDP GDI";
                return FALSE;
            }

            MyRdpContext* my_ctx = (MyRdpContext*)context;
            if (my_ctx && my_ctx->client) {
                my_ctx->client->onDesktopResize(w, h);
            }
            return TRUE;
        };

        return TRUE;
    };

    m_instance->AuthenticateEx = [](freerdp* instance, char** username, char** password, char** domain, rdp_auth_reason reason) -> BOOL {
        Q_UNUSED(reason);
        rdpSettings* settings = instance->context->settings;
        const char* s_user = freerdp_settings_get_string(settings, FreeRDP_Username);
        const char* s_pass = freerdp_settings_get_string(settings, FreeRDP_Password);
        
        *username = allocate_string(s_user);
        *password = allocate_string(s_pass);
        *domain = allocate_string("");
        return TRUE;
    };

    m_instance->VerifyCertificateEx = [](freerdp* instance, const char* host, UINT16 port,
                                         const char* common_name, const char* subject, const char* issuer,
                                         const char* fingerprint, DWORD flags) -> DWORD {
        Q_UNUSED(instance); Q_UNUSED(host); Q_UNUSED(port); Q_UNUSED(common_name);
        Q_UNUSED(subject); Q_UNUSED(issuer); Q_UNUSED(fingerprint); Q_UNUSED(flags);
        return 1; 
    };

    m_instance->VerifyChangedCertificateEx = [](freerdp* instance, const char* host, UINT16 port,
                                                const char* common_name, const char* subject, const char* issuer,
                                                const char* fingerprint, const char* old_subject,
                                                const char* old_issuer, const char* old_fingerprint, DWORD flags) -> DWORD {
        Q_UNUSED(instance); Q_UNUSED(host); Q_UNUSED(port); Q_UNUSED(common_name);
        Q_UNUSED(subject); Q_UNUSED(issuer); Q_UNUSED(fingerprint);
        Q_UNUSED(old_subject); Q_UNUSED(old_issuer); Q_UNUSED(old_fingerprint); Q_UNUSED(flags);
        return 1; 
    };

    return true;
}

void RdpClient::cleanupFreeRDP() {
    if (m_instance) {
        gdi_free(m_instance);
        freerdp_client_context_free(m_instance->context);
        m_instance = nullptr;
    }
}

void RdpClient::onEndPaint() {
    QMutexLocker locker(&m_frameMutex);
    
    if (m_instance && m_instance->context && m_instance->context->gdi) {
        rdpGdi* gdi = m_instance->context->gdi;
        if (gdi->primary_buffer) {
            int w = gdi->width;
            int h = gdi->height;
            
            if (m_frameImage.width() != w || m_frameImage.height() != h) {
                m_frameImage = QImage(w, h, QImage::Format_ARGB32);
            }
            
            int src_stride = gdi->stride;
            int dst_stride = m_frameImage.bytesPerLine();
            
            for (int y = 0; y < h; ++y) {
                const BYTE* src_row = gdi->primary_buffer + (y * src_stride);
                BYTE* dst_row = m_frameImage.scanLine(y);
                memcpy(dst_row, src_row, w * 4); 
            }
            
            emit frameUpdated();
        }
    }
}

void RdpClient::onDesktopResize(int width, int height) {
    emit desktopResized(width, height);
}

QImage RdpClient::getFrame() {
    QMutexLocker locker(&m_frameMutex);
    
    return m_frameImage;
}

bool RdpClient::isConnected() const {
    QMutexLocker locker(&m_stateMutex);
    return m_connected;
}

void RdpClient::sendMouseEvent(uint16_t flags, uint16_t x, uint16_t y) {
    QMutexLocker locker(&m_stateMutex);
    if (m_connected && m_instance && m_instance->context && m_instance->context->input) {
        freerdp_input_send_mouse_event(m_instance->context->input, flags, x, y);
    }
}

void RdpClient::sendKeyboardEvent(uint16_t flags, uint8_t scancode) {
    QMutexLocker locker(&m_stateMutex);
    if (m_connected && m_instance && m_instance->context && m_instance->context->input) {
        freerdp_input_send_keyboard_event(m_instance->context->input, flags, scancode);
    }
}

void RdpClient::sendUnicodeKeyboardEvent(uint16_t flags, uint16_t code) {
    QMutexLocker locker(&m_stateMutex);
    if (m_connected && m_instance && m_instance->context && m_instance->context->input) {
        freerdp_input_send_unicode_keyboard_event(m_instance->context->input, flags, code);
    }
}
