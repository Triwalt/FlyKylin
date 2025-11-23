/****************************************************************************
** Meta object code from reading C++ file 'TcpConnectionManager.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../src/core/communication/TcpConnectionManager.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TcpConnectionManager.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN8flykylin13communication20TcpConnectionManagerE_t {};
} // unnamed namespace

template <> constexpr inline auto flykylin::communication::TcpConnectionManager::qt_create_metaobjectdata<qt_meta_tag_ZN8flykylin13communication20TcpConnectionManagerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "flykylin::communication::TcpConnectionManager",
        "connectionStateChanged",
        "",
        "peerId",
        "ConnectionState",
        "state",
        "reason",
        "messageReceived",
        "data",
        "messageSent",
        "messageId",
        "messageFailed",
        "error",
        "onConnectionStateChanged",
        "onMessageReceived",
        "onMessageSent",
        "onMessageFailed",
        "cleanupIdleConnections",
        "processMessageQueue",
        "onPeerDiscovered",
        "flykylin::core::PeerNode",
        "node"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'connectionStateChanged'
        QtMocHelpers::SignalData<void(QString, ConnectionState, QString)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { 0x80000000 | 4, 5 }, { QMetaType::QString, 6 },
        }}),
        // Signal 'messageReceived'
        QtMocHelpers::SignalData<void(QString, QByteArray)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::QByteArray, 8 },
        }}),
        // Signal 'messageSent'
        QtMocHelpers::SignalData<void(QString, quint64)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::ULongLong, 10 },
        }}),
        // Signal 'messageFailed'
        QtMocHelpers::SignalData<void(QString, quint64, QString)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::ULongLong, 10 }, { QMetaType::QString, 12 },
        }}),
        // Slot 'onConnectionStateChanged'
        QtMocHelpers::SlotData<void(ConnectionState, QString)>(13, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 4, 5 }, { QMetaType::QString, 6 },
        }}),
        // Slot 'onMessageReceived'
        QtMocHelpers::SlotData<void(const QByteArray &)>(14, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QByteArray, 8 },
        }}),
        // Slot 'onMessageSent'
        QtMocHelpers::SlotData<void(quint64)>(15, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::ULongLong, 10 },
        }}),
        // Slot 'onMessageFailed'
        QtMocHelpers::SlotData<void(quint64, QString)>(16, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::ULongLong, 10 }, { QMetaType::QString, 12 },
        }}),
        // Slot 'cleanupIdleConnections'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'processMessageQueue'
        QtMocHelpers::SlotData<void(const QString &)>(18, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Slot 'onPeerDiscovered'
        QtMocHelpers::SlotData<void(const flykylin::core::PeerNode &)>(19, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 20, 21 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<TcpConnectionManager, qt_meta_tag_ZN8flykylin13communication20TcpConnectionManagerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject flykylin::communication::TcpConnectionManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8flykylin13communication20TcpConnectionManagerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8flykylin13communication20TcpConnectionManagerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN8flykylin13communication20TcpConnectionManagerE_t>.metaTypes,
    nullptr
} };

void flykylin::communication::TcpConnectionManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<TcpConnectionManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->connectionStateChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<ConnectionState>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 1: _t->messageReceived((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[2]))); break;
        case 2: _t->messageSent((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<quint64>>(_a[2]))); break;
        case 3: _t->messageFailed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<quint64>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 4: _t->onConnectionStateChanged((*reinterpret_cast< std::add_pointer_t<ConnectionState>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 5: _t->onMessageReceived((*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[1]))); break;
        case 6: _t->onMessageSent((*reinterpret_cast< std::add_pointer_t<quint64>>(_a[1]))); break;
        case 7: _t->onMessageFailed((*reinterpret_cast< std::add_pointer_t<quint64>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 8: _t->cleanupIdleConnections(); break;
        case 9: _t->processMessageQueue((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 10: _t->onPeerDiscovered((*reinterpret_cast< std::add_pointer_t<flykylin::core::PeerNode>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (TcpConnectionManager::*)(QString , ConnectionState , QString )>(_a, &TcpConnectionManager::connectionStateChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (TcpConnectionManager::*)(QString , QByteArray )>(_a, &TcpConnectionManager::messageReceived, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (TcpConnectionManager::*)(QString , quint64 )>(_a, &TcpConnectionManager::messageSent, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (TcpConnectionManager::*)(QString , quint64 , QString )>(_a, &TcpConnectionManager::messageFailed, 3))
            return;
    }
}

const QMetaObject *flykylin::communication::TcpConnectionManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *flykylin::communication::TcpConnectionManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8flykylin13communication20TcpConnectionManagerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int flykylin::communication::TcpConnectionManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void flykylin::communication::TcpConnectionManager::connectionStateChanged(QString _t1, ConnectionState _t2, QString _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2, _t3);
}

// SIGNAL 1
void flykylin::communication::TcpConnectionManager::messageReceived(QString _t1, QByteArray _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}

// SIGNAL 2
void flykylin::communication::TcpConnectionManager::messageSent(QString _t1, quint64 _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void flykylin::communication::TcpConnectionManager::messageFailed(QString _t1, quint64 _t2, QString _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2, _t3);
}
QT_WARNING_POP
