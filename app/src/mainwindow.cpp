#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QDateTime>
#include <QDir>
#include <QProcess>
#include <QTimer>
#include <QCoreApplication>

namespace {
QString trimmedDistroName(const QString &distroText)
{
    if (distroText.contains(" ")) {
        return distroText.split(" ").first();
    }
    return distroText;
}

QString decodeProcessOutput(const QByteArray &data)
{
    if (data.isEmpty()) {
        return {};
    }

    if (data.size() >= 2 && static_cast<unsigned char>(data[0]) == 0xFF && static_cast<unsigned char>(data[1]) == 0xFE) {
        return QString::fromUtf16(reinterpret_cast<const char16_t *>(data.constData() + 2), (data.size() - 2) / 2);
    }

    if (data.contains('\0')) {
        return QString::fromUtf16(reinterpret_cast<const char16_t *>(data.constData()), data.size() / 2);
    }

    if (data.size() >= 3
        && static_cast<unsigned char>(data[0]) == 0xEF
        && static_cast<unsigned char>(data[1]) == 0xBB
        && static_cast<unsigned char>(data[2]) == 0xBF) {
        return QString::fromUtf8(data.constData() + 3, data.size() - 3);
    }

    return QString::fromUtf8(data);
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      settings(new QSettings("config.ini", QSettings::IniFormat, this))
{
    setWindowTitle("Emulator Manager");
    resize(800, 600);
    setStyleSheet("background-color: #050505; color: #f5f5f5;");

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Controls Layout
    QHBoxLayout *controlsLayout = new QHBoxLayout();
    
    // Lifecycle Group
    QGroupBox *lifecycleGroup = new QGroupBox("Device Lifecycle");
    lifecycleGroup->setStyleSheet("QGroupBox { border: 1px solid #2f2f2f; border-radius: 5px; margin-top: 10px; color: #f5f5f5; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 0 3px; color: #b5b5b5; }");
    QVBoxLayout *lifecycleLayout = new QVBoxLayout(lifecycleGroup);
    
    QPushButton *startBtn = new QPushButton("Start Android");
    startBtn->setStyleSheet("background-color: #1b2a1b; color: #f5f5f5; padding: 10px; border: 1px solid #2f3f2f; border-radius: 5px; font-weight: bold;");
    
    QPushButton *stopBtn = new QPushButton("Stop");
    stopBtn->setStyleSheet("background-color: #2a1b1b; color: #f5f5f5; padding: 10px; border: 1px solid #3f2f2f; border-radius: 5px; font-weight: bold;");
    
    lifecycleLayout->addWidget(startBtn);
    lifecycleLayout->addWidget(stopBtn);
    
    // Settings Group
    QGroupBox *settingsGroup = new QGroupBox("Settings");
    settingsGroup->setStyleSheet("QGroupBox { border: 1px solid #2f2f2f; border-radius: 5px; margin-top: 10px; color: #f5f5f5; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 0 3px; color: #b5b5b5; }");
    QVBoxLayout *settingsLayout = new QVBoxLayout(settingsGroup);
    
    QHBoxLayout *resLayout = new QHBoxLayout();
    resLayout->addWidget(new QLabel("Width:"));
    QLineEdit *widthEdit = new QLineEdit("393");
    widthEdit->setObjectName("widthEdit");
    widthEdit->setStyleSheet("background-color: #111111; border: 1px solid #333333; padding: 5px; color: #f5f5f5;");
    resLayout->addWidget(widthEdit);
    resLayout->addWidget(new QLabel("Height:"));
    QLineEdit *heightEdit = new QLineEdit("852");
    heightEdit->setObjectName("heightEdit");
    heightEdit->setStyleSheet("background-color: #111111; border: 1px solid #333333; padding: 5px; color: #f5f5f5;");
    resLayout->addWidget(heightEdit);
    
    settingsLayout->addLayout(resLayout);
    
    QHBoxLayout *distroLayout = new QHBoxLayout();
    distroLayout->addWidget(new QLabel("WSL Distro:"));
    QComboBox *distroCombo = new QComboBox();
    distroCombo->setObjectName("distroCombo");
    
    QProcess wslProc;
    wslProc.start("wsl.exe", QStringList() << "-l" << "-q");
    wslProc.waitForFinished(2000);
    QByteArray wslOut = wslProc.readAllStandardOutput();
    QString distrosStr;
    if (wslOut.contains('\0')) {
        distrosStr = QString::fromUtf16(reinterpret_cast<const char16_t*>(wslOut.data()), wslOut.size() / 2);
    } else {
        distrosStr = QString::fromLocal8Bit(wslOut);
    }
    QStringList distrosList = distrosStr.split('\n', Qt::SkipEmptyParts);
    for (QString d : distrosList) {
        d = d.remove('\r').trimmed();
        if (!d.isEmpty()) {
            distroCombo->addItem(d);
        }
    }
    if (distroCombo->count() == 0) {
        distroCombo->addItem("Ubuntu-22.04 (Default)");
    }
    
    distroCombo->setStyleSheet("background-color: #111111; border: 1px solid #333333; padding: 5px; color: #f5f5f5;");
    distroLayout->addWidget(distroCombo);
    settingsLayout->addLayout(distroLayout);
    
    controlsLayout->addWidget(lifecycleGroup);
    controlsLayout->addWidget(settingsGroup);
    
    mainLayout->addLayout(controlsLayout);
    
    // Logs
    QLabel *logLabel = new QLabel("DEBUG LOGS");
    logLabel->setStyleSheet("color: #b5b5b5; font-weight: bold; margin-top: 10px;");
    mainLayout->addWidget(logLabel);
    
    QTextEdit *logEdit = new QTextEdit();
    logEdit->setObjectName("logEdit");
    logEdit->setReadOnly(true);
    logEdit->setStyleSheet("background-color: #0b0b0b; border: 1px solid #2f2f2f; font-family: monospace; color: #eaeaea;");
    mainLayout->addWidget(logEdit);
    
    connect(startBtn, &QPushButton::clicked, this, &MainWindow::startAndroid);
    connect(stopBtn, &QPushButton::clicked, this, &MainWindow::stopAndroid);
    
    loadSettings();
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::loadSettings()
{
    findChild<QLineEdit*>("widthEdit")->setText(settings->value("width", "393").toString());
    findChild<QLineEdit*>("heightEdit")->setText(settings->value("height", "852").toString());
    findChild<QComboBox*>("distroCombo")->setCurrentText(settings->value("distro", "Ubuntu-22.04 (Default)").toString());
}

void MainWindow::saveSettings()
{
    settings->setValue("width", findChild<QLineEdit*>("widthEdit")->text());
    settings->setValue("height", findChild<QLineEdit*>("heightEdit")->text());
    settings->setValue("distro", findChild<QComboBox*>("distroCombo")->currentText());
}

void MainWindow::logMessage(const QString &level, const QString &message)
{
    if (message.trimmed().isEmpty()) {
        return;
    }

    QTextEdit *logEdit = findChild<QTextEdit*>("logEdit");
    QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString color = "#eaeaea";
    if (level == "ERROR") color = "#f87171";
    else if (level == "INFO") color = "#86efac";
    else if (level == "WARN") color = "#fbbf24";
    else if (level == "DEBUG") color = "#c4c4c4";
    
    QString formattedMsg = QString("<span style='color: #8a8a8a;'>%1</span> &nbsp; <span style='color: %2; font-weight: bold;'>%3</span> &nbsp; %4")
                               .arg(timeStr, color, level.leftJustified(5, ' '), message);
    logEdit->append(formattedMsg);
}

void MainWindow::runScript(const QString &scriptName, const QStringList &args)
{
    QString scriptPath = QCoreApplication::applicationDirPath() + "/scripts/" + scriptName;
    logMessage("INFO", "Running script: " + scriptPath + " " + args.join(" "));
    
    QProcess *scriptProcess = new QProcess(this);
    connect(scriptProcess, &QProcess::readyReadStandardOutput, this, [this, scriptProcess]() {
        QByteArray output = scriptProcess->readAllStandardOutput();
        logMessage("DEBUG", decodeProcessOutput(output));
    });
    connect(scriptProcess, &QProcess::readyReadStandardError, this, [this, scriptProcess]() {
        QByteArray error = scriptProcess->readAllStandardError();
        logMessage("ERROR", decodeProcessOutput(error));
    });
    connect(scriptProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), scriptProcess, &QObject::deleteLater);
    
    scriptProcess->start(scriptPath, args);
}

void MainWindow::runWslCommand(const QStringList &args, const std::function<void(int, QProcess::ExitStatus, const QString &, const QString &)> &finishedCallback)
{
    QProcess *process = new QProcess(this);
    QByteArray *stdoutBuffer = new QByteArray();
    QByteArray *stderrBuffer = new QByteArray();

    connect(process, &QProcess::readyReadStandardOutput, this, [this, process, stdoutBuffer]() {
        *stdoutBuffer += process->readAllStandardOutput();
    });
    connect(process, &QProcess::readyReadStandardError, this, [this, process, stderrBuffer]() {
        *stderrBuffer += process->readAllStandardError();
    });
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this, process, stdoutBuffer, stderrBuffer, finishedCallback](int exitCode, QProcess::ExitStatus exitStatus) {
                const QString stdoutText = decodeProcessOutput(*stdoutBuffer).trimmed();
                const QString stderrText = decodeProcessOutput(*stderrBuffer).trimmed();
                if (!stdoutText.isEmpty()) {
                    logMessage("DEBUG", stdoutText);
                }
                if (!stderrText.isEmpty()) {
                    logMessage("ERROR", stderrText);
                }
                if (finishedCallback) {
                    finishedCallback(exitCode, exitStatus, stdoutText, stderrText);
                }
                delete stdoutBuffer;
                delete stderrBuffer;
                process->deleteLater();
            });

