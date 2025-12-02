/****************************************************************************
** Meta object code from reading C++ file 'PeerListWidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/ui/widgets/PeerListWidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PeerListWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_flykylin__ui__PeerListWidget_t {
    QByteArrayData data[12];
    char stringdata0[163];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_flykylin__ui__PeerListWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_flykylin__ui__PeerListWidget_t qt_meta_stringdata_flykylin__ui__PeerListWidget = {
    {
QT_MOC_LITERAL(0, 0, 28), // "flykylin::ui::PeerListWidget"
QT_MOC_LITERAL(1, 29, 12), // "userSelected"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 6), // "userId"
QT_MOC_LITERAL(4, 50, 17), // "userDoubleClicked"
QT_MOC_LITERAL(5, 68, 19), // "onSearchTextChanged"
QT_MOC_LITERAL(6, 88, 4), // "text"
QT_MOC_LITERAL(7, 93, 13), // "onItemClicked"
QT_MOC_LITERAL(8, 107, 11), // "QModelIndex"
QT_MOC_LITERAL(9, 119, 5), // "index"
QT_MOC_LITERAL(10, 125, 19), // "onItemDoubleClicked"
QT_MOC_LITERAL(11, 145, 17) // "updateOnlineCount"

    },
    "flykylin::ui::PeerListWidget\0userSelected\0"
    "\0userId\0userDoubleClicked\0onSearchTextChanged\0"
    "text\0onItemClicked\0QModelIndex\0index\0"
    "onItemDoubleClicked\0updateOnlineCount"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_flykylin__ui__PeerListWidget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x06 /* Public */,
       4,    1,   47,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    1,   50,    2, 0x08 /* Private */,
       7,    1,   53,    2, 0x08 /* Private */,
      10,    1,   56,    2, 0x08 /* Private */,
      11,    0,   59,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString,    3,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void,

       0        // eod
};

void flykylin::ui::PeerListWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PeerListWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->userSelected((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->userDoubleClicked((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->onSearchTextChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->onItemClicked((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 4: _t->onItemDoubleClicked((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 5: _t->updateOnlineCount(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PeerListWidget::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PeerListWidget::userSelected)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (PeerListWidget::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PeerListWidget::userDoubleClicked)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject flykylin::ui::PeerListWidget::staticMetaObject = { {
    &QWidget::staticMetaObject,
    qt_meta_stringdata_flykylin__ui__PeerListWidget.data,
    qt_meta_data_flykylin__ui__PeerListWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *flykylin::ui::PeerListWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *flykylin::ui::PeerListWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_flykylin__ui__PeerListWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int flykylin::ui::PeerListWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
void flykylin::ui::PeerListWidget::userSelected(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void flykylin::ui::PeerListWidget::userDoubleClicked(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
