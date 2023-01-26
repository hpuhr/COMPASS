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

#include "logger.h"
#include "compass.h"
#include "viewmanager.h"
#include "viewcontainerwidget.h"

#include <QWidget>
#include <QWindow>
#include <QString>

namespace ui_test
{

enum class FindObjectErrCode
{
    NoError = 0,
    Invalid,
    NotFound,
    WrongType
};

inline std::string objectName(const QString& obj_name)
{
    if (obj_name.isEmpty())
        return "";
    return "'" + obj_name.toStdString() + "'";
}

inline void logObjectError(const QString& prefix, 
                           const QString& obj_name,
                           FindObjectErrCode code)
{
    if (code == FindObjectErrCode::NoError)
        return;

    QString err = "yielded unknown error";
    if (code == FindObjectErrCode::Invalid)
        err = "is invalid";
    else if (code == FindObjectErrCode::NotFound)
        err = "not found";
    else if (code == FindObjectErrCode::WrongType)
        err = "has wrong type";
    
    loginf << prefix.toStdString() << ": Object " << objectName(obj_name) + " " + err.toStdString();
}

inline std::pair<FindObjectErrCode, QObject*> findObjectNoPath(QObject* parent, const QString& obj_name = "")
{
    if (!parent)
        return std::make_pair(FindObjectErrCode::Invalid, nullptr);

    if (obj_name.isEmpty() || parent->objectName() == obj_name)
        return std::make_pair(FindObjectErrCode::NoError, parent);

    //UGLY HACK, ViewContainerWidget should be a non-modal QDialog instead of a free-floating QWidget
    {
        if (obj_name.startsWith("window"))
        {
            QString num = QString(obj_name).remove("window");
            bool ok;
            int idx = num.toInt(&ok);

            if (ok)
            {
                QString view_container_name = "ViewWindow" + num;

                auto container = COMPASS::instance().viewManager().containerWidget(view_container_name.toStdString());
                if (!container)
                    return std::make_pair(FindObjectErrCode::NotFound, nullptr);

                return std::make_pair(FindObjectErrCode::NoError, container);
            }
        }
    }

    QObject* obj = parent->findChild<QObject*>(obj_name, Qt::FindChildrenRecursively);
    if (!obj)
        return std::make_pair(FindObjectErrCode::NotFound, nullptr);

    return std::make_pair(FindObjectErrCode::NoError, obj);
}

inline std::pair<FindObjectErrCode, QObject*> findObject(QObject* parent, const QString& obj_name = "")
{
    if (!parent)
        return std::make_pair(FindObjectErrCode::Invalid, nullptr);

    if (obj_name.isEmpty() || parent->objectName() == obj_name)
        return std::make_pair(FindObjectErrCode::NoError, parent);

    int idx = obj_name.indexOf(".");

    //no path? use that version
    if (idx < 0)
        return findObjectNoPath(parent, obj_name);

    //split into path
    QStringList path = obj_name.split(".");

    //traverse the given object path and jump from child to child
    QObject* last_obj = parent;
    for (const QString& sub_obj : path)
    {
        std::cout << "looking for object '" << sub_obj.toStdString() << "'" << std::endl; 

        auto obj = findObjectNoPath(last_obj, sub_obj.trimmed());
        if (obj.first != FindObjectErrCode::NoError)
            return std::make_pair(obj.first, nullptr);

        std::cout << "   FOUND" << std::endl;

        last_obj = obj.second;
    }

    return std::make_pair(FindObjectErrCode::NoError, last_obj);
}

template<class T>
inline std::pair<FindObjectErrCode, T*> findObjectAs(QObject* parent, const QString& obj_name = "")
{
    auto obj = findObject(parent, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
        return std::make_pair(obj.first, nullptr);

    T* obj_cast = dynamic_cast<T*>(obj.second);
    if (!obj_cast)
        return std::make_pair(FindObjectErrCode::WrongType, nullptr);

    return std::make_pair(FindObjectErrCode::NoError, obj_cast);
}
template<>
inline std::pair<FindObjectErrCode, QObject*> findObjectAs(QObject* parent, const QString& obj_name)
{
    return findObject(parent, obj_name);
}
template<>
inline std::pair<FindObjectErrCode, QWidget*> findObjectAs(QObject* parent, const QString& obj_name)
{
    auto obj = findObject(parent, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
        return std::make_pair(obj.first, nullptr);

    if (!obj.second->isWidgetType())
        return std::make_pair(FindObjectErrCode::WrongType, nullptr);

    return std::make_pair(FindObjectErrCode::NoError, dynamic_cast<QWidget*>(obj.second));
}
template<>
inline std::pair<FindObjectErrCode, QWindow*> findObjectAs(QObject* parent, const QString& obj_name)
{
    auto obj = findObject(parent, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
        return std::make_pair(obj.first, nullptr);

    if (!obj.second->isWindowType())
        return std::make_pair(FindObjectErrCode::WrongType, nullptr);

    return std::make_pair(FindObjectErrCode::NoError, dynamic_cast<QWindow*>(obj.second));
}

} // namespace ui_test
