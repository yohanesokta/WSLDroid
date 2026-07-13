#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include <QAbstractButton>
#include <QSize>

class ToolButton : public QAbstractButton {
    Q_OBJECT
public:
    enum IconType {
        IconPower,
        IconVolumeUp,
        IconVolumeDown,
        IconCamera,
        IconBack,
        IconHome,
        IconRecents,
        IconHelp,
        IconMinimize,
        IconClose,
        IconInstallApk
    };

    explicit ToolButton(IconType type, QWidget* parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    IconType m_type;
    bool m_hovered;
};

#endif 