    process->start("wsl.exe", args);
}

void MainWindow::ensureDistroRunning()
{
    m_distroWasRunning = false;
    logMessage("INFO", "Checking WSL distro state for " + m_distro + "...");

    runWslCommand(
        QStringList{ "-l", "-v" },
        [this](int, QProcess::ExitStatus, const QString &stdoutText, const QString &) {
            const QStringList lines = stdoutText.split('\n', Qt::SkipEmptyParts);
            for (const QString &line : lines) {
                const QString trimmedLine = line.trimmed();
                if (trimmedLine.startsWith(m_distro + " ") || trimmedLine.startsWith("* " + m_distro + " ")) {
                    if (trimmedLine.contains("Running", Qt::CaseInsensitive)) {
                        m_distroWasRunning = true;
                    }
                    break;
                }
            }

            if (m_distroWasRunning) {
                logMessage("INFO", "WSL distro is already running.");
                startWeston();
                return;
            }

            logMessage("INFO", "Starting WSL distro " + m_distro + "...");
            const bool started = QProcess::startDetached(
                "wsl.exe",
                QStringList{ "-d", m_distro, "--exec", "bash", "-lc", "true" });

            if (!started) {
                logMessage("ERROR", "Failed to start WSL distro " + m_distro + ".");
                return;
            }

            QTimer::singleShot(1500, this, &MainWindow::waitForDistroRunning);
        });
}

