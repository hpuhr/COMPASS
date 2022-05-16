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

#include "radarplotpositioncalculatortask.h"
#include "compass.h"
#include "buffer.h"
#include "dbinterface.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/variableset.h"
#include "logger.h"
#include "projection.h"
#include "projectionmanager.h"
#include "propertylist.h"
#include "radarplotpositioncalculatortaskdialog.h"
#include "stringconv.h"
#include "taskmanager.h"
#include "viewmanager.h"
#include "datasourcemanager.h"

#include <QApplication>
#include <QCoreApplication>
#include <QMessageBox>

using namespace std;
using namespace Utils;
using namespace dbContent;

const std::string RadarPlotPositionCalculatorTask::DONE_PROPERTY_NAME =
        "radar_plot_positions_calculated";

RadarPlotPositionCalculatorTask::RadarPlotPositionCalculatorTask(const std::string& class_id,
                                                                 const std::string& instance_id,
                                                                 TaskManager& task_manager)
    : Task("RadarPlotPositionCalculatorTask", "Calculate Radar Plot Positions", true, false,
           task_manager),
      Configurable(class_id, instance_id, &task_manager, "task_calc_radar_pos.json")
{
    tooltip_ =
            "Allows calculation of Radar plot position information based on the defined data sources.";

    qRegisterMetaType<std::shared_ptr<Buffer>>("std::shared_ptr<Buffer>");
    // qRegisterMetaType<DBObject>("DBObject");

}

RadarPlotPositionCalculatorTask::~RadarPlotPositionCalculatorTask() {}

RadarPlotPositionCalculatorTaskDialog* RadarPlotPositionCalculatorTask::dialog()
{
    if (!dialog_)
    {
        dialog_.reset(new RadarPlotPositionCalculatorTaskDialog(*this));

        connect(dialog_.get(), &RadarPlotPositionCalculatorTaskDialog::closeSignal,
                this, &RadarPlotPositionCalculatorTask::dialogCloseSlot);
    }

    assert(dialog_);
    return dialog_.get();
}

bool RadarPlotPositionCalculatorTask::checkPrerequisites()
{
    if (!COMPASS::instance().interface().ready())
        return false;

    if (COMPASS::instance().interface().hasProperty(DONE_PROPERTY_NAME))
        done_ =
                COMPASS::instance().interface().getProperty(DONE_PROPERTY_NAME) == "1";  // set done flag

    if (!COMPASS::instance().dbContentManager().existsDBContent("Radar"))
        return false;

    return COMPASS::instance().dbContentManager().dbContent("Radar").hasData();
}

bool RadarPlotPositionCalculatorTask::isRecommended()
{
    if (!checkPrerequisites())
        return false;

    return !done_;
}

bool RadarPlotPositionCalculatorTask::isRequired() { return false; }

bool RadarPlotPositionCalculatorTask::canRun()
{
    if (!COMPASS::instance().dbContentManager().existsDBContent("CAT001")
            && !COMPASS::instance().dbContentManager().existsDBContent("CAT048"))
        return false;

    if (!COMPASS::instance().dbContentManager().dbContent("CAT001").loadable()
            && !COMPASS::instance().dbContentManager().dbContent("CAT048").loadable())
        return false;

    return true;
}

