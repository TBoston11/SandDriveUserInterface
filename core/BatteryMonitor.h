#ifndef BATTERYMONITOR_H
#define BATTERYMONITOR_H

#include <QObject>
#include <QProcess>
#include <QTimer>

class BatteryMonitor : public QObject
{
    Q_OBJECT
public:
    explicit BatteryMonitor(QObject *parent = nullptr);
    ~BatteryMonitor();
    
    int getBatteryPercentage() const { return batteryPercentage; }
    
signals:
    void batteryPercentageChanged(int percentage);
    
private slots:
    void updateBatteryStatus();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    
private:
    QTimer *updateTimer;
    QProcess *upowerProcess;
    int batteryPercentage;
    
    void parseBatteryOutput(const QString &output);
};

#endif // BATTERYMONITOR_H
