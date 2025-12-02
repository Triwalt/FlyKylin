/****************************************************************************
** Meta object code from reading C++ file 'PeerDiscovery.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/core/communication/PeerDiscovery.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PeerDiscovery.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_flykylin__core__PeerDiscovery_t {
    QByteArrayData data[11];
    char stringdata0[149];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_flykylin__core__PeerDiscovery_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_flykylin__core__PeerDiscovery_t qt_meta_stringdata_flykylin__core__PeerDiscovery = {
    {
QT_MOC_LITERAL(0, 0, 29), // "flykylin::core::PeerDiscovery"
QT_MOC_LITERAL(1, 30, 14), // "peerDiscovered"
QT_MOC_LITERAL(2, 45, 0), // ""
QT_MOC_LITERAL(3, 46, 8), // "PeerNode"
QT_MOC_LITERAL(4, 55, 4), // "node"
QT_MOC_LITERAL(5, 60, 11), // "peerOffline"
QT_MOC_LITERAL(6, 72, 6), // "userId"
QT_MOC_LITERAL(7, 79, 13), // "peerHeartbeat"
QT_MOC_LITERAL(8, 93, 16), // "onBroadcastTimer"
QT_MOC_LITERAL(9, 110, 19), // "onTimeoutCheckTimer"
QT_MOC_LITERAL(10, 130, 18) // "onDatagramReceived"

    },
    "flykylin::core::PeerDiscovery\0"
    "peerDiscovered\0\0PeerNode\0node\0peerOffline\0"
    "userId\0peerHeartbeat\0onBroadcastTimer\0"
    "onTimeoutCheckTimer\0onDatagramReceived"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_flykylin__core__PeerDiscovery[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x06 /* Public */,
       5,    1,   47,    2, 0x06 /* Public */,
       7,    1,   50,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    0,   53,    2, 0x08 /* Private */,
       9,    0,   54,    2, 0x08 /* Private */,
      10,    0,   55,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, QMetaType::QString,    6,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void flykylin::core::PeerDiscovery::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PeerDiscovery *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->peerDiscovered((*reinterpret_cast< const PeerNode(*)>(_a[1]))); break;
        case 1: _t->peerOffline((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->peerHeartbeat((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->onBroadcastTimer(); break;
        case 4: _t->onTimeoutCheckTimer(); break;
        case 5: _t->onDatagramReceived(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PeerDiscovery::*)(const PeerNode & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PeerDiscovery::peerDiscovered)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (PeerDiscovery::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PeerDiscovery::peerOffline)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (PeerDiscovery::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PeerDiscovery::peerHeartbeat)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject flykylin::core::PeerDiscovery::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_flykylin__core__PeerDiscovery.data,
    qt_meta_data_flykylin__core__PeerDiscovery,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *flykylin::core::PeerDiscovery::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *flykylin::core::PeerDiscovery::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_flykylin__core__PeerDiscovery.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int flykylin::core::PeerDiscovery::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void flykylin::core::PeerDiscovery::peerDiscovered(const PeerNode & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void flykylin::core::PeerDiscovery::peerOffline(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void flykylin::core::PeerDiscovery::peerHeartbeat(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
