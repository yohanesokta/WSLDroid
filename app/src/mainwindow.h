#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QSettings>
#include <functional>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void startAndroid();
    void stopAndroid();

private:
    void loadSettings();
    void saveSettings();
    void logMessage(const QString &level, const QString &message);
    void runScript(const QString &scriptName, const QStringList &args);
    void runWslCommand(const QStringList &args, const std::function<void(int, QProcess::ExitStatus, const QString &, const QString &)> &finishedCallback);
    void ensureDistroRunning();
    void waitForDistroRunning();
    void startWeston();
    void startRdpEmulator();
    void waitForWaydroidIp();
    void startSocat();
    void waitForSocat();
    void connectAdb();
    void attemptAdbConnect();

    Ui::MainWindow *ui;
    QSettings *settings;
    QString m_distro;
    QString m_width;
    QString m_height;
    QString m_waydroidIp;
    QString m_wslIp;
    bool m_distroWasRunning = false;
};
#endif // MAINWINDOW_H
