/****************************************************************************
** Meta object code from reading C++ file 'widget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../widget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'widget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Widget_t {
    QByteArrayData data[18];
    char stringdata0[271];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Widget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Widget_t qt_meta_stringdata_Widget = {
    {
QT_MOC_LITERAL(0, 0, 6), // "Widget"
QT_MOC_LITERAL(1, 7, 20), // "onOpenOrCloseClicked"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 13), // "onSendClicked"
QT_MOC_LITERAL(4, 43, 16), // "onClearRxClicked"
QT_MOC_LITERAL(5, 60, 16), // "onClearTxClicked"
QT_MOC_LITERAL(6, 77, 11), // "onReadyRead"
QT_MOC_LITERAL(7, 89, 13), // "onSerialError"
QT_MOC_LITERAL(8, 103, 28), // "QSerialPort::SerialPortError"
QT_MOC_LITERAL(9, 132, 5), // "error"
QT_MOC_LITERAL(10, 138, 16), // "onReceiveTimeout"
QT_MOC_LITERAL(11, 155, 16), // "onOpenLogClicked"
QT_MOC_LITERAL(12, 172, 17), // "onAutoSendToggled"
QT_MOC_LITERAL(13, 190, 7), // "checked"
QT_MOC_LITERAL(14, 198, 25), // "onAutoSendIntervalChanged"
QT_MOC_LITERAL(15, 224, 5), // "value"
QT_MOC_LITERAL(16, 230, 21), // "onMotorForwardClicked"
QT_MOC_LITERAL(17, 252, 18) // "onMotorStopClicked"

    },
    "Widget\0onOpenOrCloseClicked\0\0onSendClicked\0"
    "onClearRxClicked\0onClearTxClicked\0"
    "onReadyRead\0onSerialError\0"
    "QSerialPort::SerialPortError\0error\0"
    "onReceiveTimeout\0onOpenLogClicked\0"
    "onAutoSendToggled\0checked\0"
    "onAutoSendIntervalChanged\0value\0"
    "onMotorForwardClicked\0onMotorStopClicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Widget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   74,    2, 0x08 /* Private */,
       3,    0,   75,    2, 0x08 /* Private */,
       4,    0,   76,    2, 0x08 /* Private */,
       5,    0,   77,    2, 0x08 /* Private */,
       6,    0,   78,    2, 0x08 /* Private */,
       7,    1,   79,    2, 0x08 /* Private */,
      10,    0,   82,    2, 0x08 /* Private */,
      11,    0,   83,    2, 0x08 /* Private */,
      12,    1,   84,    2, 0x08 /* Private */,
      14,    1,   87,    2, 0x08 /* Private */,
      16,    0,   90,    2, 0x08 /* Private */,
      17,    0,   91,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   13,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void Widget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Widget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onOpenOrCloseClicked(); break;
        case 1: _t->onSendClicked(); break;
        case 2: _t->onClearRxClicked(); break;
        case 3: _t->onClearTxClicked(); break;
        case 4: _t->onReadyRead(); break;
        case 5: _t->onSerialError((*reinterpret_cast< QSerialPort::SerialPortError(*)>(_a[1]))); break;
        case 6: _t->onReceiveTimeout(); break;
        case 7: _t->onOpenLogClicked(); break;
        case 8: _t->onAutoSendToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: _t->onAutoSendIntervalChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->onMotorForwardClicked(); break;
        case 11: _t->onMotorStopClicked(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Widget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_Widget.data,
    qt_meta_data_Widget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Widget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Widget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Widget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Widget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 12;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