void RadarPlotPositionCalculatorTask::run()
{
    loginf << "RadarPlotPositionCalculatorTask: run: start";

    assert(canRun());

    task_manager_.appendInfo("RadarPlotPositionCalculatorTask: started");

    start_time_ = boost::posix_time::microsec_clock::local_time();

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    dbcontent_man.clearData();
    dbcontent_done_.clear();

    COMPASS::instance().viewManager().disableDataDistribution(true);

    connect(&dbcontent_man, &DBContentManager::loadedDataSignal,
            this, &RadarPlotPositionCalculatorTask::loadedDataDataSlot);
    connect(&dbcontent_man, &DBContentManager::loadingDoneSignal,
            this, &RadarPlotPositionCalculatorTask::loadingDoneSlot);

    calculating_ = true;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    std::string msg = "Loading object data";
    msg_box_ = new QMessageBox;
    assert(msg_box_);
    msg_box_->setWindowTitle("Calculating Radar Plot Positions");
    msg_box_->setText(msg.c_str());
    msg_box_->setStandardButtons(QMessageBox::NoButton);
    msg_box_->show();

    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    num_loaded_ = 0;

    for (auto& dbo_it : dbcontent_man)
    {
        if (dbo_it.first != "CAT001" && dbo_it.first != "CAT048")
            continue;

        if (!dbo_it.second->hasData())
            continue;

        VariableSet read_set = getReadSetFor(dbo_it.first);

        dbo_it.second->load(read_set, false, false, true,
                            &dbcontent_man.metaGetVariable(dbo_it.first, DBContent::meta_var_tod_), true);
    }
}

void RadarPlotPositionCalculatorTask::loadedDataDataSlot(
        const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset)
{
    data_ = data;
}

