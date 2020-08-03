#include "managesectorstask.h"
#include "atsdb.h"
#include "dbinterface.h"
#include "managesectorstaskwidget.h"
#include "taskmanager.h"
#include "savedfile.h"
#include "files.h"

#include "ogrsf_frmts.h"

#include "json.hpp"

using namespace Utils;
using namespace nlohmann;
using namespace std;

ManageSectorsTask::ManageSectorsTask(const std::string& class_id, const std::string& instance_id,
                                     TaskManager& task_manager)
    : Task("ManageSectorsTask", "Manage Sectors", true, true, task_manager),
      Configurable(class_id, instance_id, &task_manager, "task_manage_sectors.json")
{
    registerParameter("current_filename", &current_filename_, "");

    createSubConfigurables();

    tooltip_ =
            "Allows management of sectors stored in the database. "
            "This task can not be run, but is performed using the GUI elements.";

    if (current_filename_.size())
        parseCurrentFile();
}

TaskWidget* ManageSectorsTask::widget()
{
    if (!widget_)
    {
        widget_.reset(new ManageSectorsTaskWidget(*this));

        connect(&task_manager_, &TaskManager::expertModeChangedSignal, widget_.get(),
                &ManageSectorsTaskWidget::expertModeChangedSlot);
    }

    assert(widget_);
    return widget_.get();
}

void ManageSectorsTask::deleteWidget() { widget_.reset(nullptr); }

