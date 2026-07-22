/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QGridLayout *rootLayout;
    QGroupBox *configBox;
    QGridLayout *configLayout;
    QLabel *portLabel;
    QComboBox *portCombo;
    QLabel *baudLabel;
    QComboBox *baudCombo;
    QLabel *stopBitsLabel;
    QComboBox *stopBitsCombo;
    QLabel *timeoutLabel;
    QSpinBox *timeoutSpinBox;
    QPushButton *refreshBtn;
    QLabel *modeLabel;
    QComboBox *modeCombo;
    QPushButton *openCloseBtn;
    QGroupBox *dataBox;
    QGridLayout *dataLayout;
    QLabel *sendLabel;
    QLabel *recvLabel;
    QPlainTextEdit *sendEdit;
    QPlainTextEdit *recvEdit;
    QHBoxLayout *controlLayout;
    QPushButton *sendBtn;
    QPushButton *clearRxBtn;
    QPushButton *clearTxBtn;
    QPushButton *openLogBtn;
    QSpacerItem *horizontalSpacer;
    QGroupBox *funcBox;
    QHBoxLayout *funcLayout;
    QCheckBox *hexSendCheck;
    QCheckBox *hexDisplayCheck;
    QCheckBox *appendNewlineCheck;
    QCheckBox *autoSaveLogCheck;
    QCheckBox *timestampCheck;
    QCheckBox *autoSendCheck;
    QLabel *autoSendLabel;
    QSpinBox *autoSendInterval;
    QSpacerItem *horizontalSpacer_2;
    QGroupBox *bluetoothBox;
    QGridLayout *btLayout;
    QPushButton *btForwardBtn;
    QPushButton *btLeftBtn;
    QPushButton *btStopBtn;
    QPushButton *btRightBtn;
    QPushButton *btBackwardBtn;
    QLabel *motorSpeedLabel;
    QSlider *motorSpeedSlider;
    QSpinBox *motorSpeedSpin;
    QPushButton *motorSpeedBtn;
    QPushButton *trackOnBtn;
    QPushButton *trackOffBtn;
    QSpacerItem *btSpacer;
    QGroupBox *visionBox;
    QGridLayout *visionLayout;
    QLabel *k210PortLabel;
    QComboBox *k210PortCombo;
    QPushButton *k210RefreshBtn;
    QPushButton *snapBtn;
    QPushButton *continueBtn;
    QPushButton *holdBtn;
    QPushButton *statusBtn;
    QLabel *imagePreviewLabel;
    QGroupBox *diagBox;
    QGridLayout *diagLayout;
    QLabel *modeValueTitle;
    QLabel *modeValueLabel;
    QLabel *speedValueTitle;
    QLabel *speedValueLabel;
    QLabel *distanceValueTitle;
    QLabel *distanceValueLabel;
    QLabel *trackValueTitle;
    QLabel *trackValueLabel;
    QLabel *k210ValueTitle;
    QLabel *k210ValueLabel;
    QHBoxLayout *statusLayout;
    QLabel *statusLabel;
    QSpacerItem *statusSpacer;
    QLabel *byteCountLabel;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QString::fromUtf8("Widget"));
        Widget->resize(960, 1040);
        rootLayout = new QGridLayout(Widget);
        rootLayout->setObjectName(QString::fromUtf8("rootLayout"));
        configBox = new QGroupBox(Widget);
        configBox->setObjectName(QString::fromUtf8("configBox"));
        configLayout = new QGridLayout(configBox);
        configLayout->setObjectName(QString::fromUtf8("configLayout"));
        portLabel = new QLabel(configBox);
        portLabel->setObjectName(QString::fromUtf8("portLabel"));

        configLayout->addWidget(portLabel, 0, 0, 1, 1);

        portCombo = new QComboBox(configBox);
        portCombo->setObjectName(QString::fromUtf8("portCombo"));

        configLayout->addWidget(portCombo, 0, 1, 1, 1);

        baudLabel = new QLabel(configBox);
        baudLabel->setObjectName(QString::fromUtf8("baudLabel"));

        configLayout->addWidget(baudLabel, 0, 2, 1, 1);

        baudCombo = new QComboBox(configBox);
        baudCombo->addItem(QString());
        baudCombo->addItem(QString());
        baudCombo->addItem(QString());
        baudCombo->addItem(QString());
        baudCombo->addItem(QString());
        baudCombo->addItem(QString());
        baudCombo->addItem(QString());
        baudCombo->addItem(QString());
        baudCombo->addItem(QString());
        baudCombo->addItem(QString());
        baudCombo->addItem(QString());
        baudCombo->setObjectName(QString::fromUtf8("baudCombo"));

        configLayout->addWidget(baudCombo, 0, 3, 1, 1);

        stopBitsLabel = new QLabel(configBox);
        stopBitsLabel->setObjectName(QString::fromUtf8("stopBitsLabel"));

        configLayout->addWidget(stopBitsLabel, 2, 0, 1, 1);

        stopBitsCombo = new QComboBox(configBox);
        stopBitsCombo->addItem(QString());
        stopBitsCombo->addItem(QString());
        stopBitsCombo->setObjectName(QString::fromUtf8("stopBitsCombo"));

        configLayout->addWidget(stopBitsCombo, 2, 1, 1, 1);

        timeoutLabel = new QLabel(configBox);
        timeoutLabel->setObjectName(QString::fromUtf8("timeoutLabel"));

        configLayout->addWidget(timeoutLabel, 3, 0, 1, 1);

        timeoutSpinBox = new QSpinBox(configBox);
        timeoutSpinBox->setObjectName(QString::fromUtf8("timeoutSpinBox"));
        timeoutSpinBox->setMinimum(0);
        timeoutSpinBox->setMaximum(60000);
        timeoutSpinBox->setSingleStep(100);
        timeoutSpinBox->setValue(3000);

        configLayout->addWidget(timeoutSpinBox, 3, 1, 1, 1);

        refreshBtn = new QPushButton(configBox);
        refreshBtn->setObjectName(QString::fromUtf8("refreshBtn"));

        configLayout->addWidget(refreshBtn, 3, 3, 1, 1);

        modeLabel = new QLabel(configBox);
        modeLabel->setObjectName(QString::fromUtf8("modeLabel"));

        configLayout->addWidget(modeLabel, 4, 0, 1, 1);

        modeCombo = new QComboBox(configBox);
        modeCombo->setObjectName(QString::fromUtf8("modeCombo"));

        configLayout->addWidget(modeCombo, 4, 1, 1, 1);

        openCloseBtn = new QPushButton(configBox);
        openCloseBtn->setObjectName(QString::fromUtf8("openCloseBtn"));

        configLayout->addWidget(openCloseBtn, 2, 3, 1, 1);


        rootLayout->addWidget(configBox, 0, 0, 1, 1);

        dataBox = new QGroupBox(Widget);
        dataBox->setObjectName(QString::fromUtf8("dataBox"));
        dataLayout = new QGridLayout(dataBox);
        dataLayout->setObjectName(QString::fromUtf8("dataLayout"));
        sendLabel = new QLabel(dataBox);
        sendLabel->setObjectName(QString::fromUtf8("sendLabel"));

        dataLayout->addWidget(sendLabel, 0, 0, 1, 1);

        recvLabel = new QLabel(dataBox);
        recvLabel->setObjectName(QString::fromUtf8("recvLabel"));

        dataLayout->addWidget(recvLabel, 0, 1, 1, 1);

        sendEdit = new QPlainTextEdit(dataBox);
        sendEdit->setObjectName(QString::fromUtf8("sendEdit"));

        dataLayout->addWidget(sendEdit, 1, 0, 1, 1);

        recvEdit = new QPlainTextEdit(dataBox);
        recvEdit->setObjectName(QString::fromUtf8("recvEdit"));

        dataLayout->addWidget(recvEdit, 1, 1, 1, 1);

        controlLayout = new QHBoxLayout();
        controlLayout->setObjectName(QString::fromUtf8("controlLayout"));
        sendBtn = new QPushButton(dataBox);
        sendBtn->setObjectName(QString::fromUtf8("sendBtn"));

        controlLayout->addWidget(sendBtn);

        clearRxBtn = new QPushButton(dataBox);
        clearRxBtn->setObjectName(QString::fromUtf8("clearRxBtn"));

        controlLayout->addWidget(clearRxBtn);

        clearTxBtn = new QPushButton(dataBox);
        clearTxBtn->setObjectName(QString::fromUtf8("clearTxBtn"));

        controlLayout->addWidget(clearTxBtn);

        openLogBtn = new QPushButton(dataBox);
        openLogBtn->setObjectName(QString::fromUtf8("openLogBtn"));

        controlLayout->addWidget(openLogBtn);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        controlLayout->addItem(horizontalSpacer);


        dataLayout->addLayout(controlLayout, 2, 0, 1, 2);


        rootLayout->addWidget(dataBox, 1, 0, 1, 1);

        funcBox = new QGroupBox(Widget);
        funcBox->setObjectName(QString::fromUtf8("funcBox"));
        funcLayout = new QHBoxLayout(funcBox);
        funcLayout->setObjectName(QString::fromUtf8("funcLayout"));
        hexSendCheck = new QCheckBox(funcBox);
        hexSendCheck->setObjectName(QString::fromUtf8("hexSendCheck"));

        funcLayout->addWidget(hexSendCheck);

        hexDisplayCheck = new QCheckBox(funcBox);
        hexDisplayCheck->setObjectName(QString::fromUtf8("hexDisplayCheck"));

        funcLayout->addWidget(hexDisplayCheck);

        appendNewlineCheck = new QCheckBox(funcBox);
        appendNewlineCheck->setObjectName(QString::fromUtf8("appendNewlineCheck"));

        funcLayout->addWidget(appendNewlineCheck);

        autoSaveLogCheck = new QCheckBox(funcBox);
        autoSaveLogCheck->setObjectName(QString::fromUtf8("autoSaveLogCheck"));

        funcLayout->addWidget(autoSaveLogCheck);

        timestampCheck = new QCheckBox(funcBox);
        timestampCheck->setObjectName(QString::fromUtf8("timestampCheck"));

        funcLayout->addWidget(timestampCheck);

        autoSendCheck = new QCheckBox(funcBox);
        autoSendCheck->setObjectName(QString::fromUtf8("autoSendCheck"));
        autoSendCheck->setEnabled(false);

        funcLayout->addWidget(autoSendCheck);

        autoSendLabel = new QLabel(funcBox);
        autoSendLabel->setObjectName(QString::fromUtf8("autoSendLabel"));

        funcLayout->addWidget(autoSendLabel);

        autoSendInterval = new QSpinBox(funcBox);
        autoSendInterval->setObjectName(QString::fromUtf8("autoSendInterval"));
        autoSendInterval->setMinimum(100);
        autoSendInterval->setMaximum(60000);
        autoSendInterval->setSingleStep(100);
        autoSendInterval->setValue(1000);

        funcLayout->addWidget(autoSendInterval);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        funcLayout->addItem(horizontalSpacer_2);


        rootLayout->addWidget(funcBox, 2, 0, 1, 1);

        bluetoothBox = new QGroupBox(Widget);
        bluetoothBox->setObjectName(QString::fromUtf8("bluetoothBox"));
        btLayout = new QGridLayout(bluetoothBox);
        btLayout->setObjectName(QString::fromUtf8("btLayout"));
        btForwardBtn = new QPushButton(bluetoothBox);
        btForwardBtn->setObjectName(QString::fromUtf8("btForwardBtn"));

        btLayout->addWidget(btForwardBtn, 0, 1, 1, 1);

        btLeftBtn = new QPushButton(bluetoothBox);
        btLeftBtn->setObjectName(QString::fromUtf8("btLeftBtn"));

        btLayout->addWidget(btLeftBtn, 1, 0, 1, 1);

        btStopBtn = new QPushButton(bluetoothBox);
        btStopBtn->setObjectName(QString::fromUtf8("btStopBtn"));

        btLayout->addWidget(btStopBtn, 1, 1, 1, 1);

        btRightBtn = new QPushButton(bluetoothBox);
        btRightBtn->setObjectName(QString::fromUtf8("btRightBtn"));

        btLayout->addWidget(btRightBtn, 1, 2, 1, 1);

        btBackwardBtn = new QPushButton(bluetoothBox);
        btBackwardBtn->setObjectName(QString::fromUtf8("btBackwardBtn"));

        btLayout->addWidget(btBackwardBtn, 2, 1, 1, 1);

        motorSpeedLabel = new QLabel(bluetoothBox);
        motorSpeedLabel->setObjectName(QString::fromUtf8("motorSpeedLabel"));

        btLayout->addWidget(motorSpeedLabel, 3, 0, 1, 1);

        motorSpeedSlider = new QSlider(bluetoothBox);
        motorSpeedSlider->setObjectName(QString::fromUtf8("motorSpeedSlider"));
        motorSpeedSlider->setMinimum(0);
        motorSpeedSlider->setMaximum(100);
        motorSpeedSlider->setSingleStep(5);
        motorSpeedSlider->setPageStep(10);
        motorSpeedSlider->setValue(40);
        motorSpeedSlider->setOrientation(Qt::Horizontal);

        btLayout->addWidget(motorSpeedSlider, 3, 1, 1, 1);

        motorSpeedSpin = new QSpinBox(bluetoothBox);
        motorSpeedSpin->setObjectName(QString::fromUtf8("motorSpeedSpin"));
        motorSpeedSpin->setMinimum(0);
        motorSpeedSpin->setMaximum(100);
        motorSpeedSpin->setSingleStep(5);
        motorSpeedSpin->setValue(40);

        btLayout->addWidget(motorSpeedSpin, 3, 2, 1, 1);

        motorSpeedBtn = new QPushButton(bluetoothBox);
        motorSpeedBtn->setObjectName(QString::fromUtf8("motorSpeedBtn"));

        btLayout->addWidget(motorSpeedBtn, 3, 3, 1, 1);

        trackOnBtn = new QPushButton(bluetoothBox);
        trackOnBtn->setObjectName(QString::fromUtf8("trackOnBtn"));

        btLayout->addWidget(trackOnBtn, 4, 1, 1, 1);

        trackOffBtn = new QPushButton(bluetoothBox);
        trackOffBtn->setObjectName(QString::fromUtf8("trackOffBtn"));

        btLayout->addWidget(trackOffBtn, 4, 2, 1, 1);

        btSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        btLayout->addItem(btSpacer, 1, 4, 1, 1);


        rootLayout->addWidget(bluetoothBox, 3, 0, 1, 1);

        visionBox = new QGroupBox(Widget);
        visionBox->setObjectName(QString::fromUtf8("visionBox"));
        visionLayout = new QGridLayout(visionBox);
        visionLayout->setObjectName(QString::fromUtf8("visionLayout"));
        k210PortLabel = new QLabel(visionBox);
        k210PortLabel->setObjectName(QString::fromUtf8("k210PortLabel"));

        visionLayout->addWidget(k210PortLabel, 0, 0, 1, 1);

        k210PortCombo = new QComboBox(visionBox);
        k210PortCombo->setObjectName(QString::fromUtf8("k210PortCombo"));

        visionLayout->addWidget(k210PortCombo, 0, 1, 1, 1);

        k210RefreshBtn = new QPushButton(visionBox);
        k210RefreshBtn->setObjectName(QString::fromUtf8("k210RefreshBtn"));

        visionLayout->addWidget(k210RefreshBtn, 0, 2, 1, 1);

        snapBtn = new QPushButton(visionBox);
        snapBtn->setObjectName(QString::fromUtf8("snapBtn"));

        visionLayout->addWidget(snapBtn, 1, 0, 1, 1);

        continueBtn = new QPushButton(visionBox);
        continueBtn->setObjectName(QString::fromUtf8("continueBtn"));

        visionLayout->addWidget(continueBtn, 1, 1, 1, 1);

        holdBtn = new QPushButton(visionBox);
        holdBtn->setObjectName(QString::fromUtf8("holdBtn"));

        visionLayout->addWidget(holdBtn, 1, 2, 1, 1);

        statusBtn = new QPushButton(visionBox);
        statusBtn->setObjectName(QString::fromUtf8("statusBtn"));

        visionLayout->addWidget(statusBtn, 1, 3, 1, 1);

        imagePreviewLabel = new QLabel(visionBox);
        imagePreviewLabel->setObjectName(QString::fromUtf8("imagePreviewLabel"));
        imagePreviewLabel->setMinimumSize(QSize(0, 260));
        imagePreviewLabel->setFrameShape(QFrame::StyledPanel);
        imagePreviewLabel->setAlignment(Qt::AlignCenter);

        visionLayout->addWidget(imagePreviewLabel, 2, 0, 1, 4);


        rootLayout->addWidget(visionBox, 4, 0, 1, 1);

        diagBox = new QGroupBox(Widget);
        diagBox->setObjectName(QString::fromUtf8("diagBox"));
        diagLayout = new QGridLayout(diagBox);
        diagLayout->setObjectName(QString::fromUtf8("diagLayout"));
        modeValueTitle = new QLabel(diagBox);
        modeValueTitle->setObjectName(QString::fromUtf8("modeValueTitle"));

        diagLayout->addWidget(modeValueTitle, 0, 0, 1, 1);

        modeValueLabel = new QLabel(diagBox);
        modeValueLabel->setObjectName(QString::fromUtf8("modeValueLabel"));

        diagLayout->addWidget(modeValueLabel, 0, 1, 1, 1);

        speedValueTitle = new QLabel(diagBox);
        speedValueTitle->setObjectName(QString::fromUtf8("speedValueTitle"));

        diagLayout->addWidget(speedValueTitle, 0, 2, 1, 1);

        speedValueLabel = new QLabel(diagBox);
        speedValueLabel->setObjectName(QString::fromUtf8("speedValueLabel"));

        diagLayout->addWidget(speedValueLabel, 0, 3, 1, 1);

        distanceValueTitle = new QLabel(diagBox);
        distanceValueTitle->setObjectName(QString::fromUtf8("distanceValueTitle"));

        diagLayout->addWidget(distanceValueTitle, 1, 0, 1, 1);

        distanceValueLabel = new QLabel(diagBox);
        distanceValueLabel->setObjectName(QString::fromUtf8("distanceValueLabel"));

        diagLayout->addWidget(distanceValueLabel, 1, 1, 1, 1);

        trackValueTitle = new QLabel(diagBox);
        trackValueTitle->setObjectName(QString::fromUtf8("trackValueTitle"));

        diagLayout->addWidget(trackValueTitle, 1, 2, 1, 1);

        trackValueLabel = new QLabel(diagBox);
        trackValueLabel->setObjectName(QString::fromUtf8("trackValueLabel"));

        diagLayout->addWidget(trackValueLabel, 1, 3, 1, 1);

        k210ValueTitle = new QLabel(diagBox);
        k210ValueTitle->setObjectName(QString::fromUtf8("k210ValueTitle"));

        diagLayout->addWidget(k210ValueTitle, 2, 0, 1, 1);

        k210ValueLabel = new QLabel(diagBox);
        k210ValueLabel->setObjectName(QString::fromUtf8("k210ValueLabel"));

        diagLayout->addWidget(k210ValueLabel, 2, 1, 1, 3);


        rootLayout->addWidget(diagBox, 5, 0, 1, 1);

        statusLayout = new QHBoxLayout();
        statusLayout->setObjectName(QString::fromUtf8("statusLayout"));
        statusLabel = new QLabel(Widget);
        statusLabel->setObjectName(QString::fromUtf8("statusLabel"));

        statusLayout->addWidget(statusLabel);

        statusSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        statusLayout->addItem(statusSpacer);

        byteCountLabel = new QLabel(Widget);
        byteCountLabel->setObjectName(QString::fromUtf8("byteCountLabel"));

        statusLayout->addWidget(byteCountLabel);


        rootLayout->addLayout(statusLayout, 6, 0, 1, 1);


        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QCoreApplication::translate("Widget", "\345\244\232\345\212\237\350\203\275\344\270\262\345\217\243\345\212\251\346\211\213", nullptr));
        configBox->setTitle(QCoreApplication::translate("Widget", "\344\270\262\345\217\243\345\217\202\346\225\260\351\205\215\347\275\256", nullptr));
        portLabel->setText(QCoreApplication::translate("Widget", "\344\270\262\345\217\243", nullptr));
        baudLabel->setText(QCoreApplication::translate("Widget", "\346\263\242\347\211\271\347\216\207", nullptr));
        baudCombo->setItemText(0, QCoreApplication::translate("Widget", "1200", nullptr));
        baudCombo->setItemText(1, QCoreApplication::translate("Widget", "2400", nullptr));
        baudCombo->setItemText(2, QCoreApplication::translate("Widget", "4800", nullptr));
        baudCombo->setItemText(3, QCoreApplication::translate("Widget", "9600", nullptr));
        baudCombo->setItemText(4, QCoreApplication::translate("Widget", "19200", nullptr));
        baudCombo->setItemText(5, QCoreApplication::translate("Widget", "38400", nullptr));
        baudCombo->setItemText(6, QCoreApplication::translate("Widget", "57600", nullptr));
        baudCombo->setItemText(7, QCoreApplication::translate("Widget", "115200", nullptr));
        baudCombo->setItemText(8, QCoreApplication::translate("Widget", "230400", nullptr));
        baudCombo->setItemText(9, QCoreApplication::translate("Widget", "460800", nullptr));
        baudCombo->setItemText(10, QCoreApplication::translate("Widget", "921600", nullptr));

        stopBitsLabel->setText(QCoreApplication::translate("Widget", "\345\201\234\346\255\242\344\275\215", nullptr));
        stopBitsCombo->setItemText(0, QCoreApplication::translate("Widget", "1", nullptr));
        stopBitsCombo->setItemText(1, QCoreApplication::translate("Widget", "2", nullptr));

        timeoutLabel->setText(QCoreApplication::translate("Widget", "\346\216\245\346\224\266\350\266\205\346\227\266(ms)", nullptr));
