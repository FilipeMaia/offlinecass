/****************************************************************************
** Meta object code from reading C++ file 'analyzer.h'
**
** Created: Wed Dec 9 08:22:59 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "analyzer.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'analyzer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_cass__Analyzer[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // slots: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x0a,
      47,   15,   15,   15, 0x0a,
      62,   15,   15,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_cass__Analyzer[] = {
    "cass::Analyzer\0\0processEvent(cass::CASSEvent*)\0"
    "saveSettings()\0loadSettings()\0"
};

const QMetaObject cass::Analyzer::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_cass__Analyzer,
      qt_meta_data_cass__Analyzer, 0 }
};

const QMetaObject *cass::Analyzer::metaObject() const
{
    return &staticMetaObject;
}

void *cass::Analyzer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_cass__Analyzer))
        return static_cast<void*>(const_cast< Analyzer*>(this));
    return QObject::qt_metacast(_clname);
}

int cass::Analyzer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: processEvent((*reinterpret_cast< cass::CASSEvent*(*)>(_a[1]))); break;
        case 1: saveSettings(); break;
        case 2: loadSettings(); break;
        default: ;
        }
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
