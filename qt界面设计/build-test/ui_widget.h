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
    QLabel *dataBitsLabel;
    QComboBox *dataBitsCombo;
    QLabel *parityLabel;
    QComboBox *parityCombo;
    QLabel *stopBitsLabel;
    QComboBox *stopBitsCombo;
    QLabel *flowControlLabel;
    QComboBox *flowControlCombo;
    QLabel *timeoutLabel;
    QSpinBox *timeoutSpinBox;
    QPushButton *openCloseBtn;
    QPushButton *refreshBtn;
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
    QGroupBox *motorBox;
    QHBoxLayout *motorLayout;
    QPushButton *motorForwardBtn;
    QPushButton *motorStopBtn;
    QSpacerItem *motorSpacer;
    QHBoxLayout *statusLayout;
    QLabel *statusLabel;
    QSpacerItem *statusSpacer;
    QLabel *byteCountLabel;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QString::fromUtf8("Widget"));
        Widget->resize(960, 700);
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

        dataBitsLabel = new QLabel(configBox);
        dataBitsLabel->setObjectName(QString::fromUtf8("dataBitsLabel"));

        configLayout->addWidget(dataBitsLabel, 1, 0, 1, 1);

        dataBitsCombo = new QComboBox(configBox);
        dataBitsCombo->addItem(QString());
        dataBitsCombo->addItem(QString());
        dataBitsCombo->addItem(QString());
        dataBitsCombo->addItem(QString());
        dataBitsCombo->setObjectName(QString::fromUtf8("dataBitsCombo"));

        configLayout->addWidget(dataBitsCombo, 1, 1, 1, 1);

        parityLabel = new QLabel(configBox);
        parityLabel->setObjectName(QString::fromUtf8("parityLabel"));

        configLayout->addWidget(parityLabel, 1, 2, 1, 1);

        parityCombo = new QComboBox(configBox);
        parityCombo->addItem(QString());
        parityCombo->addItem(QString());
        parityCombo->addItem(QString());
        parityCombo->setObjectName(QString::fromUtf8("parityCombo"));

        configLayout->addWidget(parityCombo, 1, 3, 1, 1);

        stopBitsLabel = new QLabel(configBox);
        stopBitsLabel->setObjectName(QString::fromUtf8("stopBitsLabel"));

        configLayout->addWidget(stopBitsLabel, 2, 0, 1, 1);

        stopBitsCombo = new QComboBox(configBox);
        stopBitsCombo->addItem(QString());
        stopBitsCombo->addItem(QString());
        stopBitsCombo->setObjectName(QString::fromUtf8("stopBitsCombo"));

        configLayout->addWidget(stopBitsCombo, 2, 1, 1, 1);

        flowControlLabel = new QLabel(configBox);
        flowControlLabel->setObjectName(QString::fromUtf8("flowControlLabel"));

        configLayout->addWidget(flowControlLabel, 2, 2, 1, 1);

        flowControlCombo = new QComboBox(configBox);
        flowControlCombo->addItem(QString());
        flowControlCombo->addItem(QString());
        flowControlCombo->addItem(QString());
        flowControlCombo->setObjectName(QString::fromUtf8("flowControlCombo"));

        configLayout->addWidget(flowControlCombo, 2, 3, 1, 1);

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

        openCloseBtn = new QPushButton(configBox);
        openCloseBtn->setObjectName(QString::fromUtf8("openCloseBtn"));

        configLayout->addWidget(openCloseBtn, 3, 2, 1, 1);

        refreshBtn = new QPushButton(configBox);
        refreshBtn->setObjectName(QString::fromUtf8("refreshBtn"));

        configLayout->addWidget(refreshBtn, 3, 3, 1, 1);


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

        motorBox = new QGroupBox(Widget);
        motorBox->setObjectName(QString::fromUtf8("motorBox"));
        motorLayout = new QHBoxLayout(motorBox);
        motorLayout->setObjectName(QString::fromUtf8("motorLayout"));
        motorForwardBtn = new QPushButton(motorBox);
        motorForwardBtn->setObjectName(QString::fromUtf8("motorForwardBtn"));

        motorLayout->addWidget(motorForwardBtn);

        motorStopBtn = new QPushButton(motorBox);
        motorStopBtn->setObjectName(QString::fromUtf8("motorStopBtn"));

        motorLayout->addWidget(motorStopBtn);

        motorSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        motorLayout->addItem(motorSpacer);


        rootLayout->addWidget(motorBox, 3, 0, 1, 1);

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


        rootLayout->addLayout(statusLayout, 4, 0, 1, 1);


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

        dataBitsLabel->setText(QCoreApplication::translate("Widget", "\346\225\260\346\215\256\344\275\215", nullptr));
        dataBitsCombo->setItemText(0, QCoreApplication::translate("Widget", "5", nullptr));
        dataBitsCombo->setItemText(1, QCoreApplication::translate("Widget", "6", nullptr));
        dataBitsCombo->setItemText(2, QCoreApplication::translate("Widget", "7", nullptr));
        dataBitsCombo->setItemText(3, QCoreApplication::translate("Widget", "8", nullptr));

        parityLabel->setText(QCoreApplication::translate("Widget", "\346\240\241\351\252\214\344\275\215", nullptr));
        parityCombo->setItemText(0, QCoreApplication::translate("Widget", "None", nullptr));
        parityCombo->setItemText(1, QCoreApplication::translate("Widget", "Odd", nullptr));
        parityCombo->setItemText(2, QCoreApplication::translate("Widget", "Even", nullptr));

        stopBitsLabel->setText(QCoreApplication::translate("Widget", "\345\201\234\346\255\242\344\275\215", nullptr));
        stopBitsCombo->setItemText(0, QCoreApplication::translate("Widget", "1", nullptr));
        stopBitsCombo->setItemText(1, QCoreApplication::translate("Widget", "2", nullptr));

        flowControlLabel->setText(QCoreApplication::translate("Widget", "\346\265\201\346\216\247", nullptr));
        flowControlCombo->setItemText(0, QCoreApplication::translate("Widget", "None", nullptr));
        flowControlCombo->setItemText(1, QCoreApplication::translate("Widget", "Hardware", nullptr));
        flowControlCombo->setItemText(2, QCoreApplication::translate("Widget", "Software", nullptr));

        timeoutLabel->setText(QCoreApplication::translate("Widget", "\346\216\245\346\224\266\350\266\205\346\227\266(ms)", nullptr));
