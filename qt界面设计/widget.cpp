#include "widget.h"
#include "ui_widget.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QImage>
#include <QMessageBox>
#include <QDesktopServices>
#include <QPixmap>
#include <QStandardPaths>
#include <QTextCursor>
#include <QUrl>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTextStream>
#include <QTimer>
#ifdef Q_OS_WIN
#include <QSettings>
#endif

/* ---- Constants ---- */
#define MAX_RX_BUFFER 65536  /* max accumulated receive buffer (bytes) */
#define MAX_IMAGE_BYTES 65536
#define MAX_IMAGE_BASE64_BUFFER 90000

/* ---- Log file path (Documents/serial_debugger_log.txt) ---- */
static QString logFilePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
           + QStringLiteral("/serial_debugger_log.txt");
}

static QString bluetoothAddressFromInstanceName(const QString &instanceName)
{
    const int lastSeparator = instanceName.lastIndexOf(QLatin1Char('&'));
    QString address = lastSeparator >= 0 ? instanceName.mid(lastSeparator + 1) : instanceName;
    const int suffixStart = address.indexOf(QLatin1Char('_'));
    if (suffixStart > 0) {
        address = address.left(suffixStart);
    }
    return address.size() == 12 ? address.toUpper() : QString();
}

static QHash<QString, QString> windowsBluetoothPortLabels()
{
    QHash<QString, QString> labels;
#ifdef Q_OS_WIN
    const QString rootPath = QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Enum\\BTHENUM");
    const QStringList services = QSettings(rootPath, QSettings::NativeFormat).childGroups();
    QHash<QString, QString> deviceNames;

    for (const QString &service : services) {
        QSettings serviceKey(QStringLiteral("%1\\%2").arg(rootPath, service),
                             QSettings::NativeFormat);
        for (const QString &device : serviceKey.childGroups()) {
            const QString serviceAddress = service.startsWith(QStringLiteral("Dev_"), Qt::CaseInsensitive)
                ? service.mid(4).toUpper()
                : QString();
            if (!serviceAddress.isEmpty()) {
                QSettings deviceKey(QStringLiteral("%1\\%2\\%3").arg(rootPath, service, device),
                                    QSettings::NativeFormat);
                const QString friendlyName = deviceKey.value(QStringLiteral("FriendlyName")).toString().trimmed();
                if (!friendlyName.isEmpty()) {
                    deviceNames.insert(serviceAddress, friendlyName);
                }
            }
        }
    }

    for (const QString &service : services) {
        QSettings serviceKey(QStringLiteral("%1\\%2").arg(rootPath, service),
                             QSettings::NativeFormat);
        for (const QString &device : serviceKey.childGroups()) {
            QSettings paramsKey(QStringLiteral("%1\\%2\\%3\\Device Parameters").arg(rootPath, service, device),
                                QSettings::NativeFormat);
            const QString portName = paramsKey.value(QStringLiteral("PortName")).toString().trimmed();
            if (!portName.isEmpty()) {
                const QString address = bluetoothAddressFromInstanceName(device);
                if (!address.isEmpty() && address != QStringLiteral("000000000000")) {
                    labels.insert(portName.toUpper(), deviceNames.value(address));
                }
            }
        }
    }
#endif
    return labels;
}

static bool isBluetoothSerialPort(const QSerialPortInfo &port, const QHash<QString, QString> &windowsBtPorts)
{
#ifdef Q_OS_WIN
    if (!windowsBtPorts.isEmpty()) {
        return windowsBtPorts.contains(port.portName().toUpper());
    }
#endif
    const QString text = (port.description() + QLatin1Char(' ')
                          + port.manufacturer() + QLatin1Char(' ')
                          + port.portName() + QLatin1Char(' ')
                          + port.systemLocation()).toLower();
    return text.contains(QStringLiteral("bluetooth")) ||
           text.contains(QStringLiteral("serial over bluetooth")) ||
           text.contains(QStringLiteral("蓝牙"));
}

static void splitPorts(const QList<QSerialPortInfo> &allPorts,
                       QList<QSerialPortInfo> *usbPorts,
                       QList<QSerialPortInfo> *bluetoothPorts)
{
    const QHash<QString, QString> windowsBtPorts = windowsBluetoothPortLabels();
    for (const QSerialPortInfo &port : allPorts) {
        if (isBluetoothSerialPort(port, windowsBtPorts)) {
            bluetoothPorts->append(port);
        } else {
            usbPorts->append(port);
        }
    }
}

static QString portDisplayName(const QSerialPortInfo &port, bool bluetoothPort)
{
    if (bluetoothPort) {
        const QString label = windowsBluetoothPortLabels().value(port.portName().toUpper()).trimmed();
        if (!label.isEmpty()) {
            return QStringLiteral("%1 (%2 蓝牙串口)").arg(port.portName(), label);
        }
        return QStringLiteral("%1 (蓝牙串口)").arg(port.portName());
    }
    if (!port.description().isEmpty()) {
        return QStringLiteral("%1 (%2)").arg(port.portName(), port.description());
    }
    return port.portName();
}

