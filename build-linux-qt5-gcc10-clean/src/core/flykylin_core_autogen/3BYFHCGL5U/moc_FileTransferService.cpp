/****************************************************************************
** Meta object code from reading C++ file 'FileTransferService.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/core/services/FileTransferService.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FileTransferService.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_flykylin__services__FileTransferService_t {
    QByteArrayData data[13];
    char stringdata0[197];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_flykylin__services__FileTransferService_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_flykylin__services__FileTransferService_t qt_meta_stringdata_flykylin__services__FileTransferService = {
    {
QT_MOC_LITERAL(0, 0, 39), // "flykylin::services::FileTrans..."
QT_MOC_LITERAL(1, 40, 14), // "messageCreated"
QT_MOC_LITERAL(2, 55, 0), // ""
QT_MOC_LITERAL(3, 56, 23), // "flykylin::core::Message"
QT_MOC_LITERAL(4, 80, 7), // "message"
QT_MOC_LITERAL(5, 88, 14), // "transferFailed"
QT_MOC_LITERAL(6, 103, 10), // "transferId"
QT_MOC_LITERAL(7, 114, 5), // "error"
QT_MOC_LITERAL(8, 120, 17), // "transferCompleted"
QT_MOC_LITERAL(9, 138, 25), // "incomingTransferRequested"
QT_MOC_LITERAL(10, 164, 6), // "peerId"
QT_MOC_LITERAL(11, 171, 20), // "onTcpMessageReceived"
QT_MOC_LITERAL(12, 192, 4) // "data"

    },
    "flykylin::services::FileTransferService\0"
    "messageCreated\0\0flykylin::core::Message\0"
    "message\0transferFailed\0transferId\0"
    "error\0transferCompleted\0"
    "incomingTransferRequested\0peerId\0"
    "onTcpMessageReceived\0data"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_flykylin__services__FileTransferService[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x06 /* Public */,
       5,    2,   42,    2, 0x06 /* Public */,
       8,    2,   47,    2, 0x06 /* Public */,
       9,    3,   52,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      11,    2,   59,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    6,    7,
    QMetaType::Void, QMetaType::QString, 0x80000000 | 3,    6,    4,
    QMetaType::Void, QMetaType::QString, QMetaType::QString, 0x80000000 | 3,    6,   10,    4,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::QByteArray,   10,   12,

       0        // eod
};

void flykylin::services::FileTransferService::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FileTransferService *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->messageCreated((*reinterpret_cast< const flykylin::core::Message(*)>(_a[1]))); break;
        case 1: _t->transferFailed((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 2: _t->transferCompleted((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< const flykylin::core::Message(*)>(_a[2]))); break;
        case 3: _t->incomingTransferRequested((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< const flykylin::core::Message(*)>(_a[3]))); break;
        case 4: _t->onTcpMessageReceived((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QByteArray(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< flykylin::core::Message >(); break;
            }
            break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 1:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< flykylin::core::Message >(); break;
            }
            break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 2:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< flykylin::core::Message >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (FileTransferService::*)(const flykylin::core::Message & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FileTransferService::messageCreated)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (FileTransferService::*)(QString , QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FileTransferService::transferFailed)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (FileTransferService::*)(QString , const flykylin::core::Message & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FileTransferService::transferCompleted)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (FileTransferService::*)(QString , QString , const flykylin::core::Message & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FileTransferService::incomingTransferRequested)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject flykylin::services::FileTransferService::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_flykylin__services__FileTransferService.data,
    qt_meta_data_flykylin__services__FileTransferService,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *flykylin::services::FileTransferService::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *flykylin::services::FileTransferService::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_flykylin__services__FileTransferService.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int flykylin::services::FileTransferService::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void flykylin::services::FileTransferService::messageCreated(const flykylin::core::Message & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void flykylin::services::FileTransferService::transferFailed(QString _t1, QString _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void flykylin::services::FileTransferService::transferCompleted(QString _t1, const flykylin::core::Message & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void flykylin::services::FileTransferService::incomingTransferRequested(QString _t1, QString _t2, const flykylin::core::Message & _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
