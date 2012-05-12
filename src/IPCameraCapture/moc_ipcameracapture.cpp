/****************************************************************************
** Meta object code from reading C++ file 'ipcameracapture.h'
**
** Created: Thu Nov 17 16:43:44 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ipcameracapture.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ipcameracapture.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_IPCameraCapture[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      17,   16,   16,   16, 0x05,

 // slots: signature, parameters, type, tag, flags
      30,   25,   16,   16, 0x0a,
      63,   61,   16,   16, 0x0a,
      89,   16,   16,   16, 0x0a,
     116,   16,   16,   16, 0x0a,
     138,  132,   16,   16, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_IPCameraCapture[] = {
    "IPCameraCapture\0\0abort()\0resp\0"
    "readyRead(QHttpResponseHeader)\0,\0"
    "requestFinished(int,bool)\0"
    "ConnectToCamera(Camera_t*)\0StopCapturing()\0"
    "state\0httpStateChanged(int)\0"
};

const QMetaObject IPCameraCapture::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_IPCameraCapture,
      qt_meta_data_IPCameraCapture, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &IPCameraCapture::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *IPCameraCapture::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *IPCameraCapture::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_IPCameraCapture))
        return static_cast<void*>(const_cast< IPCameraCapture*>(this));
    if (!strcmp(_clname, "SignalProcessor"))
        return static_cast< SignalProcessor*>(const_cast< IPCameraCapture*>(this));
    return QObject::qt_metacast(_clname);
}

int IPCameraCapture::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: abort(); break;
        case 1: readyRead((*reinterpret_cast< const QHttpResponseHeader(*)>(_a[1]))); break;
        case 2: requestFinished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 3: ConnectToCamera((*reinterpret_cast< Camera_t*(*)>(_a[1]))); break;
        case 4: StopCapturing(); break;
        case 5: httpStateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void IPCameraCapture::abort()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