static QString bluetoothPortDisplayName(const QString &portName, const QString &label)
{
    const QString trimmedLabel = label.trimmed();
    if (!trimmedLabel.isEmpty()) {
        return QStringLiteral("%1 (%2 蓝牙串口)").arg(portName, trimmedLabel);
    }
    return QStringLiteral("%1 (蓝牙串口)").arg(portName);
}

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , serialPort(new QSerialPort(this))
    , k210SerialPort(new QSerialPort(this))
    , recvTimeoutTimer(new QTimer(this))
    , autoSendTimer(new QTimer(this))
    , holdSendTimer(new QTimer(this))
    , logFile(nullptr)
    , rxBuffer()
    , k210RxBuffer()
    , waitingResponse(false)
    , rxBytes(0)
    , txBytes(0)
    , holdCommand()
    , holdStopCommand()
    , holdLabel()
    , imageBase64Buffer()
    , imageTransferActive(false)
    , expectedImageBytes(-1)
    , lastImage()
{
    ui->setupUi(this);
    ui->sendEdit->setPlaceholderText(QStringLiteral("请输入要发送的数据"));
    ui->recvEdit->setReadOnly(true);
    ui->baudCombo->setCurrentText(QStringLiteral("115200"));
    ui->timestampCheck->setChecked(true);
    ui->appendNewlineCheck->setChecked(true);
    ui->timeoutSpinBox->setValue(3000);
    ui->autoSendInterval->setValue(1000);

    recvTimeoutTimer->setSingleShot(true);
    recvTimeoutTimer->setInterval(ui->timeoutSpinBox->value());
    autoSendTimer->setInterval(ui->autoSendInterval->value());
    holdSendTimer->setInterval(150);

    updateByteCounts();

    ui->modeCombo->addItem(QStringLiteral("蓝牙模式"));
    ui->modeCombo->setCurrentIndex(0);
    ui->modeCombo->setEnabled(false);
    ui->modeCombo->setToolTip(QStringLiteral("当前版本只保留蓝牙模式"));

    ui->btForwardBtn->setToolTip(QStringLiteral("通过当前串口发送 F 命令到 STM32 执行前进"));
    ui->btBackwardBtn->setToolTip(QStringLiteral("通过串口发送 B 命令到 STM32 执行后退（H 桥差速转向）"));
    ui->btLeftBtn->setToolTip(QStringLiteral("通过当前串口发送 L 命令到 STM32 执行左转"));
    ui->btRightBtn->setToolTip(QStringLiteral("通过当前串口发送 R 命令到 STM32 执行右转"));
    ui->btStopBtn->setToolTip(QStringLiteral("通过当前串口发送 S 命令到 STM32 执行停止"));
    ui->motorSpeedSlider->setToolTip(QStringLiteral("设置电机 PWM 速度百分比"));
    ui->motorSpeedSpin->setToolTip(QStringLiteral("设置电机 PWM 速度百分比"));
    ui->motorSpeedBtn->setToolTip(QStringLiteral("发送 V0 到 V100 速度命令"));
    ui->trackOnBtn->setToolTip(QStringLiteral("发送 track 命令，进入循迹模式"));
    ui->trackOffBtn->setToolTip(QStringLiteral("发送 track_off 命令，退出循迹模式"));
    ui->k210PortCombo->setToolTip(QStringLiteral("K210 通过 USB 连接电脑后生成的串口"));
    ui->k210RefreshBtn->setToolTip(QStringLiteral("刷新电脑上的 K210 USB 串口列表"));
    ui->snapBtn->setToolTip(QStringLiteral("通过 K210 USB 串口发送 SNAP，触发 K210 拍照"));
    ui->continueBtn->setToolTip(QStringLiteral("发送 continue 命令，障碍确认后继续循迹"));
    ui->holdBtn->setToolTip(QStringLiteral("发送 hold 命令，让小车保持停止"));
    ui->statusBtn->setToolTip(QStringLiteral("发送 status 命令，查询当前模式、测距和循迹状态"));
    ui->imagePreviewLabel->setText(QStringLiteral("暂无图像"));
    ui->imagePreviewLabel->setAlignment(Qt::AlignCenter);
    resetDiagnostics();

    setupConnections();
    setConnectionState(false, QStringLiteral("未连接"));
    refreshPortList();
    refreshK210PortList();
    openK210PortIfAvailable();
}

Widget::~Widget()
{
    if (serialPort->isOpen()) {
        serialPort->close();
    }
    if (k210SerialPort->isOpen()) {
        k210SerialPort->close();
    }
    delete logFile;
    delete ui;
}

void Widget::setupConnections()
{
    connect(ui->openCloseBtn, &QPushButton::clicked, this, &Widget::onOpenOrCloseClicked);
    connect(ui->sendBtn, &QPushButton::clicked, this, &Widget::onSendClicked);
    connect(ui->clearRxBtn, &QPushButton::clicked, this, &Widget::onClearRxClicked);
    connect(ui->clearTxBtn, &QPushButton::clicked, this, &Widget::onClearTxClicked);
    connect(ui->openLogBtn, &QPushButton::clicked, this, &Widget::onOpenLogClicked);
    connect(ui->refreshBtn, &QPushButton::clicked, this, &Widget::refreshPortList);
    connect(ui->autoSendCheck, &QCheckBox::toggled, this, &Widget::onAutoSendToggled);
    connect(ui->autoSendInterval, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &Widget::onAutoSendIntervalChanged);
    connect(serialPort, &QSerialPort::readyRead, this, &Widget::onReadyRead);
    connect(serialPort, &QSerialPort::errorOccurred, this, &Widget::onSerialError);
    connect(k210SerialPort, &QSerialPort::readyRead, this, &Widget::onK210ReadyRead);
    connect(k210SerialPort, &QSerialPort::errorOccurred, this, &Widget::onK210SerialError);
    connect(recvTimeoutTimer, &QTimer::timeout, this, &Widget::onReceiveTimeout);
    connect(autoSendTimer, &QTimer::timeout, this, &Widget::onSendClicked);
    connect(holdSendTimer, &QTimer::timeout, this, &Widget::onHoldSendTimeout);
    connect(ui->btForwardBtn, &QPushButton::pressed, this, &Widget::onBtForwardClicked);
    connect(ui->btForwardBtn, &QPushButton::released, this, &Widget::stopHoldCommand);
    connect(ui->btBackwardBtn, &QPushButton::pressed, this, &Widget::onBtBackwardClicked);
    connect(ui->btBackwardBtn, &QPushButton::released, this, &Widget::stopHoldCommand);
    connect(ui->btLeftBtn, &QPushButton::pressed, this, &Widget::onBtLeftClicked);
    connect(ui->btLeftBtn, &QPushButton::released, this, &Widget::stopHoldCommand);
    connect(ui->btRightBtn, &QPushButton::pressed, this, &Widget::onBtRightClicked);
    connect(ui->btRightBtn, &QPushButton::released, this, &Widget::stopHoldCommand);
    connect(ui->btStopBtn, &QPushButton::clicked, this, &Widget::onBtStopClicked);
    connect(ui->motorSpeedSlider, &QSlider::valueChanged, ui->motorSpeedSpin, &QSpinBox::setValue);
    connect(ui->motorSpeedSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            ui->motorSpeedSlider, &QSlider::setValue);
    connect(ui->motorSpeedBtn, &QPushButton::clicked, this, &Widget::onMotorSpeedClicked);
    connect(ui->trackOnBtn, &QPushButton::clicked, this, &Widget::onTrackOnClicked);
    connect(ui->trackOffBtn, &QPushButton::clicked, this, &Widget::onTrackOffClicked);
    connect(ui->snapBtn, &QPushButton::clicked, this, &Widget::onSnapClicked);
    connect(ui->continueBtn, &QPushButton::clicked, this, &Widget::onContinueClicked);
    connect(ui->holdBtn, &QPushButton::clicked, this, &Widget::onHoldClicked);
    connect(ui->statusBtn, &QPushButton::clicked, this, &Widget::onStatusClicked);
    connect(ui->k210RefreshBtn, &QPushButton::clicked, this, &Widget::refreshK210PortList);
}

