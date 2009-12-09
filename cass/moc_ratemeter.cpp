/****************************************************************************
** Meta object code from reading C++ file 'ratemeter.h'
**
** Created: Wed Dec 9 08:22:59 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ratemeter.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ratemeter.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_cass__Ratemeter[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // slots: signature, parameters, type, tag, flags
      17,   16,   16,   16, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_cass__Ratemeter[] = {
    "cass::Ratemeter\0\0nextEvent()\0"
};

const QMetaObject cass::Ratemeter::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_cass__Ratemeter,
      qt_meta_data_cass__Ratemeter, 0 }
};

const QMetaObject *cass::Ratemeter::metaObject() const
{
    return &staticMetaObject;
}

void *cass::Ratemeter::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_cass__Ratemeter))
        return static_cast<void*>(const_cast< Ratemeter*>(this));
    return QObject::qt_metacast(_clname);
}

int cass::Ratemeter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: nextEvent(); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
