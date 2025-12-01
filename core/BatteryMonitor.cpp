#include "BatteryMonitor.h"
#include <QRegularExpression>
#include <QDebug>

BatteryMonitor::BatteryMonitor(QObject *parent)
    : QObject(parent)
    , batteryPercentage(0)
    , upowerProcess(nullptr)
{
    // Create timer to poll battery status every 30 seconds
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &BatteryMonitor::updateBatteryStatus);
    updateTimer->start(30000); // 30 seconds
    
    // Get initial battery status
    updateBatteryStatus();
}

BatteryMonitor::~BatteryMonitor()
{
    if (upowerProcess) {
        upowerProcess->kill();
        delete upowerProcess;
    }
}

void BatteryMonitor::updateBatteryStatus()
{
    // Don't start a new process if one is already running
    if (upowerProcess && upowerProcess->state() == QProcess::Running) {
        return;
    }
    
    // Create new process
    if (upowerProcess) {
        delete upowerProcess;
    }
    
    upowerProcess = new QProcess(this);
    connect(upowerProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &BatteryMonitor::processFinished);
    
    // Run upower command
    upowerProcess->start("upower", QStringList() << "-i" << "/org/freedesktop/UPower/devices/battery_hidpp_battery_0");
}

void BatteryMonitor::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
        qWarning() << "Failed to read battery status";
        return;
    }
    
    QString output = upowerProcess->readAllStandardOutput();
    parseBatteryOutput(output);
    
    upowerProcess->deleteLater();
    upowerProcess = nullptr;
}

void BatteryMonitor::parseBatteryOutput(const QString &output)
{
    // Look for "percentage:          XX%"
    QRegularExpression re("percentage:\\s+(\\d+)%");
    QRegularExpressionMatch match = re.match(output);
    
    if (match.hasMatch()) {
        int newPercentage = match.captured(1).toInt();
        
        if (newPercentage != batteryPercentage) {
            batteryPercentage = newPercentage;
            emit batteryPercentageChanged(batteryPercentage);
            qDebug() << "Battery percentage:" << batteryPercentage << "%";
        }
    }
}
