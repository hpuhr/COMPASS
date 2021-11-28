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

#ifndef TASK_H
#define TASK_H

#include <QObject>

class TaskManager;
class TaskWidget;

class Task : public QObject
{
    Q_OBJECT

  signals:
    void statusChangedSignal(std::string task_name);  // emitted when settings where changes which
                                                      // influence the prerequisites
    void doneSignal(std::string task_name);           // emitted when task is done

  public:
    Task(const std::string& name, const std::string& gui_name, bool gui_only, bool expert_only,
         TaskManager& task_manager)
        : name_(name),
          gui_name_(gui_name),
          gui_only_(gui_only),
          expert_only_(expert_only),
          task_manager_(task_manager)
    {
    }
    virtual ~Task() {}

    std::string name() const { return name_; }

//    virtual TaskWidget* widget() = 0;
//    virtual void deleteWidget() = 0;

    bool guiOnly() const { return gui_only_; }

    std::string guiName() const { return gui_name_; }

    virtual bool checkPrerequisites() = 0;        // returns true can be shown, false if not yet
    virtual bool canRun() { return !gui_only_; }  // returns true if can be run, to be overriden
    virtual bool isRecommended() = 0;  // returns true if it is recommended to run this task
    virtual bool isRequired() = 0;     // returns true if it is required to run this task

    bool expertOnly() const { return expert_only_; }

    bool done() const { return done_; }

    virtual void run() { assert(!gui_only_); }  // to be overriden by tasks that can run

    TaskManager& manager() const { return task_manager_; }

    std::string tooltip() const { return tooltip_; }

    bool showDoneSummary() const { return show_done_summary_; }

    void showDoneSummary(bool value) { show_done_summary_ = value; }

  protected:
    std::string name_;
    std::string gui_name_;
    bool gui_only_{false};
    bool expert_only_{false};
    bool done_{false};
    bool show_done_summary_{true};

    std::string tooltip_;

    TaskManager& task_manager_;
};

#endif  // TASK_H