void Widget::refreshPortList()
{
    const QString currentName = ui->portCombo->currentData().toString();
    const QList<QSerialPortInfo> allPorts = QSerialPortInfo::availablePorts();
    QList<QSerialPortInfo> btPorts;
    QStringList listedPorts;
    QList<QSerialPortInfo> usbPorts;
    splitPorts(allPorts, &usbPorts, &btPorts);

    ui->portCombo->clear();

    for (const QSerialPortInfo &port : btPorts) {
        const QString display = portDisplayName(port, true);
        ui->portCombo->addItem(display, port.portName());
        listedPorts.append(port.portName().toUpper());
    }

#ifdef Q_OS_WIN
    {
        const QHash<QString, QString> windowsBtPorts = windowsBluetoothPortLabels();
        QStringList portNames = windowsBtPorts.keys();
        portNames.sort(Qt::CaseInsensitive);
        for (const QString &portNameUpper : portNames) {
            if (listedPorts.contains(portNameUpper)) {
                continue;
            }
            const QString portName = portNameUpper;
            ui->portCombo->addItem(bluetoothPortDisplayName(portName, windowsBtPorts.value(portNameUpper)),
                                   portName);
            listedPorts.append(portNameUpper);
        }
    }
#endif

    const int index = ui->portCombo->findData(currentName);
    if (index >= 0) {
        ui->portCombo->setCurrentIndex(index);
    } else if (ui->portCombo->count() > 0) {
        ui->portCombo->setCurrentIndex(0);
    } else {
        const int hc553Index = ui->portCombo->findText(QStringLiteral("HC-553"), Qt::MatchContains);
        if (hc553Index >= 0) {
            ui->portCombo->setCurrentIndex(hc553Index);
        }
    }
}

void Widget::refreshK210PortList()
{
    const QString currentName = ui->k210PortCombo->currentData().toString();
    const QList<QSerialPortInfo> allPorts = QSerialPortInfo::availablePorts();
    QList<QSerialPortInfo> btPorts;
    QList<QSerialPortInfo> usbPorts;
    splitPorts(allPorts, &usbPorts, &btPorts);

    ui->k210PortCombo->clear();
    for (const QSerialPortInfo &port : usbPorts) {
        ui->k210PortCombo->addItem(portDisplayName(port, false), port.portName());
    }

    const int index = ui->k210PortCombo->findData(currentName);
    if (index >= 0) {
        ui->k210PortCombo->setCurrentIndex(index);
    }

    openK210PortIfAvailable();
}

void Widget::openK210PortIfAvailable()
{
    if (k210SerialPort->isOpen()) {
        return;
    }
    if (ui->k210PortCombo->currentData().toString().isEmpty()) {
        updateDiagnosticLabel(ui->k210ValueLabel, QStringLiteral("USB未连接"));
        return;
    }

    k210SerialPort->setPortName(ui->k210PortCombo->currentData().toString());
    k210SerialPort->setBaudRate(QSerialPort::Baud115200);
    k210SerialPort->setDataBits(QSerialPort::Data8);
    k210SerialPort->setParity(QSerialPort::NoParity);
    k210SerialPort->setStopBits(QSerialPort::OneStop);
    k210SerialPort->setFlowControl(QSerialPort::NoFlowControl);
    if (!k210SerialPort->open(QIODevice::ReadWrite)) {
        updateDiagnosticLabel(ui->k210ValueLabel, QStringLiteral("USB打开失败"));
        return;
    }

    k210SerialPort->clearError();
    ui->k210PortCombo->setEnabled(false);
    updateDiagnosticLabel(ui->k210ValueLabel,
                          QStringLiteral("USB已连接 %1").arg(k210SerialPort->portName()));
}


