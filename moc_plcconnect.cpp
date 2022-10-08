/****************************************************************************
** Meta object code from reading C++ file 'plcconnect.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "plcconnect.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'plcconnect.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_PLCConnect_t {
    QByteArrayData data[7];
    char stringdata0[68];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PLCConnect_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PLCConnect_t qt_meta_stringdata_PLCConnect = {
    {
QT_MOC_LITERAL(0, 0, 10), // "PLCConnect"
QT_MOC_LITERAL(1, 11, 8), // "plcStart"
QT_MOC_LITERAL(2, 20, 0), // ""
QT_MOC_LITERAL(3, 21, 11), // "readPLCData"
QT_MOC_LITERAL(4, 33, 12), // "disPLCSocket"
QT_MOC_LITERAL(5, 46, 7), // "ctrlPLC"
QT_MOC_LITERAL(6, 54, 13) // "allowedToRead"

    },
    "PLCConnect\0plcStart\0\0readPLCData\0"
    "disPLCSocket\0ctrlPLC\0allowedToRead"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PLCConnect[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x0a /* Public */,
       3,    1,   40,    2, 0x08 /* Private */,
       4,    0,   43,    2, 0x08 /* Private */,
       5,    0,   44,    2, 0x08 /* Private */,
       6,    0,   45,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void PLCConnect::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        PLCConnect *_t = static_cast<PLCConnect *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->plcStart(); break;
        case 1: _t->readPLCData((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->disPLCSocket(); break;
        case 3: _t->ctrlPLC(); break;
        case 4: _t->allowedToRead(); break;
        default: ;
        }
    }
}

const QMetaObject PLCConnect::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_PLCConnect.data,
      qt_meta_data_PLCConnect,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *PLCConnect::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PLCConnect::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_PLCConnect.stringdata0))
        return static_cast<void*>(const_cast< PLCConnect*>(this));
    return QObject::qt_metacast(_clname);
}

int PLCConnect::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