void MainWindow::waitForDistroRunning()
{
    runWslCommand(
        QStringList{ "-l", "-v" },
        [this](int, QProcess::ExitStatus, const QString &stdoutText, const QString &) {
            const QStringList lines = stdoutText.split('\n', Qt::SkipEmptyParts);
            for (const QString &line : lines) {
                const QString trimmedLine = line.trimmed();
                if (trimmedLine.startsWith(m_distro + " ") || trimmedLine.startsWith("* " + m_distro + " ")) {
                    if (trimmedLine.contains("Running", Qt::CaseInsensitive)) {
                        logMessage("INFO", "WSL distro is ready. Waiting 5 seconds for systemd initialization...");
                        QTimer::singleShot(5000, this, &MainWindow::startWeston);
                        return;
                    }
                    break;
                }
            }

            QTimer::singleShot(1000, this, &MainWindow::waitForDistroRunning);
        });
}

void MainWindow::startWeston()
{
    logMessage("INFO", "Starting Weston from Qt...");
    const bool started = QProcess::startDetached("wsl.exe", QStringList{ "-d", m_distro, "--exec", "/opt/start-weston.sh", m_width, m_height });
    if (!started) {
        logMessage("ERROR", "Failed to start Weston.");
        return;
    }

    QTimer::singleShot(2000, this, &MainWindow::startRdpEmulator);
}

void MainWindow::startRdpEmulator()
{
    logMessage("INFO", "Starting RDP Emulator from Qt...");
    const QString emulatorPath = QCoreApplication::applicationDirPath() + "/rdp_emulator.exe";
    const bool started = QProcess::startDetached(
        emulatorPath,
        QStringList{ "--width=" + m_width, "--height=" + m_height, "--host=localhost", "--port=3390" });

    if (!started) {
        logMessage("ERROR", "Failed to start RDP Emulator.");
        return;
    }

    QTimer::singleShot(1000, this, &MainWindow::waitForWaydroidIp);
}

void MainWindow::waitForWaydroidIp()
{
    runWslCommand(
        QStringList{ "-d", m_distro, "--exec", "bash", "-lc", "waydroid status | sed -n 's/^.*IP address:[[:space:]]*//p' | tail -n 1" },
        [this](int, QProcess::ExitStatus, const QString &stdoutText, const QString &) {
            const QString waydroidIp = stdoutText.trimmed();
            if (!waydroidIp.isEmpty()) {
                m_waydroidIp = waydroidIp;
                logMessage("INFO", "Waydroid IP = " + m_waydroidIp);
                startSocat();
                return;
            }

            QTimer::singleShot(3000, this, &MainWindow::waitForWaydroidIp);
        });
}