QByteArray Widget::parseHexString(const QString &input, bool *ok) const
{
    QByteArray result;
    QString normalized = input;
    normalized.replace('\n', ' ');
    normalized.replace('\r', ' ');
    normalized = normalized.simplified();
    if (normalized.isEmpty()) {
        *ok = false;
        return result;
    }

    const QStringList parts = normalized.split(' ', Qt::SkipEmptyParts);
    for (const QString &part : parts) {
        bool success = false;
        QString hex = part;
        if (hex.startsWith(QLatin1String("0x"), Qt::CaseInsensitive) ||
            hex.startsWith(QLatin1String("0X"), Qt::CaseInsensitive)) {
            hex = hex.mid(2);
        }
        if (hex.isEmpty()) {
            *ok = false;
            return QByteArray();
        }
        const int value = hex.toInt(&success, 16);
        if (!success || value < 0x00 || value > 0xFF) {
            *ok = false;
            return QByteArray();
        }
        result.append(static_cast<char>(value));
    }
    *ok = true;
    return result;
}

QString Widget::bytesToHexString(const QByteArray &data) const
{
    QStringList items;
    items.reserve(data.size());
    for (unsigned char byte : data) {
        items.append(QString("%1").arg(byte, 2, 16, QLatin1Char('0')).toUpper());
    }
    return items.join(' ');
}

void Widget::saveRxLog(const QString &line)
{
    if (!ui->autoSaveLogCheck->isChecked()) {
        return;
    }
    if (logFile == nullptr) {
        logFile = new QFile(logFilePath(), this);
    }
    if (!logFile->isOpen() && !logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QMessageBox::warning(this, QStringLiteral("日志错误"), QStringLiteral("无法打开日志文件。"));
        return;
    }

    QTextStream stream(logFile);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    stream.setCodec("UTF-8");
#endif
    stream << line << '\n';
    stream.flush();
}

void Widget::setConnectionState(bool connected, const QString &message)
{
    ui->statusLabel->setText(QStringLiteral("状态：%1").arg(message));
    ui->openCloseBtn->setText(connected ? QStringLiteral("关闭串口") : QStringLiteral("打开串口"));

    ui->portCombo->setEnabled(!connected);
    ui->baudCombo->setEnabled(!connected);
    ui->stopBitsCombo->setEnabled(!connected);

    if (!connected) {
        autoSendTimer->stop();
        holdSendTimer->stop();
        holdCommand.clear();
        holdStopCommand.clear();
        holdLabel.clear();
        ui->autoSendCheck->setChecked(false);
        ui->autoSendCheck->setEnabled(false);
        resetDiagnostics();
    } else {
        ui->autoSendCheck->setEnabled(true);
    }

    /* 控制按钮随连接状态启用/禁用 */
    ui->btForwardBtn->setEnabled(connected);
    ui->btBackwardBtn->setEnabled(connected);
    ui->btLeftBtn->setEnabled(connected);
    ui->btRightBtn->setEnabled(connected);
    ui->btStopBtn->setEnabled(connected);
    ui->motorSpeedSlider->setEnabled(connected);
    ui->motorSpeedSpin->setEnabled(connected);
    ui->motorSpeedBtn->setEnabled(connected);
    ui->trackOnBtn->setEnabled(connected);
    ui->trackOffBtn->setEnabled(connected);
    ui->snapBtn->setEnabled(connected || k210SerialPort->isOpen());
    ui->continueBtn->setEnabled(connected);
    ui->holdBtn->setEnabled(connected);
    ui->statusBtn->setEnabled(connected);
}

void Widget::updateByteCounts()
{
    ui->byteCountLabel->setText(QStringLiteral("RX: %1  TX: %2").arg(rxBytes).arg(txBytes));
}

void Widget::flushRxBuffer()
{
    /* Data is displayed as soon as it arrives; this buffer is only for parsing
       line-based status responses such as OK:/Error:. */
    rxBuffer.clear();
}

void Widget::resetDiagnostics()
{
    updateDiagnosticLabel(ui->modeValueLabel, QStringLiteral("--"));
    updateDiagnosticLabel(ui->speedValueLabel, QStringLiteral("--"));
    updateDiagnosticLabel(ui->distanceValueLabel, QStringLiteral("--"));
    updateDiagnosticLabel(ui->trackValueLabel, QStringLiteral("--"));
    updateDiagnosticLabel(ui->k210ValueLabel, QStringLiteral("--"));
}

void Widget::updateDiagnosticLabel(QLabel *label, const QString &value)
{
    if (label == nullptr) {
        return;
    }
    label->setText(value);
}

void Widget::applyReportedSpeed(const QString &valueText)
{
    bool ok = false;
    const int speed = valueText.toInt(&ok);
    if (!ok) {
        return;
    }
    if (speed < ui->motorSpeedSpin->minimum() || speed > ui->motorSpeedSpin->maximum()) {
        return;
    }
    ui->motorSpeedSpin->setValue(speed);
}

bool Widget::applySpeedFromLine(const QByteArray &line, const QByteArray &prefix)
{
    if (!line.startsWith(prefix)) {
        return false;
    }
    const QString value = QString::fromUtf8(line.mid(prefix.size())).trimmed();
    if (value.isEmpty()) {
        return false;
    }
    applyReportedSpeed(value);
    updateDiagnosticLabel(ui->speedValueLabel, value + QStringLiteral("%"));
    return true;
}

void Widget::resetImageTransfer()
{
    imageBase64Buffer.clear();
    k210RxBuffer.clear();
    imageTransferActive = false;
    expectedImageBytes = -1;
}

void Widget::appendImageChunk(const QByteArray &chunk)
{
    imageBase64Buffer.append(chunk.trimmed());
    if (imageBase64Buffer.size() > MAX_IMAGE_BASE64_BUFFER) {
        setConnectionState(true, QStringLiteral("图像数据过大，已中止"));
        resetImageTransfer();
    }
}

bool Widget::shouldDisplayIncomingLine(const QByteArray &line) const
{
    return !line.startsWith("IMG:DATA:");
}

