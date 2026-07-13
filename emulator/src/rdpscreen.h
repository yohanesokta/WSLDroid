#ifndef RDPSCREEN_H
#define RDPSCREEN_H

#include <QWidget>
#include <QImage>

class RdpClient;

class RdpScreen : public QWidget {
    Q_OBJECT
public:
    enum ScalingMode {
        FitAspectRatio,
        Stretch
    };

    explicit RdpScreen(RdpClient* client, QWidget* parent = nullptr);
    ~RdpScreen() override = default;

    void setScalingMode(ScalingMode mode);
    ScalingMode scalingMode() const { return m_scalingMode; }

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private:
    RdpClient* m_client;
    ScalingMode m_scalingMode;

    
    bool mapToRdpCoordinates(const QPoint& localPos, int& rdpX, int& rdpY) const;
    void sendMouseEvent(QMouseEvent* event, bool isDown);
};

#endif 