void MainWindow::startSocat()
{
    logMessage("INFO", "Starting socat from Qt...");
    const bool started = QProcess::startDetached(
        "wsl.exe",
        QStringList{ "-d", m_distro, "--exec", "/opt/start-socat.sh", m_waydroidIp });

    if (!started) {
        logMessage("ERROR", "Failed to start socat.");
        return;
    }

    QTimer::singleShot(1000, this, &MainWindow::waitForSocat);
}

void MainWindow::waitForSocat()
{
    runWslCommand(
        QStringList{ "-d", m_distro, "--exec", "bash", "-lc", "ss -ltn | grep -q ':5555 ' && echo ready" },
        [this](int exitCode, QProcess::ExitStatus, const QString &stdoutText, const QString &) {
            if (exitCode == 0 && stdoutText.contains("ready")) {
                logMessage("INFO", "Socat is ready.");
                connectAdb();
                return;
            }

            QTimer::singleShot(2000, this, &MainWindow::waitForSocat);
        });
}

void MainWindow::connectAdb()
{
    runWslCommand(
        QStringList{ "-d", m_distro, "--exec", "bash", "-lc", "hostname -I | awk '{print $1}'" },
        [this](int, QProcess::ExitStatus, const QString &stdoutText, const QString &) {
            m_wslIp = stdoutText.trimmed();
            
            
            if (m_wslIp.isEmpty()) {
                logMessage("ERROR", "Unable to determine WSL IP.");
                return;
            }


            logMessage("INFO", "WSL IP = " + m_wslIp);
             QProcess::startDetached("cmd",
                    QStringList{"/c","echo", m_wslIp + ":5555>config.cache"}
                );
            attemptAdbConnect();
        });
}

void MainWindow::attemptAdbConnect()
{
    logMessage("INFO", "Connecting ADB to " + m_wslIp + ":5555...");

    QProcess *adbProcess = new QProcess(this);
    QByteArray *stdoutBuffer = new QByteArray();
    QByteArray *stderrBuffer = new QByteArray();

    connect(adbProcess, &QProcess::readyReadStandardOutput, this, [adbProcess, stdoutBuffer]() {
        *stdoutBuffer += adbProcess->readAllStandardOutput();
    });
    connect(adbProcess, &QProcess::readyReadStandardError, this, [adbProcess, stderrBuffer]() {
        *stderrBuffer += adbProcess->readAllStandardError();
    });
    connect(adbProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this, adbProcess, stdoutBuffer, stderrBuffer](int, QProcess::ExitStatus) {
                const QString output = QString::fromLocal8Bit(*stdoutBuffer).trimmed();
                const QString error = QString::fromLocal8Bit(*stderrBuffer).trimmed();
                const QString combined = output + "\n" + error;

                if (!output.isEmpty()) {
                    logMessage("DEBUG", output);
                }
                if (!error.isEmpty()) {
                    logMessage("ERROR", error);
                }

                if (combined.contains("connected to") || combined.contains("already connected")) {
                    logMessage("INFO", "ADB connect completed.");
                    delete stdoutBuffer;
                    delete stderrBuffer;
                    adbProcess->deleteLater();
                    return;
                }

                logMessage("WARN", "ADB not ready yet, retrying in 2 seconds...");
                delete stdoutBuffer;
                delete stderrBuffer;
                adbProcess->deleteLater();
                QTimer::singleShot(2000, this, &MainWindow::attemptAdbConnect);
            });

    adbProcess->start("adb", QStringList{ "connect", m_wslIp + ":5555" });
}

void MainWindow::startAndroid()
{
    logMessage("INFO", "Starting Android...");
    m_distro = trimmedDistroName(findChild<QComboBox*>("distroCombo")->currentText());
    m_width = findChild<QLineEdit*>("widthEdit")->text();
    m_height = findChild<QLineEdit*>("heightEdit")->text();
    m_waydroidIp.clear();
    m_wslIp.clear();

    ensureDistroRunning();
}

void MainWindow::stopAndroid()
{
    logMessage("INFO", "Stopping Android...");
    QString distro = findChild<QComboBox*>("distroCombo")->currentText();
    if(distro.contains(" ")) distro = distro.split(" ").first();
    QStringList args;
    args << distro;
    runScript("stop-android.bat", args);
}