void Widget::finalizeImageTransfer()
{
    const QByteArray imageBytes = QByteArray::fromBase64(imageBase64Buffer);
    QPixmap pixmap;

    if (imageBytes.isEmpty() || !pixmap.loadFromData(imageBytes)) {
        setConnectionState(true, QStringLiteral("图像解码失败"));
        resetImageTransfer();
        return;
    }

    lastImage = pixmap;
    updateImagePreview(lastImage);
    const QString savedPath = saveImageToDisk(imageBytes);
    const QString sizeStatus =
        (expectedImageBytes >= 0 && expectedImageBytes != imageBytes.size())
            ? QStringLiteral("，声明 %1 字节").arg(expectedImageBytes)
            : QString();
    if (savedPath.isEmpty()) {
        setConnectionState(true, QStringLiteral("图像接收完成 %1 字节%2，保存失败")
                                 .arg(imageBytes.size())
                                 .arg(sizeStatus));
    } else {
        setConnectionState(true, QStringLiteral("图像接收完成 %1 字节%2，已保存 %3")
                                 .arg(imageBytes.size())
                                 .arg(sizeStatus, QFileInfo(savedPath).fileName()));
        saveRxLog(QStringLiteral("IMAGE:%1").arg(savedPath));
    }
    resetImageTransfer();
}

void Widget::updateImagePreview(const QPixmap &pixmap)
{
    if (pixmap.isNull()) {
        ui->imagePreviewLabel->setPixmap(QPixmap());
        ui->imagePreviewLabel->setText(QStringLiteral("暂无图像"));
        return;
    }

    ui->imagePreviewLabel->setText(QString());
    ui->imagePreviewLabel->setPixmap(pixmap.scaled(ui->imagePreviewLabel->size(),
                                                   Qt::KeepAspectRatio,
                                                   Qt::SmoothTransformation));
}

QString Widget::imageSaveDirectory() const
{
    const QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    return desktopPath + QStringLiteral("/K210Images/");
}

QString Widget::saveImageToDisk(const QByteArray &imageBytes)
{
    const QString directoryPath = imageSaveDirectory();
    QDir dir;
    if (!dir.mkpath(directoryPath)) {
        return QString();
    }

    const QString fileName =
        QDateTime::currentDateTime().toString(QStringLiteral("HH-mm-ss-zzz")) + QStringLiteral(".jpg");
    const QString filePath = directoryPath + QLatin1Char('/') + fileName;
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return QString();
    }
    if (file.write(imageBytes) != imageBytes.size()) {
        file.close();
        file.remove();
        return QString();
    }
    file.close();
    return filePath;
}

void Widget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (!lastImage.isNull()) {
        updateImagePreview(lastImage);
    }
}

void Widget::applyPortSettings()
{
    serialPort->setBaudRate(ui->baudCombo->currentText().toInt());
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setStopBits(ui->stopBitsCombo->currentText() == QStringLiteral("2")
                                ? QSerialPort::TwoStop
                                : QSerialPort::OneStop);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);
}

void Widget::applyBluetoothDefaults()
{
    ui->baudCombo->setCurrentText(QStringLiteral("115200"));
    ui->stopBitsCombo->setCurrentText(QStringLiteral("1"));
}

void Widget::onOpenOrCloseClicked()
{
    if (serialPort->isOpen()) {
        stopHoldCommand();
        serialPort->close();
        recvTimeoutTimer->stop();
        autoSendTimer->stop();
        ui->autoSendCheck->setChecked(false);
        flushRxBuffer();
        if (logFile && logFile->isOpen()) {
            logFile->close();
        }
        setConnectionState(false, QStringLiteral("已关闭"));
        return;
    }

    if (ui->portCombo->currentData().toString().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("串口错误"), QStringLiteral("未检测到可用串口。"));
        return;
    }

    applyBluetoothDefaults();

    serialPort->setPortName(ui->portCombo->currentData().toString());
    applyPortSettings();

    if (!serialPort->open(QIODevice::ReadWrite)) {
        QMessageBox::critical(this, QStringLiteral("打开失败"), serialPort->errorString());
        setConnectionState(false, QStringLiteral("打开失败"));
        return;
    }

    serialPort->clearError();

    setConnectionState(true, QStringLiteral("已连接 %1").arg(serialPort->portName()));
    sendLineCommand("status", QStringLiteral("Status"));
}

void Widget::onSendClicked()
{
    if (!serialPort->isOpen()) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先打开串口。"));
        return;
    }

    QByteArray payload;
    if (ui->hexSendCheck->isChecked()) {
        bool ok = false;
        payload = parseHexString(ui->sendEdit->toPlainText(), &ok);
        if (!ok) {
            if (autoSendTimer->isActive()) {
                /* 定时发送模式下不弹窗，静默跳过 */
                setConnectionState(true, QStringLiteral("Hex 格式错误，已跳过"));
                return;
            }
            QMessageBox::warning(this, QStringLiteral("Hex 格式错误"), QStringLiteral("请输入以空格分隔的十六进制字节，例如：48 65 6C 6C 6F"));
            return;
        }
    } else {
        payload = ui->sendEdit->toPlainText().toUtf8();
        if (ui->appendNewlineCheck->isChecked()) {
            payload.append("\r\n");
        }
    }

    if (payload.isEmpty()) {
        return;
    }

    const qint64 written = serialPort->write(payload);
    if (written < 0) {
        QMessageBox::warning(this, QStringLiteral("发送失败"), serialPort->errorString());
        return;
    }
    if (written == 0) {
        QMessageBox::warning(this, QStringLiteral("发送失败"), QStringLiteral("发送缓冲区已满，数据被丢弃。"));
        return;
    }

    txBytes += written;
    updateByteCounts();
    serialPort->flush();
    const bool flushed = serialPort->waitForBytesWritten(300);
    setConnectionState(true, QStringLiteral("已发送 %1/%2 字节 %3 [%4]")
                             .arg(written)
                             .arg(payload.size())
                             .arg(flushed ? QStringLiteral("已写出") : QStringLiteral("等待写出超时"))
                             .arg(bytesToHexString(payload)));

    if (!ui->autoSendCheck->isChecked()) {
        const int timeoutMs = ui->timeoutSpinBox->value();
        waitingResponse = timeoutMs > 0;
        if (waitingResponse) {
            recvTimeoutTimer->start(timeoutMs);
        } else {
            recvTimeoutTimer->stop();
        }
    }
}

