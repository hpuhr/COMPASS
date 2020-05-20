#include "viewpointstablemodel.h"
#include "viewmanager.h"
#include "viewpoint.h"
#include "json.hpp"
#include "json.h"
#include "stringconv.h"
#include "files.h"
#include "atsdb.h"
#include "dbinterface.h"
#include "filtermanager.h"
#include "viewpointswidget.h"

#include <fstream>

#include <QMessageBox>

using namespace nlohmann;
using namespace Utils;
using namespace std;

ViewPointsTableModel::ViewPointsTableModel(ViewManager& view_manager)
    : view_manager_(view_manager)
{
    table_columns_ = default_table_columns_;

    // load view points
    if (ATSDB::instance().interface().existsViewPointsTable())
    {
        for (const auto& vp_it : ATSDB::instance().interface().viewPoints())
        {
            assert (!view_points_.count(vp_it.first));
            view_points_.emplace(std::piecewise_construct,
                                 std::forward_as_tuple(vp_it.first),   // args for key
                                 std::forward_as_tuple(vp_it.first, vp_it.second, view_manager_, false));
            // args for mapped value
        }
    }

    updateTableColumns();
    updateTypes();

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
        case Qt::EditRole:
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

                const json& data = map_it->second.data().at(col_name);

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

                    const json& data = map_it->second.data().at("status");
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
            view_points_.at(id).setStatus(value.toString().toStdString());
        else
            view_points_.at(id).setComment(value.toString().toStdString());

        emit dataChanged(index, index);

        return true;
    }
    return false;
}

//int ViewPointsTableModel::columnIndex (QString name)
//{
//    assert (table_columns_.contains(name));
//    return table_columns_.indexOf(name);
//}

bool ViewPointsTableModel::updateTableColumns()
{
    loginf << "ViewPointsTableModel: updateTableColumns";

    bool changed = false;

    for (auto& vp_it : view_points_)
    {
        const nlohmann::json& data = vp_it.second.data();

        assert (data.is_object());
        for (auto& j_it : data.get<json::object_t>())
        {
            logdbg << "ViewPointsTableModel: updateTableColumns: '" << j_it.first << "'";

            if (j_it.second.is_object() || j_it.second.is_array()) // skip complex items
                continue;

            if (!table_columns_.contains(j_it.first.c_str()))
            {
                beginInsertColumns(QModelIndex(), table_columns_.size(), table_columns_.size());
                table_columns_.append(j_it.first.c_str());
                endInsertColumns();

                changed = true;
            }
        }
    }

    return changed;
}

void ViewPointsTableModel::updateTypes()
{
    loginf << "ViewPointsTableModel: updateTypes";

    QStringList old_types = types_;
    types_.clear();

    for (auto& vp_it : view_points_)
    {
        const nlohmann::json& data = vp_it.second.data();

        assert (data.contains("type"));

        const string& type = data.at("type");

        if (!types_.contains(type.c_str()))
            types_.append(type.c_str());
    }

    if (types_ != old_types)
    {
        loginf << "ViewPointsTableModel: updateTypes: changed";

        emit typesChangedSignal(types_);
    }
}

QStringList ViewPointsTableModel::types() const
{
    return types_;
}

QStringList ViewPointsTableModel::tableColumns() const
{
    return table_columns_;
}

QStringList ViewPointsTableModel::defaultTableColumns() const
{
    return default_table_columns_;
}

unsigned int ViewPointsTableModel::saveNewViewPoint(const nlohmann::json& data, bool update)
{
    unsigned int new_id {0};

    if (view_points_.size())
        new_id = view_points_.rbegin()->first + 1;

    assert (!existsViewPoint(new_id));

    nlohmann::json new_data = data;
    new_data["id"] = new_id;

    saveNewViewPoint(new_id, new_data, update);

    return new_id;
}

ViewPoint& ViewPointsTableModel::saveNewViewPoint(unsigned int id, const nlohmann::json& data, bool update)
{
    if (view_points_.count(id))
        throw std::runtime_error ("ViewPointsTableModel: addNewViewPoint: id "+std::to_string(id)+" already exists");

    unsigned int row = view_points_.size();

    if (update)
        beginInsertRows(QModelIndex(), row, row);

    nlohmann::json new_data = data;
    ATSDB::instance().filterManager().setConfigInViewPoint(new_data);

    view_points_.emplace(std::piecewise_construct,
                         std::forward_as_tuple(id),   // args for key
                         std::forward_as_tuple(id, new_data, view_manager_, true));  // args for mapped value

    assert (existsViewPoint(id));

    if (update)
    {
        emit endInsertRows();

        if (updateTableColumns()) // true if changed
            view_manager_.viewPointsWidget()->resizeColumnsToContents();

        updateTypes();
    }

    return view_points_.at(id);
}

