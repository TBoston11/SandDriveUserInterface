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
    
    // USB device passthrough
    bool attachUSBDevice(const QString &vendorId, const QString &productId);
    bool detachUSBDevice(const QString &vendorId, const QString &productId);
    
    // Get virsh console process for output/input
    QProcess* getVirshProcess() const { return virshProcess; }

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

    QProcess *virshProcess;
    QString vmName;
    
    void setupPaths();
    QString createUserOverlay();
    void startConsole();
};

#endif // VMMANAGER_H