void Widget::onClearRxClicked()
{
    if (ui->recvEdit->toPlainText().isEmpty()) {
        return;
    }
    const auto reply = QMessageBox::question(this, QStringLiteral("确认清空"),
                                              QStringLiteral("确定要清空接收区吗？"),
                                              QMessageBox::Yes | QMessageBox::No,
                                              QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        return;
    }
    ui->recvEdit->clear();
}

void Widget::onClearTxClicked()
{
    if (ui->sendEdit->toPlainText().isEmpty()) {
        return;
    }
    const auto reply = QMessageBox::question(this, QStringLiteral("确认清空"),
                                              QStringLiteral("确定要清空发送区吗？"),
                                              QMessageBox::Yes | QMessageBox::No,
                                              QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        return;
    }
    ui->sendEdit->clear();
}

void Widget::appendReceivedLine(const QByteArray &line)
{
    if (ui->hexDisplayCheck->isChecked() || !shouldDisplayIncomingLine(line)) {
        return;
    }

    QString display = QString::fromUtf8(line);
    if (ui->timestampCheck->isChecked()) {
        display = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz ") + display;
    }
    ui->recvEdit->appendPlainText(display);
    saveRxLog(display);
}

void Widget::handleIncomingLine(const QByteArray &line, bool fromK210)
{
    appendReceivedLine(line);

    if (line.startsWith("OK:")) {
        applySpeedFromLine(line, "OK: V");
        setConnectionState(true, QString::fromUtf8(line));
    } else if (line.startsWith("Error:")) {
        setConnectionState(true, QStringLiteral("错误: %1").arg(QString::fromUtf8(line.mid(7).trimmed())));
    } else if (line.startsWith("IMG:BEGIN:")) {
        bool ok = false;
        expectedImageBytes = QString::fromUtf8(line.mid(10)).trimmed().toInt(&ok);
        if (ok && expectedImageBytes > MAX_IMAGE_BYTES) {
            resetImageTransfer();
            setConnectionState(true, QStringLiteral("图像声明过大，已忽略 %1 字节").arg(expectedImageBytes));
        } else {
            imageTransferActive = true;
            imageBase64Buffer.clear();
            setConnectionState(true, ok
                ? QStringLiteral("开始接收图像，预计 %1 字节").arg(expectedImageBytes)
                : QStringLiteral("开始接收图像"));
        }
    } else if (line.startsWith("IMG:DATA:")) {
        if (imageTransferActive) {
            appendImageChunk(line.mid(9));
        }
    } else if (line.startsWith("IMG:END")) {
        if (imageTransferActive) {
            finalizeImageTransfer();
        }
    } else if (line.startsWith("BT:OK:")) {
        applySpeedFromLine(line, "BT:OK:V");
        setConnectionState(true, QString::fromUtf8(line));
    } else if (line.startsWith("BT:ERR:")) {
        setConnectionState(true, QStringLiteral("蓝牙错误: %1").arg(QString::fromUtf8(line.mid(7).trimmed())));
    } else if (line.startsWith("BT:RX:")) {
        setConnectionState(true, QStringLiteral("蓝牙收到字节: %1").arg(QString::fromUtf8(line.mid(6).trimmed())));
    } else if (line.startsWith("PC:")) {
        applySpeedFromLine(line, "PC:V");
        setConnectionState(true, QStringLiteral("蓝牙收到转发: %1").arg(QString::fromUtf8(line.mid(3).trimmed())));
    } else if (line.startsWith("MODE:")) {
        const QString value = QString::fromUtf8(line.mid(5).trimmed());
        updateDiagnosticLabel(ui->modeValueLabel, value);
        setConnectionState(true, QString::fromUtf8(line));
    } else if (line.startsWith("OBS:")) {
        const QString value = QString::fromUtf8(line.mid(4).trimmed());
        setConnectionState(true, QStringLiteral("检测到障碍: %1").arg(value));
        triggerK210Snapshot(QStringLiteral("障碍触发"));
    } else if (line.startsWith("DIST:")) {
        const QString value = QString::fromUtf8(line.mid(5).trimmed());
        updateDiagnosticLabel(ui->distanceValueLabel, value);
        setConnectionState(true, QStringLiteral("测距: %1").arg(value));
    } else if (line.startsWith("TRACK:")) {
        const QString value = QString::fromUtf8(line.mid(6).trimmed());
        updateDiagnosticLabel(ui->trackValueLabel, value);
        setConnectionState(true, QStringLiteral("循迹状态: %1").arg(value));
    } else if (line.startsWith("SPEED:")) {
        const QString value = QString::fromUtf8(line.mid(6).trimmed());
        applyReportedSpeed(value);
        updateDiagnosticLabel(ui->speedValueLabel, value + QStringLiteral("%"));
        setConnectionState(true, QStringLiteral("当前速度: %1%").arg(value));
    } else if (line.startsWith("BT_RX_OVF:")) {
        const QString value = QString::fromUtf8(line.mid(10).trimmed());
        setConnectionState(true, QStringLiteral("蓝牙接收溢出: %1").arg(value));
    } else if (line.startsWith("K210:")) {
        const QString value = QString::fromUtf8(line.mid(5).trimmed());
        updateDiagnosticLabel(ui->k210ValueLabel, value);
        setConnectionState(true, fromK210
            ? QStringLiteral("K210 USB: %1").arg(value)
            : QString::fromUtf8(line));
    }
}

