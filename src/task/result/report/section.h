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
#include "task/result/report/sectioncontent.h"
#include "property.h"

#include <QWidget>

#include "json.hpp"

#include <memory>
#include <vector>

#include <boost/optional.hpp>

class LatexVisitor;

using namespace std;

class TaskManager;
class DBInterface;

namespace ResultReport
{

class Report;
class SectionContentText;
class SectionContentTable;
class SectionContentFigure;
struct SectionContentViewable;

/**
 */
class Section : public TreeItem
{
public:
    Section(const string& heading, 
            const string& parent_heading, 
            TreeItem* parent_item,
            Report* report);
    Section(TreeItem* parent_item,
            Report* report);

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
    unsigned int addFigure (const std::string& name, 
                            const SectionContentViewable& viewable);
    std::vector<SectionContentFigure*> getFigures();

    std::shared_ptr<SectionContent> retrieveContent(unsigned int id);

    unsigned int numSections(); // all sections contained
    void addSectionsFlat (vector<shared_ptr<Section>>& result, bool include_target_details,
                            bool report_skip_targets_wo_issues);

    virtual void accept(LatexVisitor& v) const;

    const vector<shared_ptr<SectionContent>>& sectionContent() const;
    vector<shared_ptr<SectionContent>> recursiveContent() const;

    bool perTargetSection() const; // to be used for utn and sub-sections
    void perTargetSection(bool value);

    bool perTargetWithIssues() const; // te be set if requirement (any) requirement failed
    void perTargetWithIssues(bool value);

    nlohmann::json toJSON() const;
    bool fromJSON(const nlohmann::json& j);

    Report* report() { return report_; }
    const Report* report() const { return report_; }

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
    static const std::string FieldContentIDs;
    static const std::string FieldContentNames;
    static const std::string FieldContentTypes;
    static const std::string FieldExtraContentIDs;

    static void setCurrentContentID(unsigned int id);

protected:
    friend class SectionContentTable;
    friend class DBInterface;
    friend class Report;

    void addTableInternal(const std::string& name, unsigned int num_columns, vector<string> headings,
        bool sortable=true, unsigned int sort_column=0, Qt::SortOrder order=Qt::AscendingOrder);

    Section* findSubSection (const std::string& heading); // nullptr if not found
    boost::optional<size_t> findContent(const std::string& name, SectionContent::Type type) const;
    std::vector<size_t> findContents(SectionContent::Type type) const;
    bool hasContent(const std::string& name, SectionContent::Type type) const;

    void createContentWidget();

    unsigned int addContentFigure(const SectionContentViewable& viewable);

    shared_ptr<SectionContent> loadOrGetContent(size_t idx, bool is_extra_content);

    static unsigned int newContentID();

    string heading_; // name same as heading
    string parent_heading_; // e.g. "head1:head2" or ""

    Report* report_ = nullptr;

    bool per_target_section_ {false};
    bool per_target_section_with_issues_ {false};

    vector<int>                        content_types_;
    vector<std::string>                content_names_;
    vector<unsigned int>               content_ids_;
    vector<shared_ptr<SectionContent>> content_;

    vector<unsigned int>               extra_content_ids_;
    vector<shared_ptr<SectionContent>> extra_content_;

    unique_ptr<QWidget> content_widget_;

    vector<shared_ptr<Section>> sub_sections_;

    static unsigned int current_content_id_;
};

}
