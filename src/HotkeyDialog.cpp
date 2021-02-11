#include "HotkeyDialog.hpp"

#include <QDialogButtonBox>
#include <QGuiApplication>
#include <QGridLayout>
#include <QPushButton>
#include <qevent.h>
#include <QDebug>
#include <QLabel>

HotkeyDialog::HotkeyDialog(QWidget *parent)
    : QDialog(parent)
    , m_label(new QLabel)
    , m_clear(new QPushButton("Clear"))
    , m_record(new QPushButton("Record"))
{
    m_record->setCheckable(true);

    auto bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(m_clear, &QPushButton::clicked,
            this, [this] {
        m_label->clear();
        m_record->setChecked(false);
        m_keySeq = {};
    });

    connect(bb, &QDialogButtonBox::accepted,
            this, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected,
            this, &QDialog::reject);

    connect(this, &QDialog::finished,
            this, [=](int result) {
        Q_UNUSED(result)
        m_record->setChecked(false);
    });

    auto l = new QGridLayout(this);
    l->addWidget(m_label, 0, 0, 1, 2);
    l->addWidget(m_clear, 1, 0, 1, 1);
    l->addWidget(m_record, 1, 1, 1, 1);
    l->addWidget(bb, 2, 0, 1, 2);

    m_record->setFocus();
}
HotkeyDialog::~HotkeyDialog()
{
}

bool HotkeyDialog::setKeySequence(const KeySequence &keySeq)
{
    if (keySeq.text.isEmpty() || !keySeq.mod || !keySeq.key)
        return false;

    m_label->setText(keySeq.text);
    m_keySeq = keySeq;
    return true;
}
KeySequence HotkeyDialog::getKeySequence() const
{
    return m_keySeq;
}

bool HotkeyDialog::event(QEvent *e)
{
    if (m_record->isChecked() && e->type() == QEvent::KeyPress)
    {
        auto ke = reinterpret_cast<QKeyEvent *>(e);

        bool keyIsModifier = false;
        switch (ke->key())
        {
            case Qt::Key_Shift:
            case Qt::Key_Control:
            case Qt::Key_Meta:
            case Qt::Key_Alt:
            case Qt::Key_Super_L:
            case Qt::Key_Super_R:
                keyIsModifier = true;
                break;
        }

        if (!keyIsModifier && !ke->isAutoRepeat())
        {
            QString shortcutStr;

            auto appendModifierStr = [&](Qt::KeyboardModifiers modifier) {
                QString text;
                switch (modifier)
                {
                    case Qt::ShiftModifier:
                        text = "Shift";
                        break;
                    case Qt::ControlModifier:
                        text = "Ctrl";
                        break;
                    case Qt::AltModifier:
                        text = "Alt";
                        break;
                    case Qt::MetaModifier:
                        text = "Meta";
                        break;
                }
                if (!text.isEmpty())
                {
                    if (!shortcutStr.isEmpty())
                        shortcutStr += "+";
                    shortcutStr += text;
                }
            };

            const auto modifiers = ke->modifiers();
            appendModifierStr(modifiers & Qt::MetaModifier);
            appendModifierStr(modifiers & Qt::ControlModifier);
            appendModifierStr(modifiers & Qt::AltModifier);
            appendModifierStr(modifiers & Qt::ShiftModifier);

            if (!shortcutStr.isEmpty())
            {
                auto key = ke->key();
                if (modifiers & Qt::ShiftModifier)
                {
                    switch (ke->nativeVirtualKey())
                    {
                        case '~':
                            key = '`';
                            break;
                        case '!':
                            key = '1';
                            break;
                        case '@':
                            key = '2';
                            break;
                        case '#':
                            key = '3';
                            break;
                        case '$':
                            key = '4';
                            break;
                        case '%':
                            key = '5';
                            break;
                        case '^':
                            key = '6';
                            break;
                        case '&':
                            key = '7';
                            break;
                        case '*':
                            key = '8';
                            break;
                        case '(':
                            key = '9';
                            break;
                        case ')':
                            key = '0';
                            break;
                        case '_':
                            key = '-';
                            break;
                        case '+':
                            key = '=';
                            break;
                        case '{':
                            key = '[';
                            break;
                        case '}':
                            key = ']';
                            break;
                        case '|':
                            key = '\\';
                            break;
                        case ':':
                            key = ';';
                            break;
                        case '"':
                            key = '\'';
                            break;
                        case '<':
                            key = ',';
                            break;
                        case '>':
                            key = '.';
                            break;
                        case '?':
                            key = '/';
                            break;
                    }
                }
                shortcutStr += "+" + QKeySequence(key).toString();
            }

            if (setKeySequence({shortcutStr, ke->nativeModifiers(), ke->nativeScanCode()}))
                m_record->setChecked(false);
        }
    }
    return QDialog::event(e);
}