void Widget::onReadyRead()
{
    const QByteArray data = serialPort->readAll();
    if (data.isEmpty()) {
        return;
    }
    waitingResponse = false;
    recvTimeoutTimer->stop();

    rxBuffer.append(data);
    /* Enforce max buffer size to prevent unbounded growth */
    if (rxBuffer.size() > MAX_RX_BUFFER) {
        rxBuffer.remove(0, rxBuffer.size() - MAX_RX_BUFFER);
    }
    rxBytes += data.size();
    updateByteCounts();

    if (ui->hexDisplayCheck->isChecked()) {
        QString display = bytesToHexString(data);
        if (!display.isEmpty()) {
            if (ui->timestampCheck->isChecked()) {
                display = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz ") + display;
            }
            ui->recvEdit->appendPlainText(display);
            saveRxLog(display);
        }
    }

    /* Parse complete status lines without delaying the receive display.
       IMG:DATA payload lines are parsed but not displayed/logged, otherwise
       a single image transfer can flood the UI and slow the serial reader. */
    int idx;
    while ((idx = rxBuffer.indexOf('\n')) >= 0) {
        QByteArray line = rxBuffer.left(idx);
        rxBuffer.remove(0, idx + 1);

        /* Strip trailing \r for clean display */
        if (line.endsWith('\r')) {
            line.chop(1);
        }

        handleIncomingLine(line, false);
    }
}

void Widget::onSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError) {
        return;
    }

    QString errorMsg;
    switch (error) {
    case QSerialPort::ResourceError:
    case QSerialPort::DeviceNotFoundError:
        errorMsg = QStringLiteral("串口可能已断开");
        break;
    case QSerialPort::PermissionError:
        errorMsg = QStringLiteral("串口权限不足，可能被其他程序占用");
        break;
    case QSerialPort::OpenError:
        errorMsg = QStringLiteral("串口打开失败");
        break;
    case QSerialPort::TimeoutError:
        serialPort->clearError();
        return;
    case QSerialPort::UnsupportedOperationError:
        if (serialPort->isOpen()) {
            serialPort->clearError();
            ui->statusLabel->setText(QStringLiteral("状态：已连接 %1（忽略不支持的串口控制项）").arg(serialPort->portName()));
            return;
        }
        errorMsg = QStringLiteral("串口不支持当前控制项");
        break;
    default:
        errorMsg = QStringLiteral("串口发生错误");
        break;
    }

    QMessageBox::warning(this, QStringLiteral("串口异常"),
                         QStringLiteral("%1：%2").arg(errorMsg, serialPort->errorString()));

    if (error == QSerialPort::ResourceError ||
        error == QSerialPort::DeviceNotFoundError ||
        error == QSerialPort::PermissionError) {
        serialPort->blockSignals(true);
        serialPort->close();
        serialPort->blockSignals(false);
        recvTimeoutTimer->stop();
        autoSendTimer->stop();
        holdSendTimer->stop();
        holdCommand.clear();
        holdStopCommand.clear();
        holdLabel.clear();
        ui->autoSendCheck->setChecked(false);
        flushRxBuffer();
        if (logFile && logFile->isOpen()) {
            logFile->close();
        }
        setConnectionState(false, QStringLiteral("串口断开"));
    }
}

void Widget::triggerK210Snapshot(const QString &reason)
{
    if (!k210SerialPort->isOpen()) {
        updateDiagnosticLabel(ui->k210ValueLabel,
                              QStringLiteral("%1: K210 USB未打开").arg(reason));
        return;
    }

    const QByteArray payload("SNAP\r\n");
    const qint64 written = k210SerialPort->write(payload);
    if (written <= 0) {
        updateDiagnosticLabel(ui->k210ValueLabel,
                              QStringLiteral("%1: SNAP发送失败").arg(reason));
        return;
    }

    k210SerialPort->flush();
    k210SerialPort->waitForBytesWritten(300);
    updateDiagnosticLabel(ui->k210ValueLabel,
                          QStringLiteral("%1: 已发送SNAP到%2")
                              .arg(reason, k210SerialPort->portName()));
}

void Widget::onK210ReadyRead()
{
    const QByteArray data = k210SerialPort->readAll();
    if (data.isEmpty()) {
        return;
    }

    k210RxBuffer.append(data);
    if (k210RxBuffer.size() > MAX_RX_BUFFER) {
        k210RxBuffer.remove(0, k210RxBuffer.size() - MAX_RX_BUFFER);
    }

    int idx;
    while ((idx = k210RxBuffer.indexOf('\n')) >= 0) {
        QByteArray line = k210RxBuffer.left(idx);
        k210RxBuffer.remove(0, idx + 1);
        if (line.endsWith('\r')) {
            line.chop(1);
        }
        handleIncomingLine(line, true);
    }
}

void Widget::onK210SerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError || error == QSerialPort::TimeoutError) {
        if (error == QSerialPort::TimeoutError) {
            k210SerialPort->clearError();
        }
        return;
    }

    if (error == QSerialPort::ResourceError ||
        error == QSerialPort::DeviceNotFoundError ||
        error == QSerialPort::PermissionError) {
        updateDiagnosticLabel(ui->k210ValueLabel,
                              QStringLiteral("K210 USB断开: %1").arg(k210SerialPort->errorString()));
        k210SerialPort->blockSignals(true);
        k210SerialPort->close();
        k210SerialPort->blockSignals(false);
        k210RxBuffer.clear();
        ui->snapBtn->setEnabled(serialPort->isOpen());
        ui->k210PortCombo->setEnabled(true);
    }
}

void Widget::onReceiveTimeout()
{
    if (!waitingResponse) {
        return;
    }
    waitingResponse = false;
    setConnectionState(true, QStringLiteral("接收超时"));
}

