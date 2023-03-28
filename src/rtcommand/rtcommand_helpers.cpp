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

#include "rtcommand_helpers.h"
#include "rtcommand.h"

#include "compass.h"
#include "util/stringconv.h"
#include "test/ui_test_find.h"
#include "viewmanager.h"
#include "viewcontainerwidget.h"

#include <QDialog>
#include <QMainWindow>
#include <QApplication>

namespace rtcommand
{

/**
 * Returns the elements of a valid object path if the string could be split.
 */
boost::optional<std::vector<std::string>> validObjectPath(const std::string& path)
{
    std::vector<std::string> parts = Utils::String::split(path, RTCommand::ObjectPathSeparator);
    if (parts.empty())
        return {};

    for (const auto& p : parts)
        if (p.empty())
            return {};

    return parts;
}

/**
 * Sets the signal object and name from the given signal path.
 */
boost::optional<std::pair<std::string, std::string>> signalFromObjectPath(const std::string& path)
{
    auto path_parts = validObjectPath(path);
    if (!path_parts.has_value() || path_parts.value().size() < 2)
        return {};

    //last path element should be signal name
    std::string sig_name = path_parts.value().back();

    //the others describe the QObject the signal is emitted from
    std::vector<std::string> obj_path = path_parts.value();
    obj_path.pop_back();

    std::string sig_obj = Utils::String::compress(obj_path, RTCommand::ObjectPathSeparator);

    if (sig_name.empty() || sig_obj.empty())
        return {};

    return std::make_pair(sig_obj, sig_name);
}


/**
 * Return the application's main window.
 */
QMainWindow *mainWindow()
{
    for (auto win : qApp->topLevelWidgets())
    {
        auto mw = dynamic_cast<QMainWindow *>(win);
        if (mw)
            return mw;
    }
    return nullptr;
}

/**
 * Returns the currently active modal dialog.
 */
QDialog *activeDialog()
{
    QWidget *w = qApp->activeModalWidget();
    if (!w)
        return nullptr;

    QDialog *dlg = dynamic_cast<QDialog *>(w);
    if (!dlg)
        return nullptr;

    return dlg;
}

/**
 * Finds the QObject described by the given object path.
 * Unifies Configurables and UI elements (e.g. QWidget's).
 */
std::pair<rtcommand::FindObjectErrCode, QObject *> getCommandReceiver(const std::string &object_path)
{
    auto parts = validObjectPath(object_path);

    if (!parts.has_value())
        return {rtcommand::FindObjectErrCode::NotFound, nullptr};

    std::string first_part = parts.value().at(0);
    parts.value().erase(parts.value().begin());
    std::string remainder = Utils::String::compress(parts.value(), RTCommand::ObjectPathSeparator);

    if (first_part == "mainwindow")
    {
        return ui_test::findObject(mainWindow(), remainder.c_str());
    }
    else if (first_part.find("window") == 0)
    {
        //HACK, ViewContainerWidget could be a non-modal QDialog instead of a free-floating QWidget
        QString num = QString::fromStdString(first_part).remove("window");

        bool ok;
        int idx = num.toInt(&ok);

        if (ok)
        {
            QString view_container_name = "ViewWindow" + num;

            auto container = COMPASS::instance().viewManager().containerWidget(view_container_name.toStdString());
            if (!container)
                return std::make_pair(FindObjectErrCode::NotFound, nullptr);

            return ui_test::findObject(container, remainder.c_str());
        }
    }
    else if (first_part == "dialog")
    {
        return ui_test::findObject(activeDialog(), remainder.c_str());
    }   
    else if (first_part == "compass")
    {
        std::pair<rtcommand::FindObjectErrCode, Configurable*> ret = COMPASS::instance().findSubConfigurable(remainder);

        QObject *obj_casted = dynamic_cast<QObject *>(ret.second);

        if (!obj_casted)
            return {rtcommand::FindObjectErrCode::WrongType, nullptr};

        return {rtcommand::FindObjectErrCode::NoError, obj_casted};
    }

    return {rtcommand::FindObjectErrCode::NotFound, nullptr};
}

/**
 * Splits the given string parameter into a list of substrings.
 */
std::vector<std::string> parameterToStrings(const std::string& string_param)
{
    return Utils::String::split(string_param, RTCommand::ParameterListSeparator);
}

/**
 * Creates a string parameter from the given list of substrings.
 */
std::string parameterFromStrings(const std::vector<std::string>& strings)
{
    return Utils::String::compress(strings, RTCommand::ParameterListSeparator);
}

/**
 * Splits the given string parameter into a list of substrings.
 */
QStringList parameterToStrings(const QString& string_param)
{
    return string_param.split(RTCommand::ParameterListSeparator);
}

/**
 * Creates a string parameter from the given list of substrings.
 */
QString parameterFromStrings(const QStringList& strings)
{
    return strings.join(RTCommand::ParameterListSeparator);
}

} // rtcommand
