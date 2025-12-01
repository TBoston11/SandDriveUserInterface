#include "VMManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QThread>
#include <QRegularExpression>

VMManager::VMManager()
    : virshProcess(nullptr)
    , vmName("sdui-debian")
{
    // VM is managed by libvirt system daemon
}

VMManager::~VMManager()
{
    stopVM();
}

VMManager& VMManager::instance()
{
    static VMManager instance;
    return instance;
}

void VMManager::setupPaths()
{
    // Not needed - libvirt manages paths
}

QString VMManager::getBaseImagePath() const
{
    return "/var/lib/libvirt/images/sdui/sdui-debian.qcow2";
}

bool VMManager::baseImageExists() const
{
    // Check if VM exists in libvirt
    QProcess check;
    check.start("virsh", QStringList() << "--connect" << "qemu:///system" << "domstate" << vmName);
    check.waitForFinished(3000);
    return check.exitCode() == 0;
}

QString VMManager::createUserOverlay()
{
    // Libvirt handles disk management
    return getBaseImagePath();
}

bool VMManager::startVM()
{
    if (isVMRunning()) {
        qDebug() << "VM is already running";
        // Start console connection anyway
        startConsole();
        return true;
    }
    
    // Check if VM exists in libvirt
    if (!baseImageExists()) {
        emit vmError("VM '" + vmName + "' not found. Run setup script first.");
        return false;
    }
    
    // Start the VM using virsh
    QProcess startProcess;
    startProcess.start("virsh", QStringList() << "--connect" << "qemu:///system" << "start" << vmName);
    startProcess.waitForFinished(5000);
    
    if (startProcess.exitCode() != 0) {
        QString error = startProcess.readAllStandardError();
        if (!error.contains("already active")) {
            emit vmError("Failed to start VM: " + error);
            return false;
        }
    }
    
    // Confirm VM is actually running
    if (isVMRunning()) {
        qDebug() << "VM started successfully";
        emit vmStarted();
    } else {
        qDebug() << "VM start requested; waiting for state...";
    }
    
    // Connect to console
    startConsole();
    
    return true;
}

void VMManager::startConsole()
{
    if (virshProcess) {
        delete virshProcess;
    }
    
    // Get VM IP address with retries
    QString vmIP;
    for (int attempt = 0; attempt < 10; attempt++) {
        QProcess ipCheck;
        ipCheck.start("virsh", QStringList() << "--connect" << "qemu:///system" << "domifaddr" << vmName);
        ipCheck.waitForFinished(3000);
        QString ipOutput = ipCheck.readAllStandardOutput();
        
        QStringList lines = ipOutput.split('\n');
        for (const QString &line : lines) {
            if (line.contains("ipv4")) {
                QRegularExpression re("(\\d+\\.\\d+\\.\\d+\\.\\d+)");
                QRegularExpressionMatch match = re.match(line);
                if (match.hasMatch()) {
                    vmIP = match.captured(1);
                    break;
                }
            }
        }
        
        if (!vmIP.isEmpty()) {
            break;
        }
        QThread::sleep(2);
    }
    
    if (vmIP.isEmpty()) {
        emit vmError("VM starting... Wait 30 seconds for cloud-init to finish.");
        emit vmOutputReady("\nVM is booting. Cloud-init takes 30-90 seconds on first boot.\n");
        emit vmOutputReady("Click 'Start VM' again in a moment to connect.\n");
        return;
    }
    
    // Wait for SSH port to be reachable using timeout + nc
    bool sshReady = false;
    for (int attempt = 0; attempt < 20; attempt++) {
        QProcess portCheck;
        portCheck.start("sh", QStringList() << "-c" << QString("timeout 1 bash -c 'echo > /dev/tcp/%1/22' 2>/dev/null").arg(vmIP));
        portCheck.waitForFinished(2000);
        if (portCheck.exitCode() == 0) {
            sshReady = true;
            break;
        }
        QThread::sleep(1);
    }
    
    if (!sshReady) {
        emit vmError("SSH service not ready. Try connecting again.");
        emit vmOutputReady("\nSSH port not responding after 20s. Wait a moment and retry.\n");
        return;
    }
    
    // Create SSH process with sshpass for automatic login
    virshProcess = new QProcess(this);
    virshProcess->setProcessChannelMode(QProcess::MergedChannels);
    
    // Connect signals
    connect(virshProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        QString output = virshProcess->readAllStandardOutput();
        emit vmOutputReady(output);
    });
    
    connect(virshProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        qDebug() << "SSH disconnected with code:" << exitCode;
        emit vmOutputReady("\n[Connection closed]\n");
    });
    
    // Start SSH with sshpass for auto-login
    // Launch bash with interactive mode
    QStringList sshArgs;
    sshArgs << "-p" << "debian" 
            << "ssh"
            << "-o" << "StrictHostKeyChecking=no"
            << "-o" << "UserKnownHostsFile=/dev/null"
            << "-o" << "LogLevel=ERROR"
            << "debian@" + vmIP
            << "bash -i";  // Interactive bash shell
    
    virshProcess->start("sshpass", sshArgs);
    
    if (!virshProcess->waitForStarted(5000)) {
        emit vmError("Failed to connect SSH: " + virshProcess->errorString());
        emit vmOutputReady("\nSSH not ready yet. Wait and try again.\n");
        delete virshProcess;
        virshProcess = nullptr;
        return;
    }
    
    emit vmOutputReady("Connected to VM at " + vmIP + "\n");
    emit vmOutputReady("Logged in as debian\n\n");
}

