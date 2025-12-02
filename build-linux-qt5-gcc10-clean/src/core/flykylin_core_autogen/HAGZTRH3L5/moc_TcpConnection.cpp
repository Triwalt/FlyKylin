/****************************************************************************
** Meta object code from reading C++ file 'TcpConnection.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/core/communication/TcpConnection.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TcpConnection.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_flykylin__communication__TcpConnection_t {
    QByteArrayData data[23];
    char stringdata0[336];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_flykylin__communication__TcpConnection_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_flykylin__communication__TcpConnection_t qt_meta_stringdata_flykylin__communication__TcpConnection = {
    {
QT_MOC_LITERAL(0, 0, 38), // "flykylin::communication::TcpC..."
QT_MOC_LITERAL(1, 39, 12), // "stateChanged"
QT_MOC_LITERAL(2, 52, 0), // ""
QT_MOC_LITERAL(3, 53, 15), // "ConnectionState"
QT_MOC_LITERAL(4, 69, 8), // "newState"
QT_MOC_LITERAL(5, 78, 6), // "reason"
QT_MOC_LITERAL(6, 85, 15), // "messageReceived"
QT_MOC_LITERAL(7, 101, 4), // "data"
QT_MOC_LITERAL(8, 106, 11), // "messageSent"
QT_MOC_LITERAL(9, 118, 9), // "messageId"
QT_MOC_LITERAL(10, 128, 13), // "messageFailed"
QT_MOC_LITERAL(11, 142, 5), // "error"
QT_MOC_LITERAL(12, 148, 13), // "errorOccurred"
QT_MOC_LITERAL(13, 162, 18), // "handshakeCompleted"
QT_MOC_LITERAL(14, 181, 15), // "handshakeFailed"
QT_MOC_LITERAL(15, 197, 11), // "onConnected"
QT_MOC_LITERAL(16, 209, 14), // "onDisconnected"
QT_MOC_LITERAL(17, 224, 11), // "onReadyRead"
QT_MOC_LITERAL(18, 236, 13), // "onSocketError"
QT_MOC_LITERAL(19, 250, 28), // "QAbstractSocket::SocketError"
QT_MOC_LITERAL(20, 279, 18), // "onHeartbeatTimeout"
QT_MOC_LITERAL(21, 298, 18), // "onReconnectTimeout"
QT_MOC_LITERAL(22, 317, 18) // "onHandshakeTimeout"

    },
    "flykylin::communication::TcpConnection\0"
    "stateChanged\0\0ConnectionState\0newState\0"
    "reason\0messageReceived\0data\0messageSent\0"
    "messageId\0messageFailed\0error\0"
    "errorOccurred\0handshakeCompleted\0"
    "handshakeFailed\0onConnected\0onDisconnected\0"
    "onReadyRead\0onSocketError\0"
    "QAbstractSocket::SocketError\0"
    "onHeartbeatTimeout\0onReconnectTimeout\0"
    "onHandshakeTimeout"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_flykylin__communication__TcpConnection[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   84,    2, 0x06 /* Public */,
       6,    1,   89,    2, 0x06 /* Public */,
       8,    1,   92,    2, 0x06 /* Public */,
      10,    2,   95,    2, 0x06 /* Public */,
      12,    1,  100,    2, 0x06 /* Public */,
      13,    0,  103,    2, 0x06 /* Public */,
      14,    1,  104,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      15,    0,  107,    2, 0x08 /* Private */,
      16,    0,  108,    2, 0x08 /* Private */,
      17,    0,  109,    2, 0x08 /* Private */,
      18,    1,  110,    2, 0x08 /* Private */,
      20,    0,  113,    2, 0x08 /* Private */,
      21,    0,  114,    2, 0x08 /* Private */,
      22,    0,  115,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QString,    4,    5,
    QMetaType::Void, QMetaType::QByteArray,    7,
    QMetaType::Void, QMetaType::ULongLong,    9,
    QMetaType::Void, QMetaType::ULongLong, QMetaType::QString,    9,   11,
    QMetaType::Void, QMetaType::QString,   11,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   11,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 19,   11,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void flykylin::communication::TcpConnection::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TcpConnection *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->stateChanged((*reinterpret_cast< ConnectionState(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 1: _t->messageReceived((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 2: _t->messageSent((*reinterpret_cast< quint64(*)>(_a[1]))); break;
        case 3: _t->messageFailed((*reinterpret_cast< quint64(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 4: _t->errorOccurred((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 5: _t->handshakeCompleted(); break;
        case 6: _t->handshakeFailed((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 7: _t->onConnected(); break;
        case 8: _t->onDisconnected(); break;
        case 9: _t->onReadyRead(); break;
        case 10: _t->onSocketError((*reinterpret_cast< QAbstractSocket::SocketError(*)>(_a[1]))); break;
        case 11: _t->onHeartbeatTimeout(); break;
        case 12: _t->onReconnectTimeout(); break;
        case 13: _t->onHandshakeTimeout(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 10:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QAbstractSocket::SocketError >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (TcpConnection::*)(ConnectionState , QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TcpConnection::stateChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (TcpConnection::*)(const QByteArray & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TcpConnection::messageReceived)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (TcpConnection::*)(quint64 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TcpConnection::messageSent)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (TcpConnection::*)(quint64 , QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TcpConnection::messageFailed)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (TcpConnection::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TcpConnection::errorOccurred)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (TcpConnection::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TcpConnection::handshakeCompleted)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (TcpConnection::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TcpConnection::handshakeFailed)) {
                *result = 6;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject flykylin::communication::TcpConnection::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_flykylin__communication__TcpConnection.data,
    qt_meta_data_flykylin__communication__TcpConnection,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *flykylin::communication::TcpConnection::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *flykylin::communication::TcpConnection::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_flykylin__communication__TcpConnection.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int flykylin::communication::TcpConnection::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    }
    return _id;
}

// SIGNAL 0
void flykylin::communication::TcpConnection::stateChanged(ConnectionState _t1, QString _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void flykylin::communication::TcpConnection::messageReceived(const QByteArray & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void flykylin::communication::TcpConnection::messageSent(quint64 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void flykylin::communication::TcpConnection::messageFailed(quint64 _t1, QString _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void flykylin::communication::TcpConnection::errorOccurred(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void flykylin::communication::TcpConnection::handshakeCompleted()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void flykylin::communication::TcpConnection::handshakeFailed(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
