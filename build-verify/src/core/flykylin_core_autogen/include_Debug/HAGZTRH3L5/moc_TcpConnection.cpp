/****************************************************************************
** Meta object code from reading C++ file 'TcpConnection.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../src/core/communication/TcpConnection.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TcpConnection.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN8flykylin13communication13TcpConnectionE_t {};
} // unnamed namespace

template <> constexpr inline auto flykylin::communication::TcpConnection::qt_create_metaobjectdata<qt_meta_tag_ZN8flykylin13communication13TcpConnectionE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "flykylin::communication::TcpConnection",
        "stateChanged",
        "",
        "ConnectionState",
        "newState",
        "reason",
        "messageReceived",
        "data",
        "messageSent",
        "messageId",
        "messageFailed",
        "error",
        "errorOccurred",
        "handshakeCompleted",
        "handshakeFailed",
        "onConnected",
        "onDisconnected",
        "onReadyRead",
        "onSocketError",
        "QAbstractSocket::SocketError",
        "onHeartbeatTimeout",
        "onReconnectTimeout",
        "onHandshakeTimeout"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'stateChanged'
        QtMocHelpers::SignalData<void(ConnectionState, QString)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { QMetaType::QString, 5 },
        }}),
        // Signal 'messageReceived'
        QtMocHelpers::SignalData<void(const QByteArray &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QByteArray, 7 },
        }}),
        // Signal 'messageSent'
        QtMocHelpers::SignalData<void(quint64)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::ULongLong, 9 },
        }}),
        // Signal 'messageFailed'
        QtMocHelpers::SignalData<void(quint64, QString)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::ULongLong, 9 }, { QMetaType::QString, 11 },
        }}),
        // Signal 'errorOccurred'
        QtMocHelpers::SignalData<void(QString)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 11 },
        }}),
        // Signal 'handshakeCompleted'
        QtMocHelpers::SignalData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'handshakeFailed'
        QtMocHelpers::SignalData<void(QString)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 11 },
        }}),
        // Slot 'onConnected'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onDisconnected'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onReadyRead'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSocketError'
        QtMocHelpers::SlotData<void(QAbstractSocket::SocketError)>(18, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 19, 11 },
        }}),
        // Slot 'onHeartbeatTimeout'
        QtMocHelpers::SlotData<void()>(20, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onReconnectTimeout'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onHandshakeTimeout'
        QtMocHelpers::SlotData<void()>(22, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<TcpConnection, qt_meta_tag_ZN8flykylin13communication13TcpConnectionE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject flykylin::communication::TcpConnection::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8flykylin13communication13TcpConnectionE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8flykylin13communication13TcpConnectionE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN8flykylin13communication13TcpConnectionE_t>.metaTypes,
    nullptr
} };

void flykylin::communication::TcpConnection::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<TcpConnection *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->stateChanged((*reinterpret_cast< std::add_pointer_t<ConnectionState>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 1: _t->messageReceived((*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[1]))); break;
        case 2: _t->messageSent((*reinterpret_cast< std::add_pointer_t<quint64>>(_a[1]))); break;
        case 3: _t->messageFailed((*reinterpret_cast< std::add_pointer_t<quint64>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 4: _t->errorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->handshakeCompleted(); break;
        case 6: _t->handshakeFailed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->onConnected(); break;
        case 8: _t->onDisconnected(); break;
        case 9: _t->onReadyRead(); break;
        case 10: _t->onSocketError((*reinterpret_cast< std::add_pointer_t<QAbstractSocket::SocketError>>(_a[1]))); break;
        case 11: _t->onHeartbeatTimeout(); break;
        case 12: _t->onReconnectTimeout(); break;
        case 13: _t->onHandshakeTimeout(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 10:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QAbstractSocket::SocketError >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (TcpConnection::*)(ConnectionState , QString )>(_a, &TcpConnection::stateChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (TcpConnection::*)(const QByteArray & )>(_a, &TcpConnection::messageReceived, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (TcpConnection::*)(quint64 )>(_a, &TcpConnection::messageSent, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (TcpConnection::*)(quint64 , QString )>(_a, &TcpConnection::messageFailed, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (TcpConnection::*)(QString )>(_a, &TcpConnection::errorOccurred, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (TcpConnection::*)()>(_a, &TcpConnection::handshakeCompleted, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (TcpConnection::*)(QString )>(_a, &TcpConnection::handshakeFailed, 6))
            return;
    }
}

const QMetaObject *flykylin::communication::TcpConnection::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *flykylin::communication::TcpConnection::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8flykylin13communication13TcpConnectionE_t>.strings))
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
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    }
    return _id;
}

// SIGNAL 0
void flykylin::communication::TcpConnection::stateChanged(ConnectionState _t1, QString _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void flykylin::communication::TcpConnection::messageReceived(const QByteArray & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void flykylin::communication::TcpConnection::messageSent(quint64 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void flykylin::communication::TcpConnection::messageFailed(quint64 _t1, QString _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}

// SIGNAL 4
void flykylin::communication::TcpConnection::errorOccurred(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void flykylin::communication::TcpConnection::handshakeCompleted()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void flykylin::communication::TcpConnection::handshakeFailed(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}
QT_WARNING_POP
