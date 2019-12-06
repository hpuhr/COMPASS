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

#ifndef ASTERIXCONFIGWIDGET_H
#define ASTERIXCONFIGWIDGET_H

#include <QWidget>
#include <memory>

#include <jasterix/jasterix.h>

class ASTERIXImporterTask;

class QVBoxLayout;
class QGridLayout;
class QComboBox;
class QPushButton;
class ASTERIXFramingComboBox;

class ASTERIXConfigWidget : public QWidget
{
    Q_OBJECT

signals:

public slots:
    void framingChangedSlot();
    void framingEditSlot();

    void categoryCheckedSlot ();
    void editionChangedSlot(const std::string& cat_str, const std::string& ed_str);
    void refEditionChangedSlot(const std::string& cat_str, const std::string& ed_str);
    void spfEditionChangedSlot(const std::string& cat_str, const std::string& ed_str);
    void categoryEditionEditSlot ();
    void categoryREFEditionEditSlot ();
    void categorySPFEditionEditSlot ();

    void editDataBlockSlot();
    void editCategoriesSlot();
    void refreshjASTERIXSlot();

    void updateSlot();

public:
    ASTERIXConfigWidget(ASTERIXImporterTask& task, QWidget* parent=nullptr);
    virtual ~ASTERIXConfigWidget();

protected:
    ASTERIXImporterTask& task_;

    QVBoxLayout* main_layout_ {nullptr};
    QGridLayout* categories_grid_ {nullptr};

    ASTERIXFramingComboBox* framing_combo_ {nullptr};
    QPushButton* framing_edit_ {nullptr};

    std::map<unsigned int, QPushButton*> ref_edit_buttons_;
    std::map<unsigned int, QPushButton*> spf_edit_buttons_;

    void updateFraming ();
    void updateCategories ();
};

#endif // ASTERIXCONFIGWIDGET_H