bool ViewPointsTableModel::existsViewPoint(unsigned int id)
{
    return view_points_.count(id) == 1;
}

ViewPoint& ViewPointsTableModel::viewPoint(unsigned int id)
{
    assert (existsViewPoint(id));
    return view_points_.at(id);
}

//void ViewPointsTableModel::removeViewPoint(unsigned int id)
//{
//    assert (existsViewPoint(id));

//    view_points_.erase(id);
//    ATSDB::instance().interface().deleteViewPoint(id);
//}

void ViewPointsTableModel::deleteAllViewPoints ()
{
    if (!view_points_.size())
        return;

    if (table_columns_.size() > default_table_columns_.size())
    {
        beginRemoveColumns(QModelIndex(), default_table_columns_.size(), table_columns_.size()-1);
        table_columns_ = default_table_columns_;
        endRemoveColumns();
    }

    view_manager_.unsetCurrentViewPoint();

    beginRemoveRows(QModelIndex(), 0, view_points_.size()-1); // TODO

    view_points_.clear();
    ATSDB::instance().interface().deleteAllViewPoints();

    endRemoveRows();
}

void ViewPointsTableModel::printViewPoints()
{
    for (auto& vp_it : view_points_)
        vp_it.second.print();
}

void ViewPointsTableModel::importViewPoints (const std::string& filename)
{
    loginf << "ViewPointsTableModel: importViewPoints: filename '" << filename << "'";

    try
    {
        if (!Files::fileExists(filename))
            throw std::runtime_error ("File '"+filename+"' not found.");

        std::ifstream ifs(filename);
        json j = json::parse(ifs);

        if (!j.contains("view_point_context"))
            throw std::runtime_error("File '"+filename+"' has no context information");

        json& context = j.at("view_point_context");

        if (!context.contains("version"))
            throw std::runtime_error("File '"+filename+"' context has no version");

        json& version = context.at("version");

        if (!version.is_string())
            throw std::runtime_error("File '"+filename+"' context version is not number");

        string version_str = version;

        if (version_str != "0.1")
            throw std::runtime_error("File '"+filename+"' context version "+version_str+" is not supported");

        loginf << "ViewPointsTableModel: importViewPoints: context '" << j.at("view_point_context").dump(4) << "'";

        if (!j.contains("view_points"))
            throw std::runtime_error ("File '"+filename+"' does not contain view points.");

        json& view_points = j.at("view_points");

        if (!view_points.is_array())
            throw std::runtime_error ("View points are not in an array.");

        unsigned int row_begin = view_points_.size();
        unsigned int row_end = row_begin+view_points.size()-1;

        beginInsertRows(QModelIndex(), row_begin, row_end);

        unsigned int id;
        for (auto& vp_it : view_points.get<json::array_t>())
        {
            if (!vp_it.contains("id"))
                throw std::runtime_error ("View point does not contain id");

            id = vp_it.at("id");

            if (!vp_it.contains("status"))
                vp_it["status"] = "open";

            saveNewViewPoint(id, vp_it, false);
        }

        endInsertRows();

        updateTableColumns();
        updateTypes();

        //        if (view_points_widget_) // TODO
        //            view_points_widget_->update();

        QMessageBox m_info(QMessageBox::Information, "View Points Import File",
                           "File import: '"+QString(filename.c_str())+"' done.\n"
                           +QString::number(view_points.size())+" View Points added.", QMessageBox::Ok);
        m_info.exec();
    }
    catch (std::exception& e)
    {
        QMessageBox m_warning(QMessageBox::Warning, "View Points Import File",
                              "File import error: '"+QString(e.what())+"'.", QMessageBox::Ok);
        m_warning.exec();
        return;
    }
}

void ViewPointsTableModel::exportViewPoints (const std::string& filename)
{
    loginf << "ViewPointsTableModel: exportViewPoints: filename '" << filename << "'";

    json data;

    data["view_point_context"] = json::object();
    json& context = data.at("view_point_context");
    context["version"] = "0.1";

    data["view_points"] = json::array();
    json& view_points = data.at("view_points");

    unsigned int cnt = 0;
    for (auto& vp_it : view_points_)
    {
        view_points[cnt] = vp_it.second.data();
        ++cnt;
    }

    std::ofstream file(filename);
    file << data.dump(4);

    QMessageBox m_info(QMessageBox::Information, "View Points Export File",
                       "File export: '"+QString(filename.c_str())+"' done.\n"
                       +QString::number(view_points.size())+" View Points saved.", QMessageBox::Ok);
    m_info.exec();
}

//void ViewPointsTableModel::update()
//{
//    loginf << "ViewPointsTableModel: update";
//    beginResetModel();
//    updateTableColumns();
//    endResetModel();
//}

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