void RadarPlotPositionCalculatorTask::loadingDoneSlot()
{
    loginf << "RadarPlotPositionCalculatorTask: loadingDoneSlot: starting calculation";

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    disconnect(&dbcontent_man, &DBContentManager::loadedDataSignal,
               this, &RadarPlotPositionCalculatorTask::loadedDataDataSlot);
    disconnect(&dbcontent_man, &DBContentManager::loadingDoneSignal,
               this, &RadarPlotPositionCalculatorTask::loadingDoneSlot);

    dbcontent_man.clearData();

    COMPASS::instance().viewManager().disableDataDistribution(false);

    ProjectionManager& proj_man = ProjectionManager::instance();

    assert(proj_man.hasCurrentProjection());
    Projection& projection = proj_man.currentProjection();

    loginf << "RadarPlotPositionCalculatorTask: loadingDoneSlot: projection method '"
           << projection.name() << "'";

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    string dbcontent_name;
    set<unsigned int> ds_wo_full_pos;

    for (auto& buf_it : data_)
    {
        dbcontent_name = buf_it.first;
        assert (dbcontent_name == "CAT001" || dbcontent_name == "CAT048");

        assert (msg_box_);
        msg_box_->setText(("Processing "+dbcontent_name+" data").c_str());

        std::shared_ptr<Buffer> read_buffer = buf_it.second;
        unsigned int read_size = read_buffer->size();
        assert(read_size);

        loginf << "RadarPlotPositionCalculatorTask: loadingDoneSlot: calculating " << dbcontent_name
               << " read_size " << read_size;

        PropertyList update_buffer_list;

        update_buffer_list.addProperty(
                    dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_latitude_).name(),
                    PropertyDataType::DOUBLE);
        update_buffer_list.addProperty(
                    dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_longitude_).name(),
                    PropertyDataType::DOUBLE);
        update_buffer_list.addProperty(
                    dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_rec_num_).name(),
                    PropertyDataType::UINT); // must be at last position for update

        std::shared_ptr<Buffer> update_buffer =
                std::make_shared<Buffer>(update_buffer_list, dbcontent_name);

        unsigned int rec_num;
        unsigned int ds_id;

        double pos_azm_deg;
        double pos_azm_rad;
        double pos_range_nm;
        double pos_range_m;
        double altitude_ft;
        bool has_altitude;

        double lat, lon;
        unsigned int update_cnt = 0;

        assert(msg_box_);
        std::string msg;

        loginf << "RadarPlotPositionCalculatorTask: loadingDoneSlot: writing update_buffer";
        bool ret;

        size_t transformation_errors = 0;

        NullableVector<unsigned int>& read_ds_id_vec = read_buffer->get<unsigned int> (
                    dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_datasource_id_).name());
        NullableVector<unsigned int>& read_rec_num_vec = read_buffer->get<unsigned int> (
                    dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_rec_num_).name());
        NullableVector<double>& read_range_vec = read_buffer->get<double> (
                    dbcontent_man.getVariable(dbcontent_name, DBContent::var_radar_range_).name());
        NullableVector<double>& read_azimuth_vec = read_buffer->get<double> (
                    dbcontent_man.getVariable(dbcontent_name, DBContent::var_radar_azimuth_).name());
        NullableVector<float>& read_altitude_vec = read_buffer->get<float> (
                    dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mc_).name());

        NullableVector<double>& write_lat_vec = update_buffer->get<double> (
                    dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_latitude_).name());
        NullableVector<double>& write_lon_vec = update_buffer->get<double> (
                    dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_longitude_).name());
        NullableVector<unsigned int>& write_rec_num_vec = update_buffer->get<unsigned int> (
                    dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_rec_num_).name());

        assert (read_ds_id_vec.isNeverNull());
        assert (read_rec_num_vec.isNeverNull());

        for (unsigned int cnt = 0; cnt < read_size; cnt++)
        {
            rec_num = read_rec_num_vec.get(cnt);

            ds_id = read_ds_id_vec.get(cnt);

            if (read_azimuth_vec.isNull(cnt) || read_range_vec.isNull(cnt))
                continue;

            pos_azm_deg = read_azimuth_vec.get(cnt);
            pos_range_nm = read_range_vec.get(cnt);

            has_altitude = !read_altitude_vec.isNull(cnt);

            if (has_altitude)
                altitude_ft = read_altitude_vec.get(cnt);
            else
                altitude_ft = 0.0;  // has to assumed in projection later on

            pos_azm_rad = pos_azm_deg * DEG2RAD;

            pos_range_m = 1852.0 * pos_range_nm;

            if (!projection.hasCoordinateSystem(ds_id) && !ds_wo_full_pos.count(ds_id))
            {
                assert (ds_man.hasDBDataSource(ds_id));

                dbContent::DBDataSource& ds = ds_man.dbDataSource(ds_id);

                if (!ds.hasFullPosition())
                {
                    logwrn << "RadarPlotPositionCalculatorTask: loadingDoneSlot: data source " << ds.name()
                           << " does not have full position information, skipping";
                    ds_wo_full_pos.insert(ds_id);
                    continue;
                }

                projection.addCoordinateSystem(ds_id, ds.latitude(), ds.longitude(), ds.altitude());
            }

            ret = projection.polarToWGS84(ds_id, pos_azm_rad, pos_range_m, has_altitude,
                                          altitude_ft, lat, lon);

            if (!ret)
            {
                transformation_errors++;
                continue;
            }

            write_lat_vec.set(update_cnt, lat);
            write_lon_vec.set(update_cnt, lon);
            write_rec_num_vec.set(update_cnt, rec_num);

            update_cnt++;

            // loginf << "uga cnt " << update_cnt << " rec_num " << rec_num << " lat " << lat << " long
            // " << lon;
        }

        logdbg << "RadarPlotPositionCalculatorTask: loadingDoneSlot: update_buffer size "
           << update_buffer->size() << ", " << transformation_errors << " transformation errors";

        msg_box_->close();
        delete msg_box_;
        msg_box_ = nullptr;

        if (!update_buffer->size())
        {
            std::string text =
                    "There were " + std::to_string(transformation_errors) +
                    " skipped coordinates with transformation errors, no data available for insertion.";

            QMessageBox msgBox;
            msgBox.setText(text.c_str());
            msgBox.exec();

            return;
        }

        if (transformation_errors)
        {
            QApplication::restoreOverrideCursor();

            QMessageBox::StandardButton reply;

            std::string question =
                    "There were " + std::to_string(transformation_errors) +
                    " skipped coordinates with transformation errors, " +
                    std::to_string(update_buffer->size()) +
                    " coordinates were projected correctly. Do you want to insert the data?";

            reply = QMessageBox::question(nullptr, "Insert Data", question.c_str(),
                                          QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::No)
            {
                loginf << "RadarPlotPositionCalculatorTask: loadingDoneSlot: aborted by user because of "
                   "transformation errors";
                return;
            }

            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        }
        else
            task_manager_.appendInfo("RadarPlotPositionCalculatorTask: no transformation errors");

        msg_box_ = new QMessageBox;
        assert(msg_box_);
        msg_box_->setWindowTitle("Calculating Radar Plot Positions");
        msg = "Writing object data";
        msg_box_->setText(msg.c_str());
        msg_box_->setStandardButtons(QMessageBox::NoButton);
        msg_box_->show();

        logdbg << "RadarPlotPositionCalculatorTask: loadingDoneSlot: writing " << dbcontent_name
               << " size " << update_buffer->size()
               << " key " << dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_rec_num_).name();

        DBContent& dbcontent = dbcontent_man.dbContent(dbcontent_name);

        dbcontent.updateData(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_rec_num_), update_buffer);

        connect(&dbcontent, &DBContent::updateDoneSignal,
                this, &RadarPlotPositionCalculatorTask::updateDoneSlot);
    }

    loginf << "RadarPlotPositionCalculatorTask: loadingDoneSlot: end";
}

