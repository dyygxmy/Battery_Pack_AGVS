/****************************************************************************
** Meta object code from reading C++ file 'serverconnect.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "serverconnect.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'serverconnect.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_serverConnect_t {
    QByteArrayData data[7];
    char stringdata0[80];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_serverConnect_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_serverConnect_t qt_meta_stringdata_serverConnect = {
    {
QT_MOC_LITERAL(0, 0, 13), // "serverConnect"
QT_MOC_LITERAL(1, 14, 14), // "sendUpdateTime"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 11), // "serverStart"
QT_MOC_LITERAL(4, 42, 13), // "replyFinished"
QT_MOC_LITERAL(5, 56, 14), // "QNetworkReply*"
QT_MOC_LITERAL(6, 71, 8) // "handdata"

    },
    "serverConnect\0sendUpdateTime\0\0serverStart\0"
    "replyFinished\0QNetworkReply*\0handdata"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_serverConnect[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   34,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    0,   37,    2, 0x0a /* Public */,
       4,    1,   38,    2, 0x08 /* Private */,
       6,    0,   41,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    2,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 5,    2,
    QMetaType::Void,

       0        // eod
};

void serverConnect::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        serverConnect *_t = static_cast<serverConnect *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->sendUpdateTime((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->serverStart(); break;
        case 2: _t->replyFinished((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        case 3: _t->handdata(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QNetworkReply* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (serverConnect::*_t)(QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&serverConnect::sendUpdateTime)) {
                *result = 0;
            }
        }
    }
}

const QMetaObject serverConnect::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_serverConnect.data,
      qt_meta_data_serverConnect,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *serverConnect::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *serverConnect::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_serverConnect.stringdata0))
        return static_cast<void*>(const_cast< serverConnect*>(this));
    return QObject::qt_metacast(_clname);
}

int serverConnect::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void serverConnect::sendUpdateTime(QString _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
