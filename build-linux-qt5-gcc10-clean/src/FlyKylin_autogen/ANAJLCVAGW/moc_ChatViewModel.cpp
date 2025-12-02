/****************************************************************************
** Meta object code from reading C++ file 'ChatViewModel.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/ui/viewmodels/ChatViewModel.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ChatViewModel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_flykylin__ui__ChatViewModel_t {
    QByteArrayData data[42];
    char stringdata0[535];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_flykylin__ui__ChatViewModel_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_flykylin__ui__ChatViewModel_t qt_meta_stringdata_flykylin__ui__ChatViewModel = {
    {
QT_MOC_LITERAL(0, 0, 27), // "flykylin::ui::ChatViewModel"
QT_MOC_LITERAL(1, 28, 11), // "peerChanged"
QT_MOC_LITERAL(2, 40, 0), // ""
QT_MOC_LITERAL(3, 41, 6), // "peerId"
QT_MOC_LITERAL(4, 48, 8), // "peerName"
QT_MOC_LITERAL(5, 57, 15), // "messagesUpdated"
QT_MOC_LITERAL(6, 73, 15), // "messageReceived"
QT_MOC_LITERAL(7, 89, 23), // "flykylin::core::Message"
QT_MOC_LITERAL(8, 113, 7), // "message"
QT_MOC_LITERAL(9, 121, 11), // "messageSent"
QT_MOC_LITERAL(10, 133, 13), // "messageFailed"
QT_MOC_LITERAL(11, 147, 5), // "error"
QT_MOC_LITERAL(12, 153, 17), // "onMessageReceived"
QT_MOC_LITERAL(13, 171, 13), // "onMessageSent"
QT_MOC_LITERAL(14, 185, 15), // "onMessageFailed"
QT_MOC_LITERAL(15, 201, 15), // "setCurrentGroup"
QT_MOC_LITERAL(16, 217, 7), // "groupId"
QT_MOC_LITERAL(17, 225, 9), // "groupName"
QT_MOC_LITERAL(18, 235, 9), // "memberIds"
QT_MOC_LITERAL(19, 245, 11), // "sendMessage"
QT_MOC_LITERAL(20, 257, 7), // "content"
QT_MOC_LITERAL(21, 265, 9), // "sendImage"
QT_MOC_LITERAL(22, 275, 8), // "filePath"
QT_MOC_LITERAL(23, 284, 8), // "sendFile"
QT_MOC_LITERAL(24, 293, 14), // "sendScreenshot"
QT_MOC_LITERAL(25, 308, 25), // "captureScreenForSelection"
QT_MOC_LITERAL(26, 334, 21), // "sendCroppedScreenshot"
QT_MOC_LITERAL(27, 356, 8), // "fullPath"
QT_MOC_LITERAL(28, 365, 1), // "x"
QT_MOC_LITERAL(29, 367, 1), // "y"
QT_MOC_LITERAL(30, 369, 5), // "width"
QT_MOC_LITERAL(31, 375, 6), // "height"
QT_MOC_LITERAL(32, 382, 18), // "deleteConversation"
QT_MOC_LITERAL(33, 401, 14), // "findMessageRow"
QT_MOC_LITERAL(34, 416, 9), // "messageId"
QT_MOC_LITERAL(35, 426, 11), // "isGroupChat"
QT_MOC_LITERAL(36, 438, 17), // "getCurrentGroupId"
QT_MOC_LITERAL(37, 456, 11), // "isImageNsfw"
QT_MOC_LITERAL(38, 468, 17), // "resetConversation"
QT_MOC_LITERAL(39, 486, 12), // "messageModel"
QT_MOC_LITERAL(40, 499, 19), // "QStandardItemModel*"
QT_MOC_LITERAL(41, 519, 15) // "currentPeerName"

    },
    "flykylin::ui::ChatViewModel\0peerChanged\0"
    "\0peerId\0peerName\0messagesUpdated\0"
    "messageReceived\0flykylin::core::Message\0"
    "message\0messageSent\0messageFailed\0"
    "error\0onMessageReceived\0onMessageSent\0"
    "onMessageFailed\0setCurrentGroup\0groupId\0"
    "groupName\0memberIds\0sendMessage\0content\0"
    "sendImage\0filePath\0sendFile\0sendScreenshot\0"
    "captureScreenForSelection\0"
    "sendCroppedScreenshot\0fullPath\0x\0y\0"
    "width\0height\0deleteConversation\0"
    "findMessageRow\0messageId\0isGroupChat\0"
    "getCurrentGroupId\0isImageNsfw\0"
    "resetConversation\0messageModel\0"
    "QStandardItemModel*\0currentPeerName"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_flykylin__ui__ChatViewModel[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      21,   14, // methods
       2,  188, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,  119,    2, 0x06 /* Public */,
       5,    0,  124,    2, 0x06 /* Public */,
       6,    1,  125,    2, 0x06 /* Public */,
       9,    1,  128,    2, 0x06 /* Public */,
      10,    2,  131,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      12,    1,  136,    2, 0x08 /* Private */,
      13,    1,  139,    2, 0x08 /* Private */,
      14,    2,  142,    2, 0x08 /* Private */,

 // methods: name, argc, parameters, tag, flags
      15,    3,  147,    2, 0x02 /* Public */,
      19,    1,  154,    2, 0x02 /* Public */,
      21,    1,  157,    2, 0x02 /* Public */,
      23,    1,  160,    2, 0x02 /* Public */,
      24,    0,  163,    2, 0x02 /* Public */,
      25,    0,  164,    2, 0x02 /* Public */,
      26,    5,  165,    2, 0x02 /* Public */,
      32,    1,  176,    2, 0x02 /* Public */,
      33,    1,  179,    2, 0x02 /* Public */,
      35,    0,  182,    2, 0x02 /* Public */,
      36,    0,  183,    2, 0x02 /* Public */,
      37,    1,  184,    2, 0x02 /* Public */,
      38,    0,  187,    2, 0x02 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    3,    4,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void, 0x80000000 | 7, QMetaType::QString,    8,   11,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void, 0x80000000 | 7, QMetaType::QString,    8,   11,

 // methods: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::QStringList,   16,   17,   18,
    QMetaType::Void, QMetaType::QString,   20,
    QMetaType::Void, QMetaType::QString,   22,
    QMetaType::Void, QMetaType::QString,   22,
    QMetaType::Void,
    QMetaType::QString,
    QMetaType::Void, QMetaType::QString, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Int,   27,   28,   29,   30,   31,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Int, QMetaType::QString,   34,
    QMetaType::Bool,
    QMetaType::QString,
    QMetaType::Bool, QMetaType::QString,   22,
    QMetaType::Void,

 // properties: name, type, flags
      39, 0x80000000 | 40, 0x00095409,
      41, QMetaType::QString, 0x00495001,

 // properties: notify_signal_id
       0,
       0,

       0        // eod
};