void RadarPlotPositionCalculatorTask::updateDoneSlot(DBContent& db_content)
{
    loginf << "RadarPlotPositionCalculatorTask: updateDoneSlot";

    disconnect(&db_content, &DBContent::updateDoneSignal,
               this, &RadarPlotPositionCalculatorTask::updateDoneSlot);

    assert (data_.count(db_content.name()));

    assert (!dbcontent_done_.count(db_content.name()));
    dbcontent_done_.insert(db_content.name());

    if (dbcontent_done_.size() == data_.size())
    {
        loginf << "RadarPlotPositionCalculatorTask: updateDoneSlot: fully done";

        dialog_ = nullptr;
        data_.clear();

        assert(msg_box_);
        msg_box_->close();
        delete msg_box_;
        msg_box_ = nullptr;

        stop_time_ = boost::posix_time::microsec_clock::local_time();
        boost::posix_time::time_duration time_diff = stop_time_ - start_time_;
        std::string elapsed_time_str =
                String::timeStringFromDouble(time_diff.total_milliseconds() / 1000.0, false);

        done_ = true;
        COMPASS::instance().interface().setProperty(DONE_PROPERTY_NAME, "1");

        task_manager_.appendSuccess("RadarPlotPositionCalculatorTask: done after " + elapsed_time_str);

        QApplication::restoreOverrideCursor();

        msg_box_ = new QMessageBox;
        assert(msg_box_);
        msg_box_->setWindowTitle("Calculating Radar Plot Positions");
        msg_box_->setText("Writing of object data done.");
        msg_box_->setStandardButtons(QMessageBox::Ok);

        if (show_done_summary_)
            msg_box_->exec();

        delete msg_box_;
        msg_box_ = nullptr;

        emit doneSignal(name_);
    }
    else
        loginf << "RadarPlotPositionCalculatorTask: updateDoneSlot: not yet done";
}

void RadarPlotPositionCalculatorTask::dialogCloseSlot()
{
    assert (dialog_);

    bool run_wanted = dialog_->runWanted();

    dialog_->close();

    if (run_wanted)
        run();
    else
        dialog_ = nullptr;
}

bool RadarPlotPositionCalculatorTask::isCalculating() { return calculating_; }

dbContent::VariableSet RadarPlotPositionCalculatorTask::getReadSetFor(const std::string& dbcontent_name)
{
    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    VariableSet read_set;

    assert(dbcontent_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_rec_num_));
    read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_rec_num_));

    assert(dbcontent_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_datasource_id_));
    read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_datasource_id_));

    assert(dbcontent_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_mc_));
    read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mc_));

    assert(dbcontent_man.canGetVariable(dbcontent_name, DBContent::var_radar_range_));
    read_set.add(dbcontent_man.getVariable(dbcontent_name, DBContent::var_radar_range_));

    assert(dbcontent_man.canGetVariable(dbcontent_name, DBContent::var_radar_azimuth_));
    read_set.add(dbcontent_man.getVariable(dbcontent_name, DBContent::var_radar_azimuth_));

    return read_set;
}
