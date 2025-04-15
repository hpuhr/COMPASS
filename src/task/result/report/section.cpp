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

#include "task/result/report/section.h"
#include "task/result/report/sectionid.h"
#include "task/result/report/sectioncontent.h"
#include "task/result/report/sectioncontenttext.h"
#include "task/result/report/sectioncontenttable.h"
#include "task/result/report/sectioncontentfigure.h"
#include "task/result/report/report.h"

//#include "latexvisitor.h"

#include "logger.h"

#include <QVBoxLayout>
#include <QApplication>
#include <QThread>

namespace ResultReport
{

const std::string Section::DBTableName         = "report_sections";
const Property    Section::DBColumnSectionID   = Property("section_id"  , PropertyDataType::UINT);
const Property    Section::DBColumnReportID    = Property("report_id"   , PropertyDataType::UINT);
const Property    Section::DBColumnJSONContent = Property("json_content", PropertyDataType::JSON);

const std::string Section::FieldID                  = "section_id";
const std::string Section::FieldHeading             = "heading";
const std::string Section::FieldParentHeading       = "parent_heading";
const std::string Section::FieldPerTarget           = "per_target_section";
const std::string Section::FieldPerTargetWithIssues = "per_target_section_with_issues";
const std::string Section::FieldSubSections         = "sub_sections";
const std::string Section::FieldContentIDs          = "content_ids";
const std::string Section::FieldContentNames        = "content_names";
const std::string Section::FieldContentTypes        = "content_types";
const std::string Section::FieldExtraContentIDs     = "extra_content_ids";

unsigned int Section::current_content_id_ = 0;

/**
*/
Section::Section(const std::string& heading, 
                 const std::string& parent_heading, 
                 TreeItem* parent_item,
                 Report* report)
:   TreeItem       (heading, parent_item)
,   heading_       (heading)
,   parent_heading_(parent_heading)
,   report_        (report)
{
}

/**
*/
Section::Section(TreeItem* parent_item,
                 Report* report)
:   TreeItem (parent_item)
,   report_  (report)
{
}

/**
*/
unsigned int Section::newContentID()
{
    return current_content_id_++;
}

/**
*/
void Section::setCurrentContentID(unsigned int id)
{
    current_content_id_ = id;
}

/**
*/
TreeItem* Section::child(int row)
{
    if (row < 0 || row >= (int)sub_sections_.size())
        return nullptr;

    return sub_sections_.at(row).get();
}

/**
*/
int Section::childCount() const
{
    return sub_sections_.size();
}

/**
*/
int Section::columnCount() const
{
    return 1;
}

/**
*/
QVariant Section::data(int column) const
{
    assert (column == 0);

    return heading_.c_str();
}

/**
*/
int Section::row() const
{
    return 0;
}

/**
*/
std::string Section::heading() const
{
    return heading_;
}

/**
*/
std::string Section::compoundHeading() const
{
    if (parent_heading_.size())
        return parent_heading_+":"+heading_;
    else
        return heading_;
}

/**
*/
std::string Section::compoundResultsHeading() const
{
    std::string tmp;

    if (parent_heading_.size())
        tmp = SectionID::sectionID(parent_heading_, heading_);
    else
        tmp = heading_;

    //@TODO
    //return SectionID::sectionIDWithoutResults(tmp);

    return "";
}

/**
*/
bool Section::hasSubSection (const std::string& heading)
{
    return findSubSection(heading) != nullptr;
}

/**
*/
Section& Section::getSubSection (const std::string& heading)
{
    assert (hasSubSection(heading));

    Section* tmp = findSubSection (heading);
    assert (tmp);
    return *tmp;
}

/**
*/
void Section::addSubSection (const std::string& heading)
{
    logdbg << "Section " << heading_ << ": addSubSection: adding " << heading;

    assert (!hasSubSection(heading));

    sub_sections_.push_back(std::make_shared<Section>(heading, compoundHeading(), this, report_));
    assert (hasSubSection(heading));
}

/**
*/
std::vector<std::shared_ptr<Section>> Section::subSections(bool recursive) const
{
    std::vector<std::shared_ptr<Section>> sections = sub_sections_;

    if (recursive)
    {
        for (const auto& s : sub_sections_)
        {
            auto subs = s->subSections(true);
            sections.insert(sections.end(), subs.begin(), subs.end());
        }
    }

    return sections;
}

/**
*/
QWidget* Section::getContentWidget()
{
    if (!content_widget_)
    {
        createContentWidget();
        assert(content_widget_);
    }

    return content_widget_.get();
}

/**
*/
bool Section::hasText (const std::string& name)
{
    return hasContent(name, SectionContent::Type::Text);
}

/**
*/
SectionContentText& Section::getText (const std::string& name)
{
    auto idx = findContent(name, SectionContent::Type::Text);
    assert(idx.has_value());

    auto c = loadOrGetContent(idx.value(), false);
    assert(c);

    SectionContentText* tmp = dynamic_cast<SectionContentText*>(c.get());
    assert (tmp);

    return *tmp;
}

/**
*/
SectionContentText& Section::addText(const std::string& name)
{
    assert (!hasText(name));
    auto id = Section::newContentID();
    content_ids_.push_back(id);
    content_names_.push_back(name);

    auto ptr = std::make_shared<SectionContentText>(id, name, this);
    
    content_.push_back(ptr);
    content_types_.push_back((int)content_.back()->type());
    assert (hasText(name));

    return *ptr;
}

/**
*/
size_t Section::numTexts() const
{
    return numContents(SectionContent::Type::Text);
}

/**
*/
bool Section::hasTable(const std::string& name)
{
    return hasContent(name, SectionContent::Type::Table);
}

/**
*/
SectionContentTable& Section::getTable(const std::string& name)
{
    auto idx = findContent(name, SectionContent::Type::Table);
    assert(idx.has_value());

    auto c = loadOrGetContent(idx.value(), false);
    assert(c);

    SectionContentTable* tmp = dynamic_cast<SectionContentTable*>(c.get());
    assert (tmp);

    return *tmp;
}

/**
*/
std::vector<std::string> Section::getTableNames() const
{
    std::vector<std::string> names;

    auto idxs = findContents(SectionContent::Type::Table);

    for (auto idx : idxs)
        names.push_back(content_names_.at(idx));

    return names;
}

/**
*/
SectionContentTable& Section::addTable(const std::string& name, 
                                       unsigned int num_columns,
                                       std::vector<std::string> headings, 
                                       bool sortable, 
                                       unsigned int sort_column, 
                                       Qt::SortOrder order)
{
    assert (!hasTable(name));
    auto id = Section::newContentID();
    content_ids_.push_back(id);
    content_names_.push_back(name);

    auto ptr = std::make_shared<SectionContentTable>(id, 
                                                     name, 
                                                     num_columns, 
                                                     headings, 
                                                     this, 
                                                     sortable, 
                                                     sort_column, 
                                                     order);

    content_.push_back(ptr);

    content_types_.push_back((int)content_.back()->type());
    assert (hasTable(name));

    return *ptr;
}

/**
*/
size_t Section::numTables() const
{
    return numContents(SectionContent::Type::Table);
}

/**
*/
bool Section::hasFigure (const std::string& name)
{
    return hasContent(name, SectionContent::Type::Figure);
}

/**
*/
SectionContentFigure& Section::getFigure (const std::string& name)
{
    auto idx = findContent(name, SectionContent::Type::Figure);
    assert(idx.has_value());

    auto c = loadOrGetContent(idx.value(), false);
    assert(c);

    SectionContentFigure* tmp = dynamic_cast<SectionContentFigure*>(c.get());
    assert (tmp);

    return *tmp;
}

/**
*/
SectionContentFigure& Section::addFigure(const std::string& name, 
                                         const SectionContentViewable& viewable)
{
    assert (!name.empty());
    assert (!hasFigure(name));

    unsigned int id = Section::newContentID();
    content_ids_.push_back(id);
    content_names_.push_back(name);

    auto ptr = std::make_shared<SectionContentFigure>(id, 
                                                      SectionContentFigure::FigureType::Section,
                                                      name, 
                                                      viewable, 
                                                      this);

    content_.push_back(ptr);
    content_types_.push_back((int)content_.back()->type());
    assert (hasFigure(name));

    return *ptr;
}

/**
*/
unsigned int Section::addContentFigure(const SectionContentViewable& viewable)
{
    unsigned int id = Section::newContentID();
    extra_content_ids_.push_back(id);
    extra_content_.push_back(std::make_shared<SectionContentFigure>(id, 
                                                                    SectionContentFigure::FigureType::Content,
                                                                    "", 
                                                                    viewable, 
                                                                    this));
    return id;
}

/**
*/
std::vector<SectionContentFigure*> Section::getFigures()
{
    std::vector<SectionContentFigure*> figures;

    auto idxs = findContents(SectionContent::Type::Figure);

    for (auto idx : idxs)
    {
        auto c = loadOrGetContent(idx, false);
        assert(c);

        auto fig = dynamic_cast<SectionContentFigure*>(c.get());
        assert(fig);

        figures.push_back(fig);
    }

    return figures;
}

/**
*/
size_t Section::numFigures() const
{
    return numContents(SectionContent::Type::Figure);
}

/**
*/
unsigned int Section::numSections()
{
    unsigned int num = 1; // me

    for (auto& sec_it : sub_sections_)
        num += sec_it->numSections();

    return num;
}

/**
*/
void Section::addSectionsFlat(std::vector<std::shared_ptr<Section>>& result, 
                              bool include_target_details,
                              bool report_skip_targets_wo_issues)
{
    //@TODO
    // if (!include_target_details && compoundHeading() == SectionID::targetResultsID())
    //     return;

    // if (report_skip_targets_wo_issues && perTargetSection())
    // {
    //     if (perTargetWithIssues())
    //     {
    //         logdbg << "Section: addSectionsFlat: not skipping section " << compoundHeading();
    //     }
    //     else
    //     {
    //         logdbg << "Section: addSectionsFlat: skipping section " << compoundHeading();
    //         return;
    //     }
    // }

    // for (auto& sec_it : sub_sections_)
    // {
    //     if (!include_target_details && sec_it->compoundHeading() == SectionID::targetResultsID())
    //         continue;

    //     if (report_skip_targets_wo_issues && sec_it->perTargetSection())
    //     {
    //         if (sec_it->perTargetWithIssues())
    //         {
    //                 logdbg << "Section: addSectionsFlat: not skipping sub-section " << compoundHeading();
    //         }
    //         else
    //         {
    //             logdbg << "Section: addSectionsFlat: skipping sub-section" << compoundHeading();
    //             continue;
    //         }
    //     }

    //     result.push_back(sec_it);
    //     sec_it->addSectionsFlat(result, include_target_details, report_skip_targets_wo_issues);
    // }
}

/**
*/
void Section::accept(LatexVisitor& v) const
{
    loginf << "Section: accept";
    //@TODO
    //v.visit(this);
}

/**
*/
const std::vector<std::shared_ptr<SectionContent>>& Section::sectionContent() const
{
    return content_;
}

/**
*/
std::vector<std::shared_ptr<SectionContent>> Section::recursiveContent() const
{
    std::vector<std::shared_ptr<SectionContent>> sec_content;
    sec_content.insert(sec_content.end(), content_.begin(), content_.end());
    sec_content.insert(sec_content.end(), extra_content_.begin(), extra_content_.end());

    for (const auto& s : sub_sections_)
    {
        auto c = s->recursiveContent();
        sec_content.insert(sec_content.end(), c.begin(), c.end());
    }

    return sec_content;
}

/**
*/
bool Section::perTargetWithIssues() const
{
    return per_target_section_with_issues_;
}

/**
*/
void Section::perTargetWithIssues(bool value)
{
    per_target_section_with_issues_ = value;
}

/**
*/
bool Section::perTargetSection() const
{
    return per_target_section_;
}

/**
*/
void Section::perTargetSection(bool value)
{
    per_target_section_ = value;
}

/**
*/
Section* Section::findSubSection(const std::string& heading)
{
    for (auto& sec_it : sub_sections_)
    {
        if (sec_it->heading() == heading)
            return sec_it.get();
    }

    return nullptr;
}

/**
*/
boost::optional<size_t> Section::findContent(const std::string& name, SectionContent::Type type) const
{
    for (size_t i = 0; i < content_.size(); ++i)
    {
        //loginf << "name: " << content_names_[ i ] << " vs " << name << " - type: " << content_types_[ i ] << " vs " << (int)type;

        if (content_names_[ i ] == name && (SectionContent::Type)content_types_[ i ] == type)
            return i;
    }

    return boost::optional<size_t>();
}

/**
*/
std::vector<size_t> Section::findContents(SectionContent::Type type) const
{
    std::vector<size_t> idxs;

    for (size_t i = 0; i < content_.size(); ++i)
    {
        if ((SectionContent::Type)content_types_[ i ] == type)
            idxs.push_back(i);
    }

    return idxs;
}

/**
*/
bool Section::hasContent(const std::string& name, SectionContent::Type type) const
{
    return findContent(name, type).has_value();
}

/**
*/
size_t Section::numContents(SectionContent::Type type) const
{
    size_t num = 0;

    for (size_t i = 0; i < content_.size(); ++i)
    {
        if ((SectionContent::Type)content_types_[ i ] == type)
            ++num;
    }

    return num;
}

/**
*/
void Section::createContentWidget()
{
    loginf << "Section: createContentWidget: Creating content widget for section '" << name_ << "'";

    assert (!content_widget_);

    content_widget_.reset(new QWidget());

    QVBoxLayout* layout = new QVBoxLayout();

    for (size_t i = 0; i < content_.size(); ++i)
        loadOrGetContent(i, false)->addToLayout(layout);

    //layout->addStretch();

    content_widget_->setLayout(layout);
}

namespace 
{
    boost::optional<size_t> findContentInVector(const std::vector<unsigned int>& ids,
                                                unsigned int id)
    {
        auto it = std::lower_bound(ids.begin(), ids.end(), id);
        if (it == ids.end() || *it != id)
            return {};

        return std::distance(ids.begin(), it);
    }
}

/**
*/
std::shared_ptr<SectionContent> Section::retrieveContent(unsigned int id)
{
    bool in_extra = false;

    auto idx = findContentInVector(content_ids_, id);
    if (!idx.has_value())
    {
        idx = findContentInVector(extra_content_ids_, id);
        in_extra = true;
    }

    if (!idx.has_value())
        return std::shared_ptr<SectionContent>();

    return loadOrGetContent(idx.value(), in_extra);
}

/**
*/
std::shared_ptr<SectionContent> Section::loadOrGetContent(size_t idx, bool is_extra_content)
{
    auto& c_ptr = is_extra_content ? extra_content_.at(idx) : content_.at(idx);
    auto  id    = is_extra_content ? extra_content_ids_.at(idx) : content_ids_.at(idx);

    if (c_ptr)
        return c_ptr;

    auto c = report_->loadContent(this, id);
    assert(c);

    c_ptr = c;

    return c_ptr;
}

/**
*/
nlohmann::json Section::toJSON() const
{
    nlohmann::json root;

    root[ FieldHeading             ] = heading_;
    root[ FieldParentHeading       ] = parent_heading_;
    root[ FieldPerTarget           ] = per_target_section_;
    root[ FieldPerTargetWithIssues ] = per_target_section_with_issues_;
    root[ FieldContentIDs          ] = content_ids_;
    root[ FieldContentNames        ] = content_names_;
    root[ FieldContentTypes        ] = content_types_;
    root[ FieldExtraContentIDs     ] = extra_content_ids_;

    nlohmann::json j_subsections = nlohmann::json::array();

    for (const auto& s : sub_sections_)
    {
        nlohmann::json js = s->toJSON();
        j_subsections.push_back(js);
    }

    root[ FieldSubSections ] = j_subsections;

    return root;
}

/**
*/
bool Section::fromJSON(const nlohmann::json& j)
{
    if (!j.is_object()                        ||
        !j.contains(FieldHeading)             ||
        !j.contains(FieldParentHeading)       ||
        !j.contains(FieldPerTarget)           ||
        !j.contains(FieldPerTargetWithIssues) ||
        !j.contains(FieldSubSections)         ||
        !j.contains(FieldContentIDs)          ||
        !j.contains(FieldContentNames)        ||
        !j.contains(FieldContentTypes)        ||
        !j.contains(FieldExtraContentIDs))
    {
        logerr << "Section: fromJSON: Error: Section does not obtain needed fields";
        return false;
    }

    try
    {
        //root[ FieldID ]
        heading_                        = j[ FieldHeading             ];
        parent_heading_                 = j[ FieldParentHeading       ];
        per_target_section_             = j[ FieldPerTarget           ];
        per_target_section_with_issues_ = j[ FieldPerTargetWithIssues ];
        content_ids_                    = j[ FieldContentIDs          ].get<std::vector<unsigned int>>();
        content_names_                  = j[ FieldContentNames        ].get<std::vector<std::string>>();
        content_types_                  = j[ FieldContentTypes        ].get<std::vector<int>>();
        extra_content_ids_              = j[ FieldExtraContentIDs     ].get<std::vector<unsigned int>>();

        content_.resize(content_ids_.size());
        extra_content_.resize(extra_content_ids_.size());

        assert(content_ids_.size() == content_names_.size());
        assert(content_ids_.size() == content_types_.size());

        //restore TreeItem content
        setItemName(heading_);

        const auto& j_subsections = j[ FieldSubSections ];
        if (!j_subsections.is_array())
        {
            logerr << "Section: fromJSON: Error: Subsection is not an array";
            return false;
        }

        for (const auto& jss : j_subsections)
        {
            std::shared_ptr<Section> section(new Section(this, report_));
            if (!section->fromJSON(jss))
                return false;

            sub_sections_.push_back(section);
        }
    }
    catch(const std::exception& ex)
    {
        logerr << "Section: fromJSON: Error: " << ex.what();
        return false;
    }
    catch(...)
    {
        logerr << "Section: fromJSON: Error: Unknown JSON error";
        return false;
    }

    return true;
}

}
