#include "viewpointstablemodel.h"
#include "viewmanager.h"
#include "viewpoint.h"
#include "json.hpp"
#include "json.h"
#include "stringconv.h"
#include "files.h"

using namespace nlohmann;
using namespace Utils;

ViewPointsTableModel::ViewPointsTableModel(ViewManager& view_manager)
    : view_manager_(view_manager), view_points_(view_manager_.viewPoints())
{
    updateTableColumns();

    open_icon_ = QIcon(Files::getIconFilepath("not_recommended.png").c_str());
    closed_icon_ = QIcon(Files::getIconFilepath("not_todo.png").c_str());
    todo_icon_ = QIcon(Files::getIconFilepath("todo.png").c_str());
    unknown_icon_ = QIcon(Files::getIconFilepath("todo_maybe.png").c_str());
}

//ViewPointsTableModel::~ViewPointsTableModel()
//{

//}

int ViewPointsTableModel::rowCount(const QModelIndex& parent) const
{
    return view_points_.size();
}

int ViewPointsTableModel::columnCount(const QModelIndex& parent) const
{
    return table_columns_.size();
}

QVariant ViewPointsTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role)
    {
        case Qt::DisplayRole:
        {
            logdbg << "ViewPointsTableModel: data: display role: row " << index.row() << " col " << index.column();

            auto map_it = view_points_.begin();
            std::advance(map_it, index.row());

            if (map_it == view_points_.end())
                return QVariant();

            logdbg << "ViewPointsTableModel: data: got key " << map_it->first;

            assert (index.column() < table_columns_.size());
            std::string col_name = table_columns_.at(index.column()).toStdString();

            if (!map_it->second.data().contains(col_name))
                return QVariant();

            json& data = map_it->second.data().at(col_name);

//            if (col_name == "status" && (data == "open" || data == "closed" || data == "todo"))
//                return QVariant();

            // s1.find(s2) != std::string::npos
            if (data.is_number() && col_name.find("time") != std::string::npos)
                return String::timeStringFromDouble(data).c_str();

            if (data.is_boolean())
                return data.get<bool>();

            if (data.is_number())
                return data.get<float>();

            return JSON::toString(data).c_str();
        }
        case Qt::DecorationRole:
        {
            assert (index.column() < table_columns_.size());

            if (table_columns_.at(index.column()) == "status")
            {
                auto map_it = view_points_.begin();
                std::advance(map_it, index.row());

                if (map_it == view_points_.end())
                    return QVariant();

                json& data = map_it->second.data().at("status");
                assert (data.is_string());

                std::string status = data;

                if (status == "open")
                    return open_icon_;
                else if (status == "closed")
                    return closed_icon_;
                else if (status == "todo")
                    return todo_icon_;
                else
                    return unknown_icon_;
            }
            else
                return QVariant();
        }
        default:
        {
            return QVariant();
        }
    }
}

QVariant ViewPointsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        assert (section < table_columns_.size());
        return table_columns_.at(section);
    }

    return QVariant();
}

QModelIndex ViewPointsTableModel::index(int row, int column, const QModelIndex& parent) const
{
    return createIndex(row, column);
}

QModelIndex ViewPointsTableModel::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

Qt::ItemFlags ViewPointsTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    assert (index.column() < table_columns_.size());

    if (table_columns_.at(index.column()) == "comment")
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    else
        return QAbstractItemModel::flags(index);
}

bool ViewPointsTableModel::setData(const QModelIndex& index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        unsigned int id = getIdOf(index);

        loginf << "ViewPointsTableModel: setData: row " << index.row() << " col " << index.column()
               << " id " << id << " '" << value.toString().toStdString() << "'";

        assert (id < view_points_.size());

        assert (index.column() == statusColumn() || index.column() == commentColumn());

        if (index.column() == statusColumn())
            view_points_.at(id).data()["status"] = value.toString().toStdString();
        else
            view_points_.at(id).data()["comment"] = value.toString().toStdString();

        view_points_.at(id).dirty(true);

        emit dataChanged(index, index);

        return true;
    }
    return false;
}

void ViewPointsTableModel::updateTableColumns()
{
    for (auto& vp_it : view_points_)
    {
        nlohmann::json& data = vp_it.second.data();

        assert (data.is_object());
        for (auto& j_it : data.get<json::object_t>())
        {
            logdbg << "ViewPointsTableModel: updateTableColumns: '" << j_it.first << "'";

            if (j_it.second.is_object() || j_it.second.is_array()) // skip complex items
                continue;

            if (!table_columns_.contains(j_it.first.c_str()))
                table_columns_.append(j_it.first.c_str());
        }
    }
}

void ViewPointsTableModel::update()
{
    loginf << "ViewPointsTableModel: update";
    beginResetModel();
    updateTableColumns();
    endResetModel();
}

unsigned int ViewPointsTableModel::getIdOf (const QModelIndex& index)
{
     assert (index.isValid());
     auto map_it = view_points_.begin();
     std::advance(map_it, index.row());
     assert (map_it != view_points_.end());

     return map_it->first;
}

void ViewPointsTableModel::setStatus (const QModelIndex& row_index, const std::string& value)
{
    assert (row_index.isValid());

    QModelIndex status_index = index(row_index.row(), statusColumn(), QModelIndex());
    assert (status_index.isValid());

    setData(status_index, value.c_str(), Qt::EditRole);

//    unsigned int id = getIdOf(index);
//    assert (id < view_points_.size());
//    loginf << "ViewPointsTableModel: setStatus: id " << id << " status " << value;
//    view_points_.at(id).data()["status"] = value;
//    view_points_.at(id).dirty(true);

//    emit dataChanged(index, index, {Qt::UserRole});
}

