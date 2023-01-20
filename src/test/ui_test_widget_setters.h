
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
    template<>
    inline bool setUIElement(QPushButton* widget, const QString& value, int delay)
    {
        //just push the button
        return ui_test::injectClickEvent(widget, "", -1, -1, Qt::LeftButton, delay);
    }
    template<>
    inline bool setUIElement(QRadioButton* widget, const QString& value, int delay)
    {
        return ui_test::injectClickEvent(widget, "", 2, 2, Qt::LeftButton, delay);
    }
    template<>
    inline bool setUIElement(QCheckBox* widget, const QString& value, int delay)
    {
        auto v = conversions::valueFromString<bool>(value);
        if (!v)
            return false;

        return ui_test::injectCheckBoxEvent(widget, "", v.value(), delay);
    }
} // namespace ui_test
