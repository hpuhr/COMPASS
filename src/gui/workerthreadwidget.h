/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * WorkerThreadWidget.h
 *
 *  Created on: Apr 12, 2012
 *      Author: sk
 */

#ifndef WorkerThreadWidget_H_
#define WorkerThreadWidget_H_

#include <QFrame>

class WorkerThreadManager;
class QLabel;

/**
 * @brief Qt frame widget, which presents information about the TransformationManager
 *
 * Simply uses a number of labels to display information about the processing of TransformationJobs and state of each WorkerThread.
 */
class WorkerThreadWidget : public QFrame
{
  Q_OBJECT
public:
  // @brief Constructor
  WorkerThreadWidget();
  // @brief Desctructor
  virtual ~WorkerThreadWidget();

  // @brief Update the information labels
  void update ();

protected:
  /// The represented WorkerThreadManager
  WorkerThreadManager &manager_;
  /// Number of TransformationWorkers
  unsigned int num_workers_;

  int timer_id_;

  /// Number of jobs label
  QLabel *jobs_;

  /// Labels for WorkerThread states
  std::vector <QLabel *> thread_states_;
  /// Labels for WorkerThread working percentage
  std::vector <QLabel *> thread_working_;
  /// Labels for WorkerThread idle percentage
  std::vector <QLabel *> thread_idle_;
  /// Labels for WorkerThread queued jobs
  std::vector <QLabel *> thread_jobs_;

  /// @brief creates the GUI elements
  void createWidgets ();

  void timerEvent (QTimerEvent *e);
};

#endif /* WorkerThreadWidget_H_ */
