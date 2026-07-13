#ifndef PHONEFRAME_H
#define PHONEFRAME_H

#include <QWidget>

class PhoneFrame : public QWidget {
    Q_OBJECT
public:
    explicit PhoneFrame(QWidget* screenWidget, int rdpWidth, int rdpHeight, QWidget* parent = nullptr);
    ~PhoneFrame() override = default;

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const { return m_orientation; }

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    QWidget* m_screenWidget;
    Qt::Orientation m_orientation;
    int m_rdpWidth;
    int m_rdpHeight;

    
    static const int BezelWidth = 20;

    void updateMargins();
};

#endif 
