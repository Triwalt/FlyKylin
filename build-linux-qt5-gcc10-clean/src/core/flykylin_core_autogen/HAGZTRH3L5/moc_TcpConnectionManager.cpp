/****************************************************************************
** Meta object code from reading C++ file 'TcpConnectionManager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/core/communication/TcpConnectionManager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TcpConnectionManager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_flykylin__communication__TcpConnectionManager_t {
    QByteArrayData data[24];
    char stringdata0[353];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_flykylin__communication__TcpConnectionManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_flykylin__communication__TcpConnectionManager_t qt_meta_stringdata_flykylin__communication__TcpConnectionManager = {
    {
QT_MOC_LITERAL(0, 0, 45), // "flykylin::communication::TcpC..."
QT_MOC_LITERAL(1, 46, 22), // "connectionStateChanged"
QT_MOC_LITERAL(2, 69, 0), // ""
QT_MOC_LITERAL(3, 70, 6), // "peerId"
QT_MOC_LITERAL(4, 77, 15), // "ConnectionState"
QT_MOC_LITERAL(5, 93, 5), // "state"
QT_MOC_LITERAL(6, 99, 6), // "reason"
QT_MOC_LITERAL(7, 106, 15), // "messageReceived"
QT_MOC_LITERAL(8, 122, 4), // "data"
QT_MOC_LITERAL(9, 127, 11), // "messageSent"
QT_MOC_LITERAL(10, 139, 9), // "messageId"
QT_MOC_LITERAL(11, 149, 13), // "messageFailed"
QT_MOC_LITERAL(12, 163, 5), // "error"
QT_MOC_LITERAL(13, 169, 24), // "onConnectionStateChanged"
QT_MOC_LITERAL(14, 194, 17), // "onMessageReceived"
QT_MOC_LITERAL(15, 212, 13), // "onMessageSent"
QT_MOC_LITERAL(16, 226, 15), // "onMessageFailed"
QT_MOC_LITERAL(17, 242, 22), // "cleanupIdleConnections"
QT_MOC_LITERAL(18, 265, 19), // "processMessageQueue"
QT_MOC_LITERAL(19, 285, 16), // "onPeerDiscovered"
QT_MOC_LITERAL(20, 302, 24), // "flykylin::core::PeerNode"
QT_MOC_LITERAL(21, 327, 4), // "node"
QT_MOC_LITERAL(22, 332, 13), // "onPeerOffline"
QT_MOC_LITERAL(23, 346, 6) // "userId"

    },
    "flykylin::communication::TcpConnectionManager\0"
    "connectionStateChanged\0\0peerId\0"
    "ConnectionState\0state\0reason\0"
    "messageReceived\0data\0messageSent\0"
    "messageId\0messageFailed\0error\0"
    "onConnectionStateChanged\0onMessageReceived\0"
    "onMessageSent\0onMessageFailed\0"
    "cleanupIdleConnections\0processMessageQueue\0"
    "onPeerDiscovered\0flykylin::core::PeerNode\0"
    "node\0onPeerOffline\0userId"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_flykylin__communication__TcpConnectionManager[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    3,   74,    2, 0x06 /* Public */,
       7,    2,   81,    2, 0x06 /* Public */,
       9,    2,   86,    2, 0x06 /* Public */,
      11,    3,   91,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      13,    2,   98,    2, 0x08 /* Private */,
      14,    1,  103,    2, 0x08 /* Private */,
      15,    1,  106,    2, 0x08 /* Private */,
      16,    2,  109,    2, 0x08 /* Private */,
      17,    0,  114,    2, 0x08 /* Private */,
      18,    1,  115,    2, 0x08 /* Private */,
      19,    1,  118,    2, 0x08 /* Private */,
      22,    1,  121,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, 0x80000000 | 4, QMetaType::QString,    3,    5,    6,
    QMetaType::Void, QMetaType::QString, QMetaType::QByteArray,    3,    8,
    QMetaType::Void, QMetaType::QString, QMetaType::ULongLong,    3,   10,
    QMetaType::Void, QMetaType::QString, QMetaType::ULongLong, QMetaType::QString,    3,   10,   12,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 4, QMetaType::QString,    5,    6,
    QMetaType::Void, QMetaType::QByteArray,    8,
    QMetaType::Void, QMetaType::ULongLong,   10,
    QMetaType::Void, QMetaType::ULongLong, QMetaType::QString,   10,   12,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, 0x80000000 | 20,   21,
    QMetaType::Void, QMetaType::QString,   23,

       0        // eod
};

void flykylin::communication::TcpConnectionManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TcpConnectionManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->connectionStateChanged((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< ConnectionState(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 1: _t->messageReceived((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QByteArray(*)>(_a[2]))); break;
        case 2: _t->messageSent((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint64(*)>(_a[2]))); break;
        case 3: _t->messageFailed((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint64(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 4: _t->onConnectionStateChanged((*reinterpret_cast< ConnectionState(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 5: _t->onMessageReceived((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 6: _t->onMessageSent((*reinterpret_cast< quint64(*)>(_a[1]))); break;
        case 7: _t->onMessageFailed((*reinterpret_cast< quint64(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 8: _t->cleanupIdleConnections(); break;
        case 9: _t->processMessageQueue((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 10: _t->onPeerDiscovered((*reinterpret_cast< const flykylin::core::PeerNode(*)>(_a[1]))); break;
        case 11: _t->onPeerOffline((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (TcpConnectionManager::*)(QString , ConnectionState , QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TcpConnectionManager::connectionStateChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (TcpConnectionManager::*)(QString , QByteArray );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TcpConnectionManager::messageReceived)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (TcpConnectionManager::*)(QString , quint64 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TcpConnectionManager::messageSent)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (TcpConnectionManager::*)(QString , quint64 , QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TcpConnectionManager::messageFailed)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject flykylin::communication::TcpConnectionManager::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_flykylin__communication__TcpConnectionManager.data,
    qt_meta_data_flykylin__communication__TcpConnectionManager,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *flykylin::communication::TcpConnectionManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *flykylin::communication::TcpConnectionManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_flykylin__communication__TcpConnectionManager.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int flykylin::communication::TcpConnectionManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void flykylin::communication::TcpConnectionManager::connectionStateChanged(QString _t1, ConnectionState _t2, QString _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void flykylin::communication::TcpConnectionManager::messageReceived(QString _t1, QByteArray _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void flykylin::communication::TcpConnectionManager::messageSent(QString _t1, quint64 _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void flykylin::communication::TcpConnectionManager::messageFailed(QString _t1, quint64 _t2, QString _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