void ManageSectorsTask::generateSubConfigurable(const std::string& class_id,
                                                const std::string& instance_id)
{
    if (class_id == "SectorsFile")
    {
        SavedFile* file = new SavedFile(class_id, instance_id, this);
        assert(file_list_.count(file->name()) == 0);
        file_list_.insert(std::pair<std::string, SavedFile*>(file->name(), file));
    }
    else
        throw std::runtime_error("ManageSectorsTask: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

bool ManageSectorsTask::checkPrerequisites() { return ATSDB::instance().interface().ready(); }

void ManageSectorsTask::addFile(const std::string& filename)
{
    loginf << "ManageSectorsTask: addFile: filename '" << filename << "'";

    if (file_list_.count(filename) != 0)
        throw std::invalid_argument("ManageSectorsTask: addFile: name '" + filename +
                                    "' already in use");

    std::string instancename = filename;
    instancename.erase(std::remove(instancename.begin(), instancename.end(), '/'),
                       instancename.end());

    Configuration& config = addNewSubConfiguration("SectorsFile", "SectorsFile" + instancename);
    config.addParameterString("name", filename);
    generateSubConfigurable("SectorsFile", "SectorsFile" + instancename);

    current_filename_ = filename;
    parseCurrentFile();

    emit statusChangedSignal(name_);

    if (widget_)
    {
        widget_->updateFileListSlot();
        //widget_->updateText();
    }
}

void ManageSectorsTask::removeCurrentFilename()
{
    loginf << "ManageSectorsTask: removeCurrentFilename: filename '" << current_filename_ << "'";

    assert(current_filename_.size());
    assert(hasFile(current_filename_));

    if (file_list_.count(current_filename_) != 1)
        throw std::invalid_argument("ManageSectorsTask: removeCurrentFilename: name '" +
                                    current_filename_ + "' not in use");

    delete file_list_.at(current_filename_);
    file_list_.erase(current_filename_);
    current_filename_ = "";

    emit statusChangedSignal(name_);

    if (widget_)
        widget_->updateFileListSlot();
}

void ManageSectorsTask::removeAllFiles ()
{
    loginf << "ManageSectorsTask: removeAllFiles";

    while (file_list_.size())
    {
        delete file_list_.begin()->second;
        file_list_.erase(file_list_.begin());
    }

    current_filename_ = "";

    emit statusChangedSignal(name_);

    if (widget_)
        widget_->updateFileListSlot();
}

void ManageSectorsTask::currentFilename(const std::string& filename)
{
    loginf << "ManageSectorsTask: currentFilename: filename '" << filename << "'";

    bool had_filename = canImportFile();

    current_filename_ = filename;
    parseCurrentFile();

    if (!had_filename)  // not on re-select
        emit statusChangedSignal(name_);
}

bool ManageSectorsTask::canImportFile()
{
    if (!current_filename_.size())
        return false;

    if (!Files::fileExists(current_filename_))
    {
        loginf << "ManageSectorsTask: canImportFile: not possible since file '"
               << current_filename_ << "'does not exist";
        return false;
    }

    return true;
}

void ManageSectorsTask::parseCurrentFile ()
{
    loginf << "ManageSectorsTask: parseCurrentFile: file '" << current_filename_ << "'";

    GDALAllRegister();

    GDALDataset* data_set;

    data_set = (GDALDataset*) GDALOpenEx(current_filename_.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL );

    if(data_set == NULL)
    {
        logwrn << "ManageSectorsTask: parseCurrentFile: open failed";
        return;
    }

    data_.clear();

    for (auto* layer_it : data_set->GetLayers()) // OGRLayer*
    {
        loginf << "ManageSectorsTask: parseCurrentFile: found layer '" << layer_it->GetName() << "'";

        std::string layer_name = layer_it->GetName();

        for (auto& feature_t : layer_it) // OGRFeature
        {
            loginf << "ManageSectorsTask: parseCurrentFile: found feature '"
                   << feature_t->GetDefnRef()->GetName() << "'";

            std::string feature_name = feature_t->GetDefnRef()->GetName();

            for( auto&& field_t: *feature_t )
            {
                switch( field_t.GetType() )
                {
                    case OFTInteger:
                        loginf << "ManageSectorsTask: parseCurrentFile: int " << field_t.GetInteger();
                        break;
                    case OFTInteger64:
                        loginf << "ManageSectorsTask: parseCurrentFile: int64 " << field_t.GetInteger64();
                        //printf( CPL_FRMT_GIB ",", oField.GetInteger64() );
                        break;
                    case OFTReal:
                        loginf << "ManageSectorsTask: parseCurrentFile: double " << field_t.GetDouble();
                        break;
                    case OFTString:
                        loginf << "ManageSectorsTask: parseCurrentFile: string '" << field_t.GetString() << "'";
                        break;
                    default:
                        loginf << "ManageSectorsTask: parseCurrentFile: default " << field_t.GetString();
                        break;
                }
            }

            OGRGeometry* geometry;

            geometry = feature_t->GetGeometryRef();

            if (geometry != NULL)
            {
                if (wkbFlatten(geometry->getGeometryType()) == wkbPolygon )
                {
                    OGRPolygon* polygon = geometry->toPolygon();
                    assert (polygon);

                    for (auto* ring_it : *polygon) // OGRLinearRing
                    {
                        loginf << "ManageSectorsTask: parseCurrentFile: linear ring '"
                               << ring_it->getGeometryName() << "'";

                        vector<pair<double,double>> points;
                        for (auto& point_it : *ring_it)
                        {
                            loginf << "ManageSectorsTask: parseCurrentFile: point x " << point_it.getX()
                                   << " y " << point_it.getY() << " z " << point_it.getZ();

                            points.push_back({point_it.getY(), point_it.getX()});
                        }

                        if (points.size())
                            addPolygon (layer_name, feature_name, move(points));
                    }
                }
                else if (wkbFlatten(geometry->getGeometryType()) == wkbMultiPolygon)
                {
                    OGRMultiPolygon* multi_polygon = geometry->toMultiPolygon();
                    assert (multi_polygon);

                    for (auto* poly_it : *multi_polygon) // OGRMultiPolygon
                    {
                        for (auto* ring_it : *poly_it) // OGRLinearRing
                        {
                            loginf << "ManageSectorsTask: parseCurrentFile: linear ring '"
                                   << ring_it->getGeometryName() << "'";

                            vector<pair<double,double>> points;

                            for (auto& point_it : *ring_it)
                            {
                                loginf << "ManageSectorsTask: parseCurrentFile: point x " << point_it.getX()
                                       << " y " << point_it.getY() << " z " << point_it.getZ();
                                points.push_back({point_it.getY(), point_it.getX()});
                            }

                            if (points.size())
                                addPolygon (layer_name, feature_name, move(points));
                        }
                    }
                }
                else
                    loginf << "ManageSectorsTask: parseCurrentFile: polygons found";
            }
            else
                loginf << "ManageSectorsTask: parseCurrentFile: no geometry found";
        }
    }
}

void ManageSectorsTask::addPolygon (const std::string& layer_name, const std::string& polyon_name,
                                    std::vector<std::pair<double,double>> points)
{
    loginf << "ManageSectorsTask: addPolygon: layer '" << layer_name << "' poly '" << polyon_name
           << "' num points " << points.size();

    bool exists_poly = find(polygon_names_.begin(), polygon_names_.end(), polyon_name) != polygon_names_.end();

    string tmp_poly_name = polyon_name;

    if (exists_poly)
    {
        unsigned int cnt=2;

        while (find(polygon_names_.begin(), polygon_names_.end(), tmp_poly_name+to_string(cnt)) != polygon_names_.end())
            ++cnt;

        tmp_poly_name = tmp_poly_name+to_string(cnt);
    }

    loginf << "ManageSectorsTask: addPolygon: using polygon name '" << tmp_poly_name << "'";

    polygon_names_.push_back(tmp_poly_name);
    data_[layer_name][tmp_poly_name] = points;
}