void VMManager::stopVM()
{
    qDebug() << "Stopping VM...";
    
    // Disconnect console first
    if (virshProcess) {
        virshProcess->terminate();
        virshProcess->waitForFinished(1000);
        delete virshProcess;
        virshProcess = nullptr;
    }
    
    // Shutdown the VM gracefully
    QProcess shutdownProcess;
    shutdownProcess.start("virsh", QStringList() << "--connect" << "qemu:///system" << "shutdown" << vmName);
    shutdownProcess.waitForFinished(3000);
    
    emit vmStopped();
}

bool VMManager::isVMRunning() const
{
    QProcess check;
    check.start("virsh", QStringList() << "--connect" << "qemu:///system" << "domstate" << vmName);
    check.waitForFinished(2000);
    
    QString state = check.readAllStandardOutput().trimmed();
    return state == "running";
}

bool VMManager::attachUSBDevice(const QString &vendorId, const QString &productId)
{
    if (!isVMRunning()) {
        qDebug() << "Cannot attach USB: VM is not running";
        return false;
    }
    
    // Create XML for USB device passthrough
    QString usbXML = QString(
        "<hostdev mode='subsystem' type='usb'>\n"
        "  <source>\n"
        "    <vendor id='0x%1'/>\n"
        "    <product id='0x%2'/>\n"
        "  </source>\n"
        "</hostdev>\n"
    ).arg(vendorId, productId);
    
    // Write XML to temporary file
    QProcess mktemp;
    mktemp.start("mktemp");
    mktemp.waitForFinished(1000);
    QString tmpFile = mktemp.readAllStandardOutput().trimmed();
    
    if (tmpFile.isEmpty()) {
        qDebug() << "Failed to create temp file for USB XML";
        return false;
    }
    
    QFile file(tmpFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to write USB XML";
        return false;
    }
    file.write(usbXML.toUtf8());
    file.close();
    
    // Attach device using virsh
    QProcess attach;
    attach.start("virsh", QStringList() << "--connect" << "qemu:///system" 
                 << "attach-device" << vmName << tmpFile);
    attach.waitForFinished(5000);
    
    int exitCode = attach.exitCode();
    QString output = attach.readAllStandardOutput();
    QString error = attach.readAllStandardError();
    
    // Clean up temp file
    QFile::remove(tmpFile);
    
    if (exitCode == 0) {
        qDebug() << "USB device" << vendorId << ":" << productId << "attached successfully";
        emit vmOutputReady(QString("\n[USB Device %1:%2 connected to VM]\n").arg(vendorId, productId));
        return true;
    } else {
        qDebug() << "Failed to attach USB device:" << error;
        return false;
    }
}

bool VMManager::detachUSBDevice(const QString &vendorId, const QString &productId)
{
    if (!isVMRunning()) {
        return false;
    }
    
    // Create same XML structure for detach
    QString usbXML = QString(
        "<hostdev mode='subsystem' type='usb'>\n"
        "  <source>\n"
        "    <vendor id='0x%1'/>\n"
        "    <product id='0x%2'/>\n"
        "  </source>\n"
        "</hostdev>\n"
    ).arg(vendorId, productId);
    
    QProcess mktemp;
    mktemp.start("mktemp");
    mktemp.waitForFinished(1000);
    QString tmpFile = mktemp.readAllStandardOutput().trimmed();
    
    if (tmpFile.isEmpty()) {
        return false;
    }
    
    QFile file(tmpFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    file.write(usbXML.toUtf8());
    file.close();
    
    QProcess detach;
    detach.start("virsh", QStringList() << "--connect" << "qemu:///system" 
                 << "detach-device" << vmName << tmpFile);
    detach.waitForFinished(5000);
    
    QFile::remove(tmpFile);
    
    if (detach.exitCode() == 0) {
        qDebug() << "USB device" << vendorId << ":" << productId << "detached";
        emit vmOutputReady(QString("\n[USB Device %1:%2 disconnected from VM]\n").arg(vendorId, productId));
        return true;
    }
    
    return false;
}