void flykylin::ui::ChatViewModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ChatViewModel *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->peerChanged((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 1: _t->messagesUpdated(); break;
        case 2: _t->messageReceived((*reinterpret_cast< const flykylin::core::Message(*)>(_a[1]))); break;
        case 3: _t->messageSent((*reinterpret_cast< const flykylin::core::Message(*)>(_a[1]))); break;
        case 4: _t->messageFailed((*reinterpret_cast< const flykylin::core::Message(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 5: _t->onMessageReceived((*reinterpret_cast< const flykylin::core::Message(*)>(_a[1]))); break;
        case 6: _t->onMessageSent((*reinterpret_cast< const flykylin::core::Message(*)>(_a[1]))); break;
        case 7: _t->onMessageFailed((*reinterpret_cast< const flykylin::core::Message(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 8: _t->setCurrentGroup((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QStringList(*)>(_a[3]))); break;
        case 9: _t->sendMessage((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 10: _t->sendImage((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 11: _t->sendFile((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 12: _t->sendScreenshot(); break;
        case 13: { QString _r = _t->captureScreenForSelection();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 14: _t->sendCroppedScreenshot((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4])),(*reinterpret_cast< int(*)>(_a[5]))); break;
        case 15: _t->deleteConversation((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 16: { int _r = _t->findMessageRow((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        case 17: { bool _r = _t->isGroupChat();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 18: { QString _r = _t->getCurrentGroupId();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 19: { bool _r = _t->isImageNsfw((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 20: _t->resetConversation(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< flykylin::core::Message >(); break;
            }
            break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< flykylin::core::Message >(); break;
            }
            break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< flykylin::core::Message >(); break;
            }
            break;
        case 5:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< flykylin::core::Message >(); break;
            }
            break;
        case 6:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< flykylin::core::Message >(); break;
            }
            break;
        case 7:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< flykylin::core::Message >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ChatViewModel::*)(QString , QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ChatViewModel::peerChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ChatViewModel::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ChatViewModel::messagesUpdated)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (ChatViewModel::*)(const flykylin::core::Message & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ChatViewModel::messageReceived)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (ChatViewModel::*)(const flykylin::core::Message & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ChatViewModel::messageSent)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (ChatViewModel::*)(const flykylin::core::Message & , QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ChatViewModel::messageFailed)) {
                *result = 4;
                return;
            }
        }
    } else if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QStandardItemModel* >(); break;
        }
    }

#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<ChatViewModel *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QStandardItemModel**>(_v) = _t->messageModel(); break;
        case 1: *reinterpret_cast< QString*>(_v) = _t->getCurrentPeerName(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject flykylin::ui::ChatViewModel::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_flykylin__ui__ChatViewModel.data,
    qt_meta_data_flykylin__ui__ChatViewModel,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *flykylin::ui::ChatViewModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *flykylin::ui::ChatViewModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_flykylin__ui__ChatViewModel.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int flykylin::ui::ChatViewModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 21)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 21;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 21)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 21;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 2;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void flykylin::ui::ChatViewModel::peerChanged(QString _t1, QString _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void flykylin::ui::ChatViewModel::messagesUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void flykylin::ui::ChatViewModel::messageReceived(const flykylin::core::Message & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void flykylin::ui::ChatViewModel::messageSent(const flykylin::core::Message & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void flykylin::ui::ChatViewModel::messageFailed(const flykylin::core::Message & _t1, QString _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
