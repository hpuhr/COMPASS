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

#ifndef CREATEASSOCIATIONSSTATUSDIALOG_H
#define CREATEASSOCIATIONSSTATUSDIALOG_H

#include <QDialog>

#include "boost/date_time/posix_time/posix_time.hpp"

class QLabel;
class QPushButton;
class QGridLayout;
class CreateAssociationsTask;

class CreateAssociationsStatusDialog : public QDialog
{
    Q_OBJECT

signals:
    void closeSignal();

public slots:
    void okClickedSlot();

public:
    CreateAssociationsStatusDialog(CreateAssociationsTask& task,
                                   QWidget* parent=nullptr, Qt::WindowFlags f=0);

    void markStartTime();
    void setDone();

    void setDBODoneFlags(const std::map<std::string, bool>& dbo_done_flags);

    void setAssociationStatus(const std::string& status);

private:
    CreateAssociationsTask& task_;

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;
    boost::posix_time::time_duration time_diff_;
    std::string elapsed_time_str_;

    std::map<std::string, bool> dbo_done_flags_;

    std::string association_status_{"Waiting"};

    std::map<std::string, unsigned int> association_counts_;

    QLabel* time_label_{nullptr};

    QGridLayout* dbo_done_grid_{nullptr};

    QLabel* association_status_label_{nullptr};

    QGridLayout* dbo_associated_grid_{nullptr};

    QPushButton* ok_button_{nullptr};

    void updateTime();
    void updateDBODoneGrid();
    void updateDBOAssociatedGrid();
};

#endif // CREATEASSOCIATIONSSTATUSDIALOG_H
