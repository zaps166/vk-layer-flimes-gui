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
        m_qKeySeq = {};
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

bool HotkeyDialog::setKeySequence(const QtKeySequence &qKeySeq)
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
    appendModifierStr(qKeySeq.mod() & Qt::MetaModifier);
    appendModifierStr(qKeySeq.mod() & Qt::ControlModifier);
    appendModifierStr(qKeySeq.mod() & Qt::AltModifier);
    appendModifierStr(qKeySeq.mod() & Qt::ShiftModifier);

    if (!shortcutStr.isEmpty()
            && ((qKeySeq.key() >= Qt::Key_A && qKeySeq.key() <= Qt::Key_Z)
                || (qKeySeq.key() >= Qt::Key_0 && qKeySeq.key() <= Qt::Key_9)
                || (qKeySeq.key() >= Qt::Key_F1 && qKeySeq.key() <= Qt::Key_F35)))
    {
        shortcutStr += "+";

        if (qKeySeq.key() >= Qt::Key_F1 && qKeySeq.key() <= Qt::Key_F35)
            shortcutStr += QString("F%1").arg(qKeySeq.key() - Qt::Key_F1 + 1);
        else
            shortcutStr += QString(qKeySeq.key());

        m_label->setText(shortcutStr);
        m_qKeySeq = qKeySeq;
        return true;
    }

    return false;
}
QtKeySequence HotkeyDialog::getKeySequence() const
{
    return m_qKeySeq;
}

bool HotkeyDialog::event(QEvent *e)
{
    if (m_record->isChecked() && e->type() == QEvent::KeyPress)
    {
        auto ke = reinterpret_cast<QKeyEvent *>(e);
        if (!ke->isAutoRepeat() && setKeySequence({QGuiApplication::keyboardModifiers(), static_cast<Qt::Key>(ke->key())}))
            m_record->setChecked(false);
    }
    return QDialog::event(e);
}
