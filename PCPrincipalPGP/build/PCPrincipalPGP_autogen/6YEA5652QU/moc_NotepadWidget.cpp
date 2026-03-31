/****************************************************************************
** Meta object code from reading C++ file 'NotepadWidget.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../include/NotepadWidget.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NotepadWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.1. It"
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
struct qt_meta_tag_ZN5PCPGP10CodeEditorE_t {};
} // unnamed namespace

template <> constexpr inline auto PCPGP::CodeEditor::qt_create_metaobjectdata<qt_meta_tag_ZN5PCPGP10CodeEditorE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "PCPGP::CodeEditor",
        "updateLineNumberAreaWidth",
        "",
        "updateLineNumberArea",
        "QRect",
        "rect",
        "dy"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'updateLineNumberAreaWidth'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'updateLineNumberArea'
        QtMocHelpers::SlotData<void(const QRect &, int)>(3, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 4, 5 }, { QMetaType::Int, 6 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<CodeEditor, qt_meta_tag_ZN5PCPGP10CodeEditorE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject PCPGP::CodeEditor::staticMetaObject = { {
    QMetaObject::SuperData::link<QPlainTextEdit::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN5PCPGP10CodeEditorE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN5PCPGP10CodeEditorE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN5PCPGP10CodeEditorE_t>.metaTypes,
    nullptr
} };

void PCPGP::CodeEditor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<CodeEditor *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->updateLineNumberAreaWidth(); break;
        case 1: _t->updateLineNumberArea((*reinterpret_cast<std::add_pointer_t<QRect>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObject *PCPGP::CodeEditor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PCPGP::CodeEditor::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN5PCPGP10CodeEditorE_t>.strings))
        return static_cast<void*>(this);
    return QPlainTextEdit::qt_metacast(_clname);
}

int PCPGP::CodeEditor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QPlainTextEdit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 2;
    }
    return _id;
}
namespace {
struct qt_meta_tag_ZN5PCPGP13NotepadWidgetE_t {};
} // unnamed namespace

template <> constexpr inline auto PCPGP::NotepadWidget::qt_create_metaobjectdata<qt_meta_tag_ZN5PCPGP13NotepadWidgetE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "PCPGP::NotepadWidget",
        "textChanged",
        "",
        "textModified",
        "modified",
        "encryptionRequested",
        "decryptionRequested",
        "signingRequested",
        "verificationRequested",
        "statusMessage",
        "message",
        "onCut",
        "onCopy",
        "onPaste",
        "onSelectAll",
        "onUndo",
        "onRedo",
        "onFind",
        "onReplace",
        "onZoomIn",
        "onZoomOut",
        "onResetZoom",
        "onWordWrap",
        "wrap",
        "onEncryptClicked",
        "onDecryptClicked",
        "onSignClicked",
        "onVerifyClicked",
        "onClearSignClicked",
        "onTextModified",
        "onProfileChanged",
        "index",
        "onInsertPublicKey",
        "onFormatPGPBlock"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'textChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'textModified'
        QtMocHelpers::SignalData<void(bool)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 4 },
        }}),
        // Signal 'encryptionRequested'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'decryptionRequested'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'signingRequested'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'verificationRequested'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'statusMessage'
        QtMocHelpers::SignalData<void(const QString &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 },
        }}),
        // Slot 'onCut'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onCopy'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onPaste'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onSelectAll'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onUndo'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onRedo'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onFind'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onReplace'
        QtMocHelpers::SlotData<void()>(18, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onZoomIn'
        QtMocHelpers::SlotData<void()>(19, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onZoomOut'
        QtMocHelpers::SlotData<void()>(20, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onResetZoom'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onWordWrap'
        QtMocHelpers::SlotData<void(bool)>(22, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 23 },
        }}),
        // Slot 'onEncryptClicked'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onDecryptClicked'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onSignClicked'
        QtMocHelpers::SlotData<void()>(26, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onVerifyClicked'
        QtMocHelpers::SlotData<void()>(27, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onClearSignClicked'
        QtMocHelpers::SlotData<void()>(28, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onTextModified'
        QtMocHelpers::SlotData<void()>(29, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onProfileChanged'
        QtMocHelpers::SlotData<void(int)>(30, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 31 },
        }}),
        // Slot 'onInsertPublicKey'
        QtMocHelpers::SlotData<void()>(32, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onFormatPGPBlock'
        QtMocHelpers::SlotData<void()>(33, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<NotepadWidget, qt_meta_tag_ZN5PCPGP13NotepadWidgetE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject PCPGP::NotepadWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN5PCPGP13NotepadWidgetE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN5PCPGP13NotepadWidgetE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN5PCPGP13NotepadWidgetE_t>.metaTypes,
    nullptr
} };

void PCPGP::NotepadWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<NotepadWidget *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->textChanged(); break;
        case 1: _t->textModified((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 2: _t->encryptionRequested(); break;
        case 3: _t->decryptionRequested(); break;
        case 4: _t->signingRequested(); break;
        case 5: _t->verificationRequested(); break;
        case 6: _t->statusMessage((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->onCut(); break;
        case 8: _t->onCopy(); break;
        case 9: _t->onPaste(); break;
        case 10: _t->onSelectAll(); break;
        case 11: _t->onUndo(); break;
        case 12: _t->onRedo(); break;
        case 13: _t->onFind(); break;
        case 14: _t->onReplace(); break;
        case 15: _t->onZoomIn(); break;
        case 16: _t->onZoomOut(); break;
        case 17: _t->onResetZoom(); break;
        case 18: _t->onWordWrap((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 19: _t->onEncryptClicked(); break;
        case 20: _t->onDecryptClicked(); break;
        case 21: _t->onSignClicked(); break;
        case 22: _t->onVerifyClicked(); break;
        case 23: _t->onClearSignClicked(); break;
        case 24: _t->onTextModified(); break;
        case 25: _t->onProfileChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 26: _t->onInsertPublicKey(); break;
        case 27: _t->onFormatPGPBlock(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (NotepadWidget::*)()>(_a, &NotepadWidget::textChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (NotepadWidget::*)(bool )>(_a, &NotepadWidget::textModified, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (NotepadWidget::*)()>(_a, &NotepadWidget::encryptionRequested, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (NotepadWidget::*)()>(_a, &NotepadWidget::decryptionRequested, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (NotepadWidget::*)()>(_a, &NotepadWidget::signingRequested, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (NotepadWidget::*)()>(_a, &NotepadWidget::verificationRequested, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (NotepadWidget::*)(const QString & )>(_a, &NotepadWidget::statusMessage, 6))
            return;
    }
}

const QMetaObject *PCPGP::NotepadWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PCPGP::NotepadWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN5PCPGP13NotepadWidgetE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int PCPGP::NotepadWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 28)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 28;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 28)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 28;
    }
    return _id;
}

// SIGNAL 0
void PCPGP::NotepadWidget::textChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void PCPGP::NotepadWidget::textModified(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void PCPGP::NotepadWidget::encryptionRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void PCPGP::NotepadWidget::decryptionRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void PCPGP::NotepadWidget::signingRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void PCPGP::NotepadWidget::verificationRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void PCPGP::NotepadWidget::statusMessage(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}
QT_WARNING_POP
