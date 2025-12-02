/****************************************************************************
** Meta object code from reading C++ file 'ChatWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/ui/windows/ChatWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ChatWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_flykylin__ui__ChatWindow_t {
    QByteArrayData data[13];
    char stringdata0[206];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_flykylin__ui__ChatWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_flykylin__ui__ChatWindow_t qt_meta_stringdata_flykylin__ui__ChatWindow = {
    {
QT_MOC_LITERAL(0, 0, 24), // "flykylin::ui::ChatWindow"
QT_MOC_LITERAL(1, 25, 6), // "closed"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 19), // "onSendButtonClicked"
QT_MOC_LITERAL(4, 53, 24), // "onSendImageButtonClicked"
QT_MOC_LITERAL(5, 78, 23), // "onSendFileButtonClicked"
QT_MOC_LITERAL(6, 102, 17), // "onMessagesUpdated"
QT_MOC_LITERAL(7, 120, 17), // "onMessageReceived"
QT_MOC_LITERAL(8, 138, 23), // "flykylin::core::Message"
QT_MOC_LITERAL(9, 162, 7), // "message"
QT_MOC_LITERAL(10, 170, 13), // "onMessageSent"
QT_MOC_LITERAL(11, 184, 15), // "onMessageFailed"
QT_MOC_LITERAL(12, 200, 5) // "error"

    },
    "flykylin::ui::ChatWindow\0closed\0\0"
    "onSendButtonClicked\0onSendImageButtonClicked\0"
    "onSendFileButtonClicked\0onMessagesUpdated\0"
    "onMessageReceived\0flykylin::core::Message\0"
    "message\0onMessageSent\0onMessageFailed\0"
    "error"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_flykylin__ui__ChatWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   54,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    0,   55,    2, 0x08 /* Private */,
       4,    0,   56,    2, 0x08 /* Private */,
       5,    0,   57,    2, 0x08 /* Private */,
       6,    0,   58,    2, 0x08 /* Private */,
       7,    1,   59,    2, 0x08 /* Private */,
      10,    1,   62,    2, 0x08 /* Private */,
      11,    2,   65,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, 0x80000000 | 8, QMetaType::QString,    9,   12,

       0        // eod
};

void flykylin::ui::ChatWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ChatWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->closed(); break;
        case 1: _t->onSendButtonClicked(); break;
        case 2: _t->onSendImageButtonClicked(); break;
        case 3: _t->onSendFileButtonClicked(); break;
        case 4: _t->onMessagesUpdated(); break;
        case 5: _t->onMessageReceived((*reinterpret_cast< const flykylin::core::Message(*)>(_a[1]))); break;
        case 6: _t->onMessageSent((*reinterpret_cast< const flykylin::core::Message(*)>(_a[1]))); break;
        case 7: _t->onMessageFailed((*reinterpret_cast< const flykylin::core::Message(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
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
            using _t = void (ChatWindow::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ChatWindow::closed)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject flykylin::ui::ChatWindow::staticMetaObject = { {
    &QWidget::staticMetaObject,
    qt_meta_stringdata_flykylin__ui__ChatWindow.data,
    qt_meta_data_flykylin__ui__ChatWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *flykylin::ui::ChatWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *flykylin::ui::ChatWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_flykylin__ui__ChatWindow.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int flykylin::ui::ChatWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void flykylin::ui::ChatWindow::closed()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