#if QT_CONFIG(tooltip)
        timeoutSpinBox->setToolTip(QCoreApplication::translate("Widget", "0 \350\241\250\347\244\272\345\205\263\351\227\255\350\266\205\346\227\266\346\243\200\346\265\213", nullptr));
#endif // QT_CONFIG(tooltip)
        openCloseBtn->setText(QCoreApplication::translate("Widget", "\346\211\223\345\274\200\344\270\262\345\217\243", nullptr));
        refreshBtn->setText(QCoreApplication::translate("Widget", "\345\210\267\346\226\260\344\270\262\345\217\243", nullptr));
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
        motorBox->setTitle(QCoreApplication::translate("Widget", "\347\224\265\346\234\272\346\216\247\345\210\266", nullptr));
        motorForwardBtn->setText(QCoreApplication::translate("Widget", "\345\211\215\350\277\233 (go)", nullptr));
        motorStopBtn->setText(QCoreApplication::translate("Widget", "\345\201\234\346\255\242 (stop)", nullptr));
        statusLabel->setText(QCoreApplication::translate("Widget", "\347\212\266\346\200\201\357\274\232\346\234\252\350\277\236\346\216\245", nullptr));
        byteCountLabel->setText(QCoreApplication::translate("Widget", "RX: 0  TX: 0", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
