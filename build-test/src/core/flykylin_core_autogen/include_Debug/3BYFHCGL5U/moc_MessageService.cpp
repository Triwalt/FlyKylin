/****************************************************************************
** Meta object code from reading C++ file 'MessageService.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../src/core/services/MessageService.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MessageService.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN8flykylin8services14MessageServiceE_t {};
} // unnamed namespace

template <> constexpr inline auto flykylin::services::MessageService::qt_create_metaobjectdata<qt_meta_tag_ZN8flykylin8services14MessageServiceE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "flykylin::services::MessageService",
        "messageReceived",
        "",
        "flykylin::core::Message",
        "message",
        "messageSent",
        "messageFailed",
        "error",
        "onTcpMessageReceived",
        "peerId",
        "data",
        "onTcpMessageSent",
        "messageId",
        "onTcpMessageFailed"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'messageReceived'
        QtMocHelpers::SignalData<void(const flykylin::core::Message &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'messageSent'
        QtMocHelpers::SignalData<void(const flykylin::core::Message &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'messageFailed'
        QtMocHelpers::SignalData<void(const flykylin::core::Message &, const QString &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { QMetaType::QString, 7 },
        }}),
        // Slot 'onTcpMessageReceived'
        QtMocHelpers::SlotData<void(QString, QByteArray)>(8, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 9 }, { QMetaType::QByteArray, 10 },
        }}),
        // Slot 'onTcpMessageSent'
        QtMocHelpers::SlotData<void(QString, quint64)>(11, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 9 }, { QMetaType::ULongLong, 12 },
        }}),
        // Slot 'onTcpMessageFailed'
        QtMocHelpers::SlotData<void(QString, quint64, QString)>(13, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 9 }, { QMetaType::ULongLong, 12 }, { QMetaType::QString, 7 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MessageService, qt_meta_tag_ZN8flykylin8services14MessageServiceE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject flykylin::services::MessageService::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8flykylin8services14MessageServiceE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8flykylin8services14MessageServiceE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN8flykylin8services14MessageServiceE_t>.metaTypes,
    nullptr
} };

void flykylin::services::MessageService::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MessageService *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->messageReceived((*reinterpret_cast< std::add_pointer_t<flykylin::core::Message>>(_a[1]))); break;
        case 1: _t->messageSent((*reinterpret_cast< std::add_pointer_t<flykylin::core::Message>>(_a[1]))); break;
        case 2: _t->messageFailed((*reinterpret_cast< std::add_pointer_t<flykylin::core::Message>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 3: _t->onTcpMessageReceived((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[2]))); break;
        case 4: _t->onTcpMessageSent((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<quint64>>(_a[2]))); break;
        case 5: _t->onTcpMessageFailed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<quint64>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< flykylin::core::Message >(); break;
            }
            break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< flykylin::core::Message >(); break;
            }
            break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< flykylin::core::Message >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (MessageService::*)(const flykylin::core::Message & )>(_a, &MessageService::messageReceived, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (MessageService::*)(const flykylin::core::Message & )>(_a, &MessageService::messageSent, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (MessageService::*)(const flykylin::core::Message & , const QString & )>(_a, &MessageService::messageFailed, 2))
            return;
    }
}

const QMetaObject *flykylin::services::MessageService::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *flykylin::services::MessageService::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8flykylin8services14MessageServiceE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int flykylin::services::MessageService::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void flykylin::services::MessageService::messageReceived(const flykylin::core::Message & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void flykylin::services::MessageService::messageSent(const flykylin::core::Message & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void flykylin::services::MessageService::messageFailed(const flykylin::core::Message & _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}
QT_WARNING_POP
