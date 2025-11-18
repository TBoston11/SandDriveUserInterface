#include "VMManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDebug>

VMManager::VMManager()
    : qemuProcess(nullptr)
{
    setupPaths();
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
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString vmsDir = appDataPath + "/vms";
    
    // Ensure vms directory exists
    QDir dir;
    if (!dir.exists(vmsDir)) {
        dir.mkpath(vmsDir);
    }
    
    baseImagePath = vmsDir + "/kali-base.qcow2";
    userOverlayPath = vmsDir + "/kali-user.qcow2";
}

QString VMManager::getBaseImagePath() const
{
    return baseImagePath;
}

bool VMManager::baseImageExists() const
{
    return QFile::exists(baseImagePath);
}

QString VMManager::createUserOverlay()
{
    // For now, just use the base image directly
    // TODO: Implement copy-on-write overlays for multi-user support
    return baseImagePath;
}

bool VMManager::startVM()
{
    if (isVMRunning()) {
        qDebug() << "VM is already running";
        return true;
    }
    
    if (!baseImageExists()) {
        emit vmError("Kali Linux base image not found at: " + baseImagePath);
        return false;
    }
    
    // Create QProcess for QEMU
    qemuProcess = new QProcess(this);
    
    // Connect signals
    connect(qemuProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        QString output = qemuProcess->readAllStandardOutput();
        emit vmOutputReady(output);
    });
    
    connect(qemuProcess, &QProcess::readyReadStandardError, this, [this]() {
        QString error = qemuProcess->readAllStandardError();
        emit vmOutputReady(error); // Show stderr as output too
    });
    
    connect(qemuProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        qDebug() << "QEMU process finished with code:" << exitCode;
        emit vmStopped();
    });
    
    // Build QEMU command
    QString qemuPath = "qemu-system-x86_64";
    QStringList arguments;
    arguments << "-m" << "2048"                                    // 2GB RAM
              << "-smp" << "2"                                      // 2 CPU cores
              << "-drive" << QString("file=%1,format=qcow2").arg(baseImagePath)
              << "-netdev" << "user,id=net0,hostfwd=tcp::2222-:22" // SSH port forwarding
              << "-device" << "virtio-net,netdev=net0"
              << "-nographic";                                      // Headless mode
    
    qDebug() << "Starting QEMU with command:" << qemuPath << arguments.join(" ");
    
    // Start the process
    qemuProcess->start(qemuPath, arguments);
    
    if (!qemuProcess->waitForStarted(5000)) {
        emit vmError("Failed to start QEMU: " + qemuProcess->errorString());
        delete qemuProcess;
        qemuProcess = nullptr;
        return false;
    }
    
    emit vmStarted();
    qDebug() << "VM started successfully";
    return true;
}

void VMManager::stopVM()
{
    if (qemuProcess && qemuProcess->state() == QProcess::Running) {
        qDebug() << "Stopping VM...";
        
        // Send quit command to QEMU monitor
        qemuProcess->write("quit\n");
        qemuProcess->waitForFinished(3000);
        
        if (qemuProcess->state() == QProcess::Running) {
            qemuProcess->kill();
        }
        
        delete qemuProcess;
        qemuProcess = nullptr;
        emit vmStopped();
    }
}

bool VMManager::isVMRunning() const
{
    return qemuProcess && qemuProcess->state() == QProcess::Running;
}
