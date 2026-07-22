#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QByteArray>
#include <QPixmap>
#include <QSerialPort>

QT_BEGIN_NAMESPACE
class QLabel;
class QTimer;
class QFile;
namespace Ui { class Widget; }
QT_END_NAMESPACE



class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void startHoldCommand(const QByteArray &cmd, const QByteArray &stopCmd, const QString &label);
    void stopHoldCommand();
    void setupConnections();
    void refreshPortList();
    void refreshK210PortList();
    void openK210PortIfAvailable();
    QByteArray parseHexString(const QString &input, bool *ok) const;
    QString bytesToHexString(const QByteArray &data) const;
    void saveRxLog(const QString &line);
    void setConnectionState(bool connected, const QString &message);
    void updateByteCounts();
    void applyBluetoothDefaults();
    void applyPortSettings();
    void flushRxBuffer();
    void resetDiagnostics();
    void updateDiagnosticLabel(QLabel *label, const QString &value);
    void applyReportedSpeed(const QString &valueText);
    bool applySpeedFromLine(const QByteArray &line, const QByteArray &prefix);
    void sendSerialPayload(const QByteArray &payload, const QString &label, bool expectResponse = true);
    void sendRawDirectionCmd(const QByteArray &cmd, const QString &label);
    void sendLineCommand(const QByteArray &cmd, const QString &label);
    void triggerK210Snapshot(const QString &reason);
    void appendReceivedLine(const QByteArray &line);
    void handleIncomingLine(const QByteArray &line, bool fromK210);
    void resetImageTransfer();
    void appendImageChunk(const QByteArray &chunk);
    void finalizeImageTransfer();
    bool shouldDisplayIncomingLine(const QByteArray &line) const;
    void updateImagePreview(const QPixmap &pixmap);
    QString imageSaveDirectory() const;
    QString saveImageToDisk(const QByteArray &imageBytes);

private slots:
    void onOpenOrCloseClicked();
    void onSendClicked();
    void onClearRxClicked();
    void onClearTxClicked();
    void onReadyRead();
    void onSerialError(QSerialPort::SerialPortError error);
    void onK210ReadyRead();
    void onK210SerialError(QSerialPort::SerialPortError error);
    void onReceiveTimeout();
    void onOpenLogClicked();
    void onAutoSendToggled(bool checked);
    void onAutoSendIntervalChanged(int value);
    void onHoldSendTimeout();
    void onBtForwardClicked();
    void onBtBackwardClicked();
    void onBtLeftClicked();
    void onBtRightClicked();
    void onBtStopClicked();
    void onMotorSpeedClicked();
    void onTrackOnClicked();
    void onTrackOffClicked();
    void onSnapClicked();
    void onContinueClicked();
    void onHoldClicked();
    void onStatusClicked();

private:
    Ui::Widget *ui;
    QSerialPort *serialPort;
    QSerialPort *k210SerialPort;
    QTimer *recvTimeoutTimer;
    QTimer *autoSendTimer;
    QTimer *holdSendTimer;
    QFile *logFile;
    QByteArray rxBuffer;
    QByteArray k210RxBuffer;
    bool waitingResponse;
    quint64 rxBytes;
    quint64 txBytes;
    QByteArray holdCommand;
    QByteArray holdStopCommand;
    QString holdLabel;
    QByteArray imageBase64Buffer;
    bool imageTransferActive;
    int expectedImageBytes;
    QPixmap lastImage;
};
#endif // WIDGET_H
