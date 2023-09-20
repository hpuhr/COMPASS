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

#ifndef GPSIMPORTCSVTASKWIDGET_H
#define GPSIMPORTCSVTASKWIDGET_H

#include <taskwidget.h>

class GPSImportCSVTask;

class QPushButton;
class QLabel;
class QTextEdit;
class QTabWidget;
class QHBoxLayout;
class QLineEdit;
class QCheckBox;

class GPSImportCSVTaskWidget : public TaskWidget
{
    Q_OBJECT

  public slots:
    void sacEditedSlot(const QString& value);
    void sicEditedSlot(const QString& value);
    void nameEditedSlot(const QString& value);

    void todOffsetEditedSlot(const QString& value);

    void mode3ACheckedSlot();
    void mode3AEditedSlot(const QString& value);

    void targetAddressCheckedSlot();
    void targetAddressEditedSlot(const QString& value);

    void callsignCheckedSlot();
    void callsignEditedSlot(const QString& value);

    void lineIDEditSlot(const QString& text);

public:
    GPSImportCSVTaskWidget(GPSImportCSVTask& task, QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~GPSImportCSVTaskWidget();

    void selectFile(const std::string& filename);

    void updateConfig ();
    void updateText ();

    void expertModeChangedSlot();

protected:
    GPSImportCSVTask& task_;

    QHBoxLayout* main_layout_{nullptr};

    QTabWidget* tab_widget_{nullptr};

    QLabel* file_label_{nullptr};

    QTextEdit* text_edit_ {nullptr};

    QLineEdit* sac_edit_ {nullptr};
    QLineEdit* sic_edit_ {nullptr};
    QLineEdit* name_edit_ {nullptr};

    QLineEdit* tod_offset_edit_ {nullptr};

    QCheckBox* set_mode_3a_code_check_ {nullptr};
    QLineEdit* mode_3a_code_edit_ {nullptr};

    QCheckBox* set_target_address_check_ {nullptr};
    QLineEdit* target_address_edit_ {nullptr};

    QCheckBox* set_callsign_check_ {nullptr};
    QLineEdit* callsign_edit_ {nullptr};

    void addMainTab();
    void addConfigTab();
};

#endif // GPSIMPORTCSVTASKWIDGET_H
