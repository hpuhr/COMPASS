
#pragma once

#include "logger.h"

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

inline std::pair<FindObjectErrCode, QObject*> findObject(QWidget* parent, const QString& obj_name = "")
{
    if (!parent)
        return std::make_pair(FindObjectErrCode::Invalid, nullptr);

    if (obj_name.isEmpty() || parent->objectName() == obj_name)
        return std::make_pair(FindObjectErrCode::NoError, parent);

    QObject* obj = parent->findChild<QObject*>(obj_name, Qt::FindChildrenRecursively);
    if (!obj)
        return std::make_pair(FindObjectErrCode::NotFound, nullptr);

    return std::make_pair(FindObjectErrCode::NoError, obj);
}

template<class T>
inline std::pair<FindObjectErrCode, T*> findObjectAs(QWidget* parent, const QString& obj_name = "")
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
inline std::pair<FindObjectErrCode, QObject*> findObjectAs(QWidget* parent, const QString& obj_name)
{
    return findObject(parent, obj_name);
}
template<>
inline std::pair<FindObjectErrCode, QWidget*> findObjectAs(QWidget* parent, const QString& obj_name)
{
    auto obj = findObject(parent, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
        return std::make_pair(obj.first, nullptr);

    if (!obj.second->isWidgetType())
        return std::make_pair(FindObjectErrCode::WrongType, nullptr);

    return std::make_pair(FindObjectErrCode::NoError, dynamic_cast<QWidget*>(obj.second));
}
template<>
inline std::pair<FindObjectErrCode, QWindow*> findObjectAs(QWidget* parent, const QString& obj_name)
{
    auto obj = findObject(parent, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
        return std::make_pair(obj.first, nullptr);

    if (!obj.second->isWindowType())
        return std::make_pair(FindObjectErrCode::WrongType, nullptr);

    return std::make_pair(FindObjectErrCode::NoError, dynamic_cast<QWindow*>(obj.second));
}

} // namespace ui_test
