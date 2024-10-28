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

#pragma once

#include <boost/optional.hpp>

#include <QString>
#include <QLineEdit>

class QWidget;

/**
 */
class TextInputDialog
{
public:
    static boost::optional<QString> getText(QWidget *parent, 
                                            const QString &title, 
                                            const QString &label,
                                            bool allow_empty,
                                            QLineEdit::EchoMode echo = QLineEdit::Normal,
                                            const QString& text = QString(), 
                                            Qt::WindowFlags flags = Qt::WindowFlags(),
                                            Qt::InputMethodHints input_method_hints = Qt::ImhNone);
};
