/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "textinputdialog.h"

#include <QDialog>
#include <QPushButton>

#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>

/**
*/
boost::optional<QString> TextInputDialog::getText(QWidget *parent, 
                                                  const QString &title, 
                                                  const QString &label,
                                                  bool allow_empty,
                                                  QLineEdit::EchoMode echo,
                                                  const QString& text, 
                                                  Qt::WindowFlags flags,
                                                  Qt::InputMethodHints input_method_hints)
{
    QDialog dlg(parent, flags);
    dlg.setWindowTitle(title);

    auto layout = new QVBoxLayout;
    dlg.setLayout(layout);

    auto flayout = new QFormLayout;
    layout->addLayout(flayout);

    auto txt_edit = new QLineEdit;
    txt_edit->setText(text);
    txt_edit->setInputMethodHints(input_method_hints);

    flayout->addRow(label, txt_edit);

    layout->addStretch(1);

    auto button_layout = new QHBoxLayout;
    layout->addLayout(button_layout);

    auto button_ok     = new QPushButton("Ok");
    auto button_cancel = new QPushButton("Cancel");

    QObject::connect(button_ok    , &QPushButton::pressed, &dlg, &QDialog::accept);
    QObject::connect(button_cancel, &QPushButton::pressed, &dlg, &QDialog::reject);

    button_layout->addStretch(1);
    button_layout->addWidget(button_ok);
    button_layout->addWidget(button_cancel);

    button_ok->setEnabled(allow_empty || !text.isEmpty());

    if (!allow_empty)
    {
        auto cb = [ button_ok ] (const QString& txt)
        {
            button_ok->setEnabled(!txt.isEmpty());
        };
        QObject::connect(txt_edit, &QLineEdit::textEdited, cb);
    }

    if (dlg.exec() == QDialog::Rejected)
        return {};

    return txt_edit->text();
}
