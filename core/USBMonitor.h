#ifndef USBMONITOR_H
#define USBMONITOR_H

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QSet>
#include <QString>

struct USBDevice {
    QString vendorId;
    QString productId;
    QString bus;
    QString device;
    QString description;
    
    QString identifier() const {
        return QString("%1:%2").arg(vendorId, productId);
    }
};

class USBMonitor : public QObject
{
    Q_OBJECT

public:
    explicit USBMonitor(QObject *parent = nullptr);
    ~USBMonitor();
    
    void startMonitoring();
    void stopMonitoring();

signals:
    void usbDeviceInserted(const USBDevice &device);
    void usbDeviceRemoved(const USBDevice &device);

private slots:
    void checkUSBDevices();

private:
    QTimer *pollTimer;
    QSet<QString> connectedDevices;  // Track currently connected USB IDs
    
    QList<USBDevice> getConnectedUSBDevices();
};

#endif // USBMONITOR_H
