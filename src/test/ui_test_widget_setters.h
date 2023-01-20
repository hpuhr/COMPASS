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

#include <QMenu>
#include <QMenuBar>
#include <QComboBox>
#include <QTabWidget>
#include <QToolBar>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QPushButton>
#include <QToolButton>
#include <QCheckBox>
#include <QRadioButton>

#include <QWidget>
#include <QString>
#include <QStringList>

#include <boost/optional.hpp>

namespace ui_test
{
    /**
     * Value conversions from string values to specific value types.
     */
    namespace conversions
    {
        template<typename T>
        inline boost::optional<T> valueFromString(const QString& value)
        {
            return {};
        }
        template<>
        inline boost::optional<int> valueFromString(const QString& value)
        {
            if (value.isEmpty())
                return {};

            bool ok;
            int ret = value.toInt(&ok);

            if (!ok)
                return {};

            return ret;
        }
        template<>
        inline boost::optional<double> valueFromString(const QString& value)
        {
            if (value.isEmpty())
                return {};

            bool ok;
            double ret = value.toDouble(&ok);

            if (!ok)
                return {};

            return ret;
        }
        template<>
        inline boost::optional<QStringList> valueFromString(const QString& value)
        {
            if (value.isEmpty())
                return {};
            
            QStringList strings = value.split("|");
            if (strings.empty())
                return {};

            return strings;
        }
        template<>
        inline boost::optional<bool> valueFromString(const QString& value)
        {
            if (value.isEmpty())
                return {};

            if (value == "true")
                return true;
            else if (value == "false")
                return false;

            //unknown value
            return {};
        }
    }

    /**
     * Invoke widget specific event injectors here and
     * feed them with values they understand.
     */
    template <class T>
    inline bool setUIElement(T* widget, const QString& value, int delay = -1)
    {
        return false;
    }
    template<>
    inline bool setUIElement(QMenuBar* widget, const QString& value, int delay)
    {
        auto v = conversions::valueFromString<QStringList>(value);
        if (!v)
            return false;

        return injectMenuBarEvent(widget, "", v.value(), delay);
    }
    template<>
    inline bool setUIElement(QComboBox* widget, const QString& value, int delay)
    {
        return injectComboBoxEditEvent(widget, "", value, delay);
    }
    template<>
    inline bool setUIElement(QTabWidget* widget, const QString& value, int delay)
    {
        return injectTabSelectionEvent(widget, "", value, delay);
    }
    template<>
    inline bool setUIElement(QToolBar* widget, const QString& value, int delay)
    {
        return injectToolSelectionEvent(widget, "", value, delay);
    }
    template<>
    inline bool setUIElement(QLineEdit* widget, const QString& value, int delay)
    {
        return injectLineEditEvent(widget, "", value, delay);
    }
    template<>
    inline bool setUIElement(QSpinBox* widget, const QString& value, int delay)
    {
        auto v = conversions::valueFromString<int>(value);
        if (!v)
            return false;

        return ui_test::injectSpinBoxEvent(widget, "", v.value(), delay);
    }
    template<>
    inline bool setUIElement(QDoubleSpinBox* widget, const QString& value, int delay)
    {
        auto v = conversions::valueFromString<double>(value);
        if (!v)
            return false;

        return ui_test::injectDoubleSpinBoxEvent(widget, "", v.value(), delay);
    }
    template<>
    inline bool setUIElement(QSlider* widget, const QString& value, int delay)
    {
        auto v = conversions::valueFromString<double>(value);
        if (!v)
            return false;

        return ui_test::injectSliderEditEvent(widget, "", v.value(), delay);
    }

    namespace
    {
        bool buttonEvent(QAbstractButton* widget, const QString& value, bool center, int delay)
        {
            const int x = center ? -1 : widget->height() / 2;
            const int y = center ? -1 : widget->height() / 2;

            return ui_test::injectClickEvent(widget, "", x, y, Qt::LeftButton, delay);
        }
        bool menuButtonEvent(QAbstractButton* widget, const QString& value, int delay)
        {
            auto v = conversions::valueFromString<QStringList>(value);
            if (!v)
                return false;

            return ui_test::injectButtonMenuEvent(widget, "", v.value(), delay);
        }
        bool dualButtonEvent(QAbstractButton* widget, const QString& value, int delay)
        {
            if (value.isEmpty())
            {
                //just push the button
                return buttonEvent(widget, "", true, delay);
            }
            //interpret as menu triggering button
            return menuButtonEvent(widget, value, delay);
        }
    }

    template<>
    inline bool setUIElement(QPushButton* widget, const QString& value, int delay)
    {
        return dualButtonEvent(widget, value, delay);
    }
    template<>
    inline bool setUIElement(QToolButton* widget, const QString& value, int delay)
    {
        return dualButtonEvent(widget, value, delay);
    }
    template<>
    inline bool setUIElement(QRadioButton* widget, const QString& value, int delay)
    {
        return buttonEvent(widget, value, false, delay);
    }
    template<>
    inline bool setUIElement(QCheckBox* widget, const QString& value, int delay)
    {
        auto v = conversions::valueFromString<bool>(value);
        if (!v)
            return false;

        return ui_test::injectCheckBoxEvent(widget, "", v.value(), delay);
    }
    template<>
    inline bool setUIElement(QAbstractButton* widget, const QString& value, int delay)
    {
        //default button fallback
        return buttonEvent(widget, value, true, delay);
    }
} // namespace ui_test
