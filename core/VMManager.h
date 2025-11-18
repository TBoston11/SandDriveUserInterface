#ifndef VMMANAGER_H
#define VMMANAGER_H

#include <QObject>
#include <QProcess>
#include <QString>

class VMManager : public QObject
{
    Q_OBJECT

public:
    static VMManager& instance();

    // VM lifecycle management
    bool startVM();
    void stopVM();
    bool isVMRunning() const;
    
    // VM configuration
    QString getBaseImagePath() const;
    bool baseImageExists() const;
    
    // Get QEMU process for output/input
    QProcess* getQemuProcess() const { return qemuProcess; }

signals:
    void vmStarted();
    void vmStopped();
    void vmOutputReady(const QString &output);
    void vmError(const QString &error);

private:
    VMManager();
    ~VMManager();
    VMManager(const VMManager&) = delete;
    VMManager& operator=(const VMManager&) = delete;

    QProcess *qemuProcess;
    QString baseImagePath;
    QString userOverlayPath;
    
    void setupPaths();
    QString createUserOverlay();
};

#endif // VMMANAGER_H
