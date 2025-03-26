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

#pragma once

#include "task/result/report/treeitem.h"
#include "property.h"

#include <QWidget>

#include "json.hpp"

#include <memory>
#include <vector>

class LatexVisitor;

using namespace std;

class TaskManager;

namespace ResultReport
{

class SectionContent;
class SectionContentText;
class SectionContentTable;
class SectionContentFigure;

/**
 */
class Section : public TreeItem
{
public:
    Section(const string& heading, 
            const string& parent_heading, 
            TreeItem* parent_item,
            TaskManager& task_man);
    Section(TreeItem* parent_item,
            TaskManager& task_man);

    virtual TreeItem* child(int row) override;
    virtual int childCount() const override;
    virtual int columnCount() const override;
    virtual QVariant data(int column) const override;
    virtual int row() const override;

    string heading() const;
    string compoundHeading() const; // "head1:head2" or "head1", starts with "Results"
    string compoundResultsHeading() const; // without "Results", can be ""

    bool hasSubSection (const std::string& heading);
    Section& getSubSection (const std::string& heading);
    void addSubSection (const std::string& heading);
    std::vector<std::shared_ptr<Section>> subSections(bool recursive) const;

    QWidget* getContentWidget();

    bool hasText (const std::string& name);
    SectionContentText& getText (const std::string& name);
    void addText (const std::string& name);

    bool hasTable (const std::string& name);
    SectionContentTable& getTable (const std::string& name);
    void addTable (const std::string& name, unsigned int num_columns, vector<string> headings,
                    bool sortable=true, unsigned int sort_column=0, Qt::SortOrder order=Qt::AscendingOrder);
    std::vector<std::string> getTableNames() const;

    bool hasFigure (const std::string& name);
    SectionContentFigure& getFigure (const std::string& name);
    void addFigure (const std::string& name, const string& caption,
                    std::function<std::shared_ptr<nlohmann::json::object_t>(void)> viewable_fnc,
                    int render_delay_msec = 0);
    std::vector<SectionContentFigure*> getFigures() const;

    unsigned int numSections(); // all sections contained
    void addSectionsFlat (vector<shared_ptr<Section>>& result, bool include_target_details,
                            bool report_skip_targets_wo_issues);

    virtual void accept(LatexVisitor& v) const;

    const vector<shared_ptr<SectionContent>>& content() const;
    std::vector<SectionContentFigure*> sectionsFigures(bool recursive) const;

    bool perTargetSection() const; // to be used for utn and sub-sections
    void perTargetSection(bool value);

    bool perTargetWithIssues() const; // te be set if requirement (any) requirement failed
    void perTargetWithIssues(bool value);

    nlohmann::json toJSON() const;
    bool fromJSON(const nlohmann::json& j);

    static const std::string DBTableName;
    static const Property    DBColumnSectionID;
    static const Property    DBColumnReportID;
    static const Property    DBColumnJSONContent;

    static const std::string FieldID;
    static const std::string FieldHeading;
    static const std::string FieldParentHeading;
    static const std::string FieldPerTarget;
    static const std::string FieldPerTargetWithIssues;
    static const std::string FieldSubSections;

protected:
    Section* findSubSection (const std::string& heading); // nullptr if not found
    SectionContentText* findText (const std::string& name); // nullptr if not found
    SectionContentTable* findTable (const std::string& name); // nullptr if not found
    SectionContentFigure* findFigure (const std::string& name); // nullptr if not found

    void createContentWidget();

    static unsigned int newContentID();

    string heading_; // name same as heading
    string parent_heading_; // e.g. "head1:head2" or ""

    TaskManager& task_man_;

    bool per_target_section_ {false};
    bool per_target_section_with_issues_ {false};

    vector<shared_ptr<SectionContent>> content_;

    unique_ptr<QWidget> content_widget_;

    vector<shared_ptr<Section>> sub_sections_;

    static unsigned int current_content_id_;
};

}