#if QT_CONFIG(tooltip)
        timeoutSpinBox->setToolTip(QCoreApplication::translate("Widget", "0 \350\241\250\347\244\272\345\205\263\351\227\255\350\266\205\346\227\266\346\243\200\346\265\213", nullptr));
#endif // QT_CONFIG(tooltip)
        refreshBtn->setText(QCoreApplication::translate("Widget", "\345\210\267\346\226\260\344\270\262\345\217\243", nullptr));
        modeLabel->setText(QCoreApplication::translate("Widget", "\350\277\236\346\216\245\346\250\241\345\274\217", nullptr));
        openCloseBtn->setText(QCoreApplication::translate("Widget", "\346\211\223\345\274\200\344\270\262\345\217\243", nullptr));
        dataBox->setTitle(QCoreApplication::translate("Widget", "\346\225\260\346\215\256\346\224\266\345\217\221\345\214\272", nullptr));
        sendLabel->setText(QCoreApplication::translate("Widget", "\345\217\221\351\200\201\346\226\207\346\234\254", nullptr));
        recvLabel->setText(QCoreApplication::translate("Widget", "\346\216\245\346\224\266\346\226\207\346\234\254", nullptr));
        sendBtn->setText(QCoreApplication::translate("Widget", "\345\217\221\351\200\201\346\225\260\346\215\256", nullptr));
        clearRxBtn->setText(QCoreApplication::translate("Widget", "\346\270\205\347\251\272\346\216\245\346\224\266", nullptr));
        clearTxBtn->setText(QCoreApplication::translate("Widget", "\346\270\205\347\251\272\345\217\221\351\200\201", nullptr));
        openLogBtn->setText(QCoreApplication::translate("Widget", "\346\211\223\345\274\200\346\227\245\345\277\227", nullptr));
        funcBox->setTitle(QCoreApplication::translate("Widget", "\345\212\237\350\203\275\345\244\215\351\200\211\346\241\206", nullptr));
        hexSendCheck->setText(QCoreApplication::translate("Widget", "Hex \345\217\221\351\200\201", nullptr));
        hexDisplayCheck->setText(QCoreApplication::translate("Widget", "Hex \346\230\276\347\244\272", nullptr));
        appendNewlineCheck->setText(QCoreApplication::translate("Widget", "\345\217\221\351\200\201\346\226\260\350\241\214", nullptr));
        autoSaveLogCheck->setText(QCoreApplication::translate("Widget", "\350\207\252\345\212\250\344\277\235\345\255\230\346\227\245\345\277\227", nullptr));
        timestampCheck->setText(QCoreApplication::translate("Widget", "\346\227\266\351\227\264\346\210\263", nullptr));
        autoSendCheck->setText(QCoreApplication::translate("Widget", "\345\256\232\346\227\266\345\217\221\351\200\201", nullptr));
        autoSendLabel->setText(QCoreApplication::translate("Widget", "\351\227\264\351\232\224(ms)", nullptr));
