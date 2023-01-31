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

#include "ui_test_event_injections.h"
#include "ui_test_common.h"
#include "ui_test_conversions.h"

#include <QMenu>
#include <QMenuBar>
#include <QComboBox>
#include <QTabWidget>
#include <QToolBar>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QPushButton>
#include <QToolButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QLabel>

#include <QWidget>
#include <QString>
#include <QStringList>

#include <boost/optional.hpp>

namespace ui_test
{
    
    /**
     * Invoke widget specific event injectors here and
     * feed them with values they understand.
     * Passed hints are used/interpreted when suitable.
     */
    template <class T>
    inline bool setUIElement(T* widget, const QString& value, int delay = -1, const SetUIHint& hint = SetUIHint())
    {
        return false;
    }
    template<>
    inline bool setUIElement(QLabel* widget, const QString& value, int delay, const SetUIHint& hint)
    {
        //just for completeness
        widget->setText(value);
        return true;
    }
    template<>
    inline bool setUIElement(QMenuBar* widget, const QString& value, int delay, const SetUIHint& hint)
    {
        auto v = conversions::valueFromString<QStringList>(value);
        if (!v)
            return false;

        return injectMenuBarEvent(widget, "", v.value(), delay);
    }
    template<>
    inline bool setUIElement(QComboBox* widget, const QString& value, int delay, const SetUIHint& hint)
    {
        return injectComboBoxEditEvent(widget, "", value, delay);
    }
    template<>
    inline bool setUIElement(QTabWidget* widget, const QString& value, int delay, const SetUIHint& hint)
    {
        return injectTabSelectionEvent(widget, "", value, delay);
    }
    template<>
    inline bool setUIElement(QToolBar* widget, const QString& value, int delay, const SetUIHint& hint)
    {
        return injectToolSelectionEvent(widget, "", value, delay);
    }
    template<>
    inline bool setUIElement(QLineEdit* widget, const QString& value, int delay, const SetUIHint& hint)
    {
        return injectLineEditEvent(widget, "", value, delay);
    }
    template<>
    inline bool setUIElement(QTextEdit* widget, const QString& value, int delay, const SetUIHint& hint)
    {
        return injectTextEditEvent(widget, "", value, delay);
    }
    template<>
    inline bool setUIElement(QSpinBox* widget, const QString& value, int delay, const SetUIHint& hint)
    {
        auto v = conversions::valueFromString<int>(value);
        if (!v)
            return false;

        return ui_test::injectSpinBoxEvent(widget, "", v.value(), delay);
    }
    template<>
    inline bool setUIElement(QDoubleSpinBox* widget, const QString& value, int delay, const SetUIHint& hint)
    {
        auto v = conversions::valueFromString<double>(value);
        if (!v)
            return false;

        return ui_test::injectDoubleSpinBoxEvent(widget, "", v.value(), delay);
    }
    template<>
    inline bool setUIElement(QSlider* widget, const QString& value, int delay, const SetUIHint& hint)
    {
        auto v = conversions::valueFromString<double>(value);
        if (!v)
            return false;

        return ui_test::injectSliderEditEvent(widget, "", v.value(), delay);
    }

    namespace
    {
        bool buttonEvent(QAbstractButton* widget, const QString& value, bool center, int delay, const SetUIHint& hint)
        {
            //hint always comes first
            const int x = (hint.x >= 0 ? hint.x : (center ? -1 : widget->height() / 2));
            const int y = (hint.y >= 0 ? hint.y : (center ? -1 : widget->height() / 2));

            return ui_test::injectClickEvent(widget, "", x, y, Qt::LeftButton, delay);
        }
        bool menuButtonEvent(QAbstractButton* widget, const QString& value, int delay, const SetUIHint& hint)
        {
            auto v = conversions::valueFromString<QStringList>(value);
            if (!v)
                return false;

            //hint always comes first
            const int x = (hint.x >= 0 ? hint.x : -1);
            const int y = (hint.y >= 0 ? hint.y : -1);

            return ui_test::injectButtonMenuEvent(widget, "", v.value(), x, y, delay);
        }
        bool dualButtonEvent(QAbstractButton* widget, const QString& value, int delay, const SetUIHint& hint)
        {
            if (value.isEmpty())
            {
                //just push the button
                return buttonEvent(widget, "", true, delay, hint);
            }
            //interpret as menu triggering button
            return menuButtonEvent(widget, value, delay, hint);
        }
    }

    template<>
    inline bool setUIElement(QPushButton* widget, const QString& value, int delay, const SetUIHint& hint)
    {
        return dualButtonEvent(widget, value, delay, hint);
    }
    template<>
    inline bool setUIElement(QToolButton* widget, const QString& value, int delay, const SetUIHint& hint)
    {
        return dualButtonEvent(widget, value, delay, hint);
    }
    template<>
    inline bool setUIElement(QRadioButton* widget, const QString& value, int delay, const SetUIHint& hint)
    {
        return buttonEvent(widget, value, false, delay, hint);
    }
    template<>
    inline bool setUIElement(QCheckBox* widget, const QString& value, int delay, const SetUIHint& hint)
    {
        auto v = conversions::valueFromString<bool>(value);
        if (!v)
            return false;

        //@TODO: respect hint
        return ui_test::injectCheckBoxEvent(widget, "", v.value(), delay);
    }
    template<>
    inline bool setUIElement(QAbstractButton* widget, const QString& value, int delay, const SetUIHint& hint)
    {
        //default button fallback
        return buttonEvent(widget, value, true, delay, hint);
    }
} // namespace ui_test
