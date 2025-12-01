#include "USBMonitor.h"
#include <QDebug>
#include <QRegularExpression>

USBMonitor::USBMonitor(QObject *parent)
    : QObject(parent)
    , pollTimer(new QTimer(this))
{
    connect(pollTimer, &QTimer::timeout, this, &USBMonitor::checkUSBDevices);
}

USBMonitor::~USBMonitor()
{
    stopMonitoring();
}

void USBMonitor::startMonitoring()
{
    qDebug() << "Starting USB monitoring...";
    
    // Initial scan
    QList<USBDevice> devices = getConnectedUSBDevices();
    for (const USBDevice &device : devices) {
        connectedDevices.insert(device.identifier());
    }
    
    // Poll every 2 seconds for changes
    pollTimer->start(2000);
}

void USBMonitor::stopMonitoring()
{
    pollTimer->stop();
}

void USBMonitor::checkUSBDevices()
{
    QList<USBDevice> currentDevices = getConnectedUSBDevices();
    QSet<QString> currentIds;
    
    // Build set of current device IDs
    for (const USBDevice &device : currentDevices) {
        currentIds.insert(device.identifier());
    }
    
    // Check for new devices (inserted)
    for (const USBDevice &device : currentDevices) {
        if (!connectedDevices.contains(device.identifier())) {
            qDebug() << "USB device inserted:" << device.description;
            emit usbDeviceInserted(device);
        }
    }
    
    // Check for removed devices
    QSet<QString> removed = connectedDevices - currentIds;
    for (const QString &id : removed) {
        qDebug() << "USB device removed:" << id;
        // Create a minimal device object for the removed signal
        USBDevice device;
        QStringList parts = id.split(':');
        if (parts.size() == 2) {
            device.vendorId = parts[0];
            device.productId = parts[1];
        }
        emit usbDeviceRemoved(device);
    }
    
    // Update tracked devices
    connectedDevices = currentIds;
}

QList<USBDevice> USBMonitor::getConnectedUSBDevices()
{
    QList<USBDevice> devices;
    
    QProcess lsusb;
    lsusb.start("lsusb");
    lsusb.waitForFinished(3000);
    
    QString output = lsusb.readAllStandardOutput();
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    
    // Parse lsusb output: Bus 001 Device 002: ID 046d:c52b Logitech, Inc. Unifying Receiver
    QRegularExpression re("Bus (\\d+) Device (\\d+): ID ([0-9a-f]+):([0-9a-f]+) (.+)");
    
    for (const QString &line : lines) {
        QRegularExpressionMatch match = re.match(line);
        if (match.hasMatch()) {
            USBDevice device;
            device.bus = match.captured(1);
            device.device = match.captured(2);
            device.vendorId = match.captured(3);
            device.productId = match.captured(4);
            device.description = match.captured(5).trimmed();
            
            // Skip USB hubs and root hubs
            if (!device.description.contains("Hub", Qt::CaseInsensitive) &&
                !device.description.contains("Linux Foundation", Qt::CaseInsensitive)) {
                devices.append(device);
            }
        }
    }
    
    return devices;
}