#if QT_CONFIG(tooltip)
        autoSendInterval->setToolTip(QCoreApplication::translate("Widget", "\345\256\232\346\227\266\345\217\221\351\200\201\351\227\264\351\232\224\357\274\210\346\257\253\347\247\222\357\274\211", nullptr));
#endif // QT_CONFIG(tooltip)
        bluetoothBox->setTitle(QCoreApplication::translate("Widget", "\350\223\235\347\211\231\346\216\247\345\210\266 (CH-553)", nullptr));
        btForwardBtn->setText(QCoreApplication::translate("Widget", "\345\211\215\350\277\233 (F)", nullptr));
        btLeftBtn->setText(QCoreApplication::translate("Widget", "\345\267\246\350\275\254 (L)", nullptr));
        btStopBtn->setText(QCoreApplication::translate("Widget", "\345\201\234\346\255\242 (S)", nullptr));
        btRightBtn->setText(QCoreApplication::translate("Widget", "\345\217\263\350\275\254 (R)", nullptr));
        btBackwardBtn->setText(QCoreApplication::translate("Widget", "\345\220\216\351\200\200 (B)", nullptr));
        motorSpeedLabel->setText(QCoreApplication::translate("Widget", "\351\200\237\345\272\246", nullptr));
        motorSpeedSpin->setSuffix(QCoreApplication::translate("Widget", "%", nullptr));
        motorSpeedBtn->setText(QCoreApplication::translate("Widget", "\350\256\276\347\275\256\351\200\237\345\272\246", nullptr));
        trackOnBtn->setText(QCoreApplication::translate("Widget", "\345\274\200\345\220\257\345\276\252\350\277\271", nullptr));
        trackOffBtn->setText(QCoreApplication::translate("Widget", "\345\205\263\351\227\255\345\276\252\350\277\271", nullptr));
        visionBox->setTitle(QCoreApplication::translate("Widget", "\350\247\206\350\247\211\345\233\236\344\274\240", nullptr));
        k210PortLabel->setText(QCoreApplication::translate("Widget", "K210 USB\344\270\262\345\217\243", nullptr));
        k210RefreshBtn->setText(QCoreApplication::translate("Widget", "\345\210\267\346\226\260K210", nullptr));
        snapBtn->setText(QCoreApplication::translate("Widget", "\346\213\215\347\205\247", nullptr));
        continueBtn->setText(QCoreApplication::translate("Widget", "\347\273\247\347\273\255\345\211\215\350\277\233", nullptr));
        holdBtn->setText(QCoreApplication::translate("Widget", "\344\277\235\346\214\201\345\201\234\346\255\242", nullptr));
        statusBtn->setText(QCoreApplication::translate("Widget", "\346\237\245\350\257\242\347\212\266\346\200\201", nullptr));
        imagePreviewLabel->setText(QCoreApplication::translate("Widget", "\346\232\202\346\227\240\345\233\276\345\203\217", nullptr));
        diagBox->setTitle(QCoreApplication::translate("Widget", "\350\257\212\346\226\255\346\221\230\350\246\201", nullptr));
        modeValueTitle->setText(QCoreApplication::translate("Widget", "\346\250\241\345\274\217", nullptr));
        modeValueLabel->setText(QCoreApplication::translate("Widget", "--", nullptr));
        speedValueTitle->setText(QCoreApplication::translate("Widget", "\351\200\237\345\272\246", nullptr));
        speedValueLabel->setText(QCoreApplication::translate("Widget", "--", nullptr));
        distanceValueTitle->setText(QCoreApplication::translate("Widget", "\346\265\213\350\267\235", nullptr));
        distanceValueLabel->setText(QCoreApplication::translate("Widget", "--", nullptr));
        trackValueTitle->setText(QCoreApplication::translate("Widget", "\345\276\252\350\277\271", nullptr));
        trackValueLabel->setText(QCoreApplication::translate("Widget", "--", nullptr));
        k210ValueTitle->setText(QCoreApplication::translate("Widget", "K210", nullptr));
        k210ValueLabel->setText(QCoreApplication::translate("Widget", "--", nullptr));
        statusLabel->setText(QCoreApplication::translate("Widget", "\347\212\266\346\200\201\357\274\232\346\234\252\350\277\236\346\216\245", nullptr));
        byteCountLabel->setText(QCoreApplication::translate("Widget", "RX: 0  TX: 0", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