void Widget::onOpenLogClicked()
{
    const QString logPath = logFilePath();
    if (!QFile::exists(logPath)) {
        QMessageBox::information(this, QStringLiteral("日志不存在"), QStringLiteral("当前还没有日志文件，请先勾选\"自动保存日志\"并接收数据。"));
        return;
    }

    const bool opened = QDesktopServices::openUrl(QUrl::fromLocalFile(logPath));
    if (!opened) {
        QMessageBox::warning(this, QStringLiteral("打开失败"), QStringLiteral("无法打开日志文件：%1").arg(logPath));
    }
}

void Widget::onAutoSendToggled(bool checked)
{
    if (checked) {
        if (!serialPort->isOpen()) {
            ui->autoSendCheck->setChecked(false);
            QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先打开串口。"));
            return;
        }
        autoSendTimer->start(ui->autoSendInterval->value());
        /* 仅更新标签，不调用 setConnectionState() 避免禁用配置控件 */
        ui->statusLabel->setText(QStringLiteral("状态：定时发送已启动"));
    } else {
        autoSendTimer->stop();
        /* 恢复连接状态显示 */
        if (serialPort->isOpen()) {
            ui->statusLabel->setText(QStringLiteral("状态：已连接 %1（定时发送已停止）").arg(serialPort->portName()));
        } else {
            ui->statusLabel->setText(QStringLiteral("状态：未连接"));
        }
    }
}

void Widget::onAutoSendIntervalChanged(int value)
{
    if (autoSendTimer->isActive()) {
        autoSendTimer->start(value);
    }
}

void Widget::startHoldCommand(const QByteArray &cmd, const QByteArray &stopCmd, const QString &label)
{
    if (!serialPort->isOpen()) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先打开串口。"));
        return;
    }

    holdCommand = cmd;
    holdStopCommand = stopCmd;
    holdLabel = label;
    sendRawDirectionCmd(holdCommand, holdLabel);
    if (serialPort->isOpen()) {
        holdSendTimer->start();
    }
}

void Widget::stopHoldCommand()
{
    const bool wasActive = holdSendTimer->isActive();
    holdSendTimer->stop();
    if (wasActive && serialPort->isOpen() && !holdStopCommand.isEmpty()) {
        sendRawDirectionCmd(holdStopCommand, QStringLiteral("%1 stop").arg(holdLabel));
    }
    holdCommand.clear();
    holdStopCommand.clear();
    holdLabel.clear();
}

void Widget::onHoldSendTimeout()
{
    if (holdCommand.isEmpty()) {
        holdSendTimer->stop();
        return;
    }
    sendRawDirectionCmd(holdCommand, holdLabel);
}

/* ---- Bluetooth (CH-553) command helpers ---- */

void Widget::sendSerialPayload(const QByteArray &payload, const QString &label, bool expectResponse)
{
    if (!serialPort->isOpen()) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先打开串口。"));
        return;
    }
    const qint64 written = serialPort->write(payload);
    if (written < 0) {
        QMessageBox::warning(this, QStringLiteral("发送失败"), serialPort->errorString());
        return;
    }
    if (written == 0) {
        setConnectionState(true, QStringLiteral("发送失败: 写入 0 字节"));
        return;
    }
    txBytes += written;
    updateByteCounts();
    serialPort->flush();
    const bool flushed = serialPort->waitForBytesWritten(300);
    setConnectionState(true, QStringLiteral("已发送: %1 %2/%3 %4 [%5]")
                             .arg(label)
                             .arg(written)
                             .arg(payload.size())
                             .arg(flushed ? QStringLiteral("已写出") : QStringLiteral("等待写出超时"))
                             .arg(bytesToHexString(payload)));

    if (expectResponse) {
        const int timeoutMs = ui->timeoutSpinBox->value();
        waitingResponse = timeoutMs > 0;
        if (waitingResponse) {
            recvTimeoutTimer->start(timeoutMs);
        }
    }
}

void Widget::sendRawDirectionCmd(const QByteArray &cmd, const QString &label)
{
    sendSerialPayload(cmd, label, false);
}

void Widget::sendLineCommand(const QByteArray &cmd, const QString &label)
{
    sendSerialPayload(cmd + "\r\n", label);
}

void Widget::onBtForwardClicked()
{
    startHoldCommand("F", "S", "Forward");
}

void Widget::onBtBackwardClicked()
{
    startHoldCommand("B", "S", "Backward");
}

void Widget::onBtLeftClicked()
{
    startHoldCommand("L", "S", "Left");
}

void Widget::onBtRightClicked()
{
    startHoldCommand("R", "S", "Right");
}

void Widget::onBtStopClicked()
{
    stopHoldCommand();
    sendRawDirectionCmd("S", "Stop");
}

void Widget::onMotorSpeedClicked()
{
    const int speed = ui->motorSpeedSpin->value();
    sendLineCommand(QByteArray("V") + QByteArray::number(speed),
                    QStringLiteral("Speed %1%").arg(speed));
}

void Widget::onTrackOnClicked()
{
    stopHoldCommand();
    sendLineCommand("track", QStringLiteral("Track on"));
}

void Widget::onTrackOffClicked()
{
    stopHoldCommand();
    sendLineCommand("track_off", QStringLiteral("Track off"));
}

void Widget::onSnapClicked()
{
    stopHoldCommand();
    triggerK210Snapshot(QStringLiteral("手动拍照"));
}

void Widget::onContinueClicked()
{
    stopHoldCommand();
    sendLineCommand("continue", QStringLiteral("Continue"));
}

void Widget::onHoldClicked()
{
    stopHoldCommand();
    sendLineCommand("hold", QStringLiteral("Hold"));
}

void Widget::onStatusClicked()
{
    stopHoldCommand();
    sendLineCommand("status", QStringLiteral("Status"));
}
