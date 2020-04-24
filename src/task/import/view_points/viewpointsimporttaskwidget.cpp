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

#include "viewpointsimporttaskwidget.h"
#include "viewpointsimporttask.h"
#include "atsdb.h"
#include "global.h"
#include "logger.h"
#include "stringconv.h"

#include <QComboBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QStackedWidget>
#include <QTextEdit>
#include <QVBoxLayout>


ViewPointsImportTaskWidget::ViewPointsImportTaskWidget(ViewPointsImportTask& task, QWidget* parent)
    : TaskWidget(parent), task_(task)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* main_layout_ = new QVBoxLayout();

    import_button_ = new QPushButton("Import");
    connect (import_button_, &QPushButton::clicked, this, &ViewPointsImportTaskWidget::importSlot);
    main_layout_->addWidget(import_button_);

    setLayout(main_layout_);
}

ViewPointsImportTaskWidget::~ViewPointsImportTaskWidget()
{
    logdbg << "ViewPointsImportTaskWidget: destructor";

}

void ViewPointsImportTaskWidget::expertModeChangedSlot() {}

void ViewPointsImportTaskWidget::importSlot()
{
    task_.currentFilename("/home/sk/data/austria/20200417_CommandLine_Sample/view_points_test.json");

    if (task_.currentError().size())
    {
        loginf << "ViewPointsImportTaskWidget: importSlot: error '" << task_.currentError() << "'";
        return;
    }

    if (!task_.canImport())
    {
        loginf << "ViewPointsImportTaskWidget: importSlot: task can not import";
        return;
    }

    task_.import();
}
