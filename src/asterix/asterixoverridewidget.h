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

#ifndef ASTERIXOVERRIDEWIDGET_H
#define ASTERIXOVERRIDEWIDGET_H

#include <QWidget>

class ASTERIXImportTask;

class QCheckBox;
class QLineEdit;

class ASTERIXOverrideWidget : public QWidget
{
    Q_OBJECT

  public slots:
    void updateSlot();

    void activeCheckedSlot();

    void sacOrgEditedSlot(const QString& value);
    void sicOrgEditedSlot(const QString& value);
    void sacNewEditedSlot(const QString& value);
    void sicNewEditedSlot(const QString& value);
    void todOffsetEditedSlot(const QString& value);

  public:
    ASTERIXOverrideWidget(ASTERIXImportTask& task, QWidget* parent = nullptr);
    virtual ~ASTERIXOverrideWidget();

  protected:
    ASTERIXImportTask& task_;

    QCheckBox* active_check_{nullptr};

    QLineEdit* sac_org_edit_{nullptr};
    QLineEdit* sic_org_edit_{nullptr};

    QLineEdit* sac_new_edit_{nullptr};
    QLineEdit* sic_new_edit_{nullptr};

    QLineEdit* tod_offset_edit_{nullptr};
};

#endif  // ASTERIXOVERRIDEWIDGET_H
