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

    setLayout(main_layout_);
}

ViewPointsImportTaskWidget::~ViewPointsImportTaskWidget()
{
    logdbg << "ViewPointsImportTaskWidget: destructor";

}

void ViewPointsImportTaskWidget::expertModeChangedSlot() {}
