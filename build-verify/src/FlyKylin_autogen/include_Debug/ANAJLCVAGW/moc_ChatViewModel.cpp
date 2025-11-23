/****************************************************************************
** Meta object code from reading C++ file 'ChatViewModel.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/ui/viewmodels/ChatViewModel.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ChatViewModel.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN8flykylin2ui13ChatViewModelE_t {};
} // unnamed namespace

template <> constexpr inline auto flykylin::ui::ChatViewModel::qt_create_metaobjectdata<qt_meta_tag_ZN8flykylin2ui13ChatViewModelE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "flykylin::ui::ChatViewModel",
        "peerChanged",
        "",
        "peerId",
        "peerName",
        "messagesUpdated",
        "messageReceived",
        "flykylin::core::Message",
        "message",
        "messageSent",
        "messageFailed",
        "error",
        "onMessageReceived",
        "onMessageSent",
        "onMessageFailed"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'peerChanged'
        QtMocHelpers::SignalData<void(QString, QString)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::QString, 4 },
        }}),
        // Signal 'messagesUpdated'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'messageReceived'
        QtMocHelpers::SignalData<void(const flykylin::core::Message &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 7, 8 },
        }}),
        // Signal 'messageSent'
        QtMocHelpers::SignalData<void(const flykylin::core::Message &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 7, 8 },
        }}),
        // Signal 'messageFailed'
        QtMocHelpers::SignalData<void(const flykylin::core::Message &, QString)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 7, 8 }, { QMetaType::QString, 11 },
        }}),
        // Slot 'onMessageReceived'
        QtMocHelpers::SlotData<void(const flykylin::core::Message &)>(12, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 7, 8 },
        }}),
        // Slot 'onMessageSent'
        QtMocHelpers::SlotData<void(const flykylin::core::Message &)>(13, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 7, 8 },
        }}),
        // Slot 'onMessageFailed'
        QtMocHelpers::SlotData<void(const flykylin::core::Message &, const QString &)>(14, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 7, 8 }, { QMetaType::QString, 11 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ChatViewModel, qt_meta_tag_ZN8flykylin2ui13ChatViewModelE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject flykylin::ui::ChatViewModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8flykylin2ui13ChatViewModelE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8flykylin2ui13ChatViewModelE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN8flykylin2ui13ChatViewModelE_t>.metaTypes,
    nullptr
} };

void flykylin::ui::ChatViewModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ChatViewModel *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->peerChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 1: _t->messagesUpdated(); break;
        case 2: _t->messageReceived((*reinterpret_cast< std::add_pointer_t<flykylin::core::Message>>(_a[1]))); break;
        case 3: _t->messageSent((*reinterpret_cast< std::add_pointer_t<flykylin::core::Message>>(_a[1]))); break;
        case 4: _t->messageFailed((*reinterpret_cast< std::add_pointer_t<flykylin::core::Message>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 5: _t->onMessageReceived((*reinterpret_cast< std::add_pointer_t<flykylin::core::Message>>(_a[1]))); break;
        case 6: _t->onMessageSent((*reinterpret_cast< std::add_pointer_t<flykylin::core::Message>>(_a[1]))); break;
        case 7: _t->onMessageFailed((*reinterpret_cast< std::add_pointer_t<flykylin::core::Message>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< flykylin::core::Message >(); break;
            }
            break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< flykylin::core::Message >(); break;
            }
            break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< flykylin::core::Message >(); break;
            }
            break;
        case 5:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< flykylin::core::Message >(); break;
            }
            break;
        case 6:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< flykylin::core::Message >(); break;
            }
            break;
        case 7:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< flykylin::core::Message >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ChatViewModel::*)(QString , QString )>(_a, &ChatViewModel::peerChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ChatViewModel::*)()>(_a, &ChatViewModel::messagesUpdated, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ChatViewModel::*)(const flykylin::core::Message & )>(_a, &ChatViewModel::messageReceived, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (ChatViewModel::*)(const flykylin::core::Message & )>(_a, &ChatViewModel::messageSent, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (ChatViewModel::*)(const flykylin::core::Message & , QString )>(_a, &ChatViewModel::messageFailed, 4))
            return;
    }
}

const QMetaObject *flykylin::ui::ChatViewModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *flykylin::ui::ChatViewModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8flykylin2ui13ChatViewModelE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int flykylin::ui::ChatViewModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void flykylin::ui::ChatViewModel::peerChanged(QString _t1, QString _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void flykylin::ui::ChatViewModel::messagesUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void flykylin::ui::ChatViewModel::messageReceived(const flykylin::core::Message & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void flykylin::ui::ChatViewModel::messageSent(const flykylin::core::Message & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void flykylin::ui::ChatViewModel::messageFailed(const flykylin::core::Message & _t1, QString _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2);
}
QT_WARNING_POP
