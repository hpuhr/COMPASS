#include "managesectorstask.h"
#include "atsdb.h"
#include "dbinterface.h"
#include "managesectorstaskwidget.h"
#include "taskmanager.h"
#include "savedfile.h"
#include "files.h"
#include "sector.h"

#include "ogrsf_frmts.h"

#include "json.hpp"

#include <QMessageBox>

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

    if (canImportFile())
        parseCurrentFile(false);
}

ManageSectorsTask::~ManageSectorsTask()
{
    for (auto it : file_list_)
        delete it.second;

    file_list_.clear();
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
    parseCurrentFile(false);

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

    parse_message_ = "";
    found_sectors_num_ = 0;

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

    parse_message_ = "";
    found_sectors_num_ = 0;

    emit statusChangedSignal(name_);

    if (widget_)
        widget_->updateFileListSlot();
}

void ManageSectorsTask::currentFilename(const std::string& filename)
{
    loginf << "ManageSectorsTask: currentFilename: filename '" << filename << "'";

    bool had_filename = canImportFile();

    current_filename_ = filename;

    if (canImportFile())
        parseCurrentFile(false);
    else
    {
        parse_message_ = "";
        found_sectors_num_ = 0;
    }

    if (!had_filename)  // not on re-select
        emit statusChangedSignal(name_);
}

std::string ManageSectorsTask::parseMessage() const
{
    return parse_message_;
}

//const map<string, map<string, vector<pair<double, double>>>>& ManageSectorsTask::parsedData() const
//{
//    return data_;
//}

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

void ManageSectorsTask::importFile ()
{
    assert (canImportFile());

    if (!found_sectors_num_)
    {
        loginf << "ManageSectorsTask: importFile: not possible since no data found";

        QMessageBox msgBox;
        msgBox.setText("Import not possible since no data was found.");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();

        return;
    }

    task_manager_.appendInfo("ManageSectorsTask: import of file '" + current_filename_ +
                             "' started");

    parseCurrentFile(true);

    task_manager_.appendSuccess("ManageSectorsTask: imported " + to_string(found_sectors_num_)
                                +" sectors");

    QMessageBox msgBox;
    msgBox.setText(QString("Import of ")+QString::number(found_sectors_num_)+" sectors done");
    msgBox.setIcon(QMessageBox::Information);

    if (show_done_summary_)
        msgBox.exec();

    loginf << "ManageSectorsTask: importFile: done";
}


void ManageSectorsTask::parseCurrentFile (bool import)
{
    loginf << "ManageSectorsTask: parseCurrentFile: file '" << current_filename_ << "' import " << import;

    found_sectors_num_ = 0;
    parse_message_ = "";

    GDALAllRegister();

    GDALDataset* data_set;

    data_set = (GDALDataset*) GDALOpenEx(current_filename_.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL );

    if(data_set == NULL)
    {
        logwrn << "ManageSectorsTask: parseCurrentFile: open failed";
        parse_message_ = "file '"+current_filename_+"' open failed.";
        return;
    }


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
                            loginf << "ManageSectorsTask: parseCurrentFile: point lat " << point_it.getY()
                                   << " lon " << point_it.getX() << " z " << point_it.getZ();

                            points.push_back({point_it.getY(), point_it.getX()});
                        }

                        if (points.size())
                        {
                            parse_message_ += "Found layer '"+layer_name+"' sector '"+feature_name
                                    +"' num points "+to_string(points.size());

                            ++found_sectors_num_;

                            if (import)
                                addSector (feature_name, layer_name, move(points));
                        }
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
                                loginf << "ManageSectorsTask: parseCurrentFile: point lat " << point_it.getY()
                                       << " lon " << point_it.getX() << " z " << point_it.getZ();

                                points.push_back({point_it.getY(), point_it.getX()});
                            }

                            if (points.size())
                            {
                                parse_message_ += "Found layer '"+layer_name+"' sector '"+feature_name
                                        +"' num points "+to_string(points.size());

                                ++found_sectors_num_;

                                if (import)
                                    addSector (feature_name, layer_name, move(points));
                            }
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

    if (widget_)
        widget_->updateParseMessage();
}

void ManageSectorsTask::addSector (const std::string& sector_name, const std::string& layer_name,
                                   std::vector<std::pair<double,double>> points)
{
    loginf << "ManageSectorsTask: addSector: layer '" << layer_name << "' name '" << sector_name
           << "' num points " << points.size();

    DBInterface& db_interface = ATSDB::instance().interface();
    assert (db_interface.ready());

    string new_sector_name = sector_name;

    if (db_interface.hasSector(new_sector_name, layer_name))
    {
        unsigned int cnt = 2;

        while(db_interface.hasSector(new_sector_name, new_sector_name))
        {
            new_sector_name = sector_name+to_string(cnt);
            ++cnt;
        }
    }

    db_interface.createNewSector(new_sector_name, layer_name, points);
}
