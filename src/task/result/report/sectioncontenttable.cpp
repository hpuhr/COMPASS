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

#include "task/result/report/sectioncontenttable.h"
#include "task/result/report/section.h"
#include "task/result/report/sectioncontentfigure.h"
#include "task/result/report/report.h"
#include "task/result/report/reportexporter.h"
#include "task/result/taskresult.h"

#include "taskmanager.h"
#include "compass.h"
#include "dbcontentmanager.h"

#include "logger.h"
#include "stringconv.h"
#include "stringmat.h"
#include "asynctask.h"
#include "files.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QMenu>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>
#include <QInputDialog>
#include <QThread>

#include <cassert>
#include <type_traits>
#include <iostream>
#include <fstream>

namespace ResultReport
{

/***************************************************************************************************
 * SectionContentTable
 ***************************************************************************************************/

const std::string SectionContentTable::FieldHeadings      = "headings";
const std::string SectionContentTable::FieldSortable      = "sortable";
const std::string SectionContentTable::FieldSortColumn    = "sort_column";
const std::string SectionContentTable::FieldSortOrder     = "order";
const std::string SectionContentTable::FieldRows          = "rows";
const std::string SectionContentTable::FieldAnnotations   = "annotations";
const std::string SectionContentTable::FieldColumnStyles  = "column_styles";
const std::string SectionContentTable::FieldCellStyles    = "cell_styles";
const std::string SectionContentTable::FieldShowTooltips  = "show_tooltips";
const std::string SectionContentTable::FieldMaxRowCount   = "max_row_count";

const std::string SectionContentTable::FieldDocColumns  = "columns";
const std::string SectionContentTable::FieldDocData     = "data";
const std::string SectionContentTable::FieldDocPath     = "path";

const std::string SectionContentTable::FieldAnnoFigureID      = "figure_id";
const std::string SectionContentTable::FieldAnnoSectionLink   = "section_link";
const std::string SectionContentTable::FieldAnnoSectionFigure = "section_figure";
const std::string SectionContentTable::FieldAnnoOnDemand      = "on_demand";
const std::string SectionContentTable::FieldAnnoIndex         = "index";
const std::string SectionContentTable::FieldAnnoStyle         = "style";

const QColor SectionContentTable::ColorTextRed    = Colors::TextRed;
const QColor SectionContentTable::ColorTextOrange = Colors::TextOrange;
const QColor SectionContentTable::ColorTextGreen  = Colors::TextGreen;
const QColor SectionContentTable::ColorTextGray   = Colors::TextGray;

const QColor SectionContentTable::ColorBGRed      = Colors::BGRed;
const QColor SectionContentTable::ColorBGOrange   = Colors::BGOrange;
const QColor SectionContentTable::ColorBGGreen    = Colors::BGGreen;
const QColor SectionContentTable::ColorBGGray     = Colors::BGGray;
const QColor SectionContentTable::ColorBGYellow   = Colors::BGYellow;

/**
 */
SectionContentTable::SectionContentTable(unsigned int id,
                                         const std::string& name,
                                         unsigned int num_columns,
                                         const std::vector<std::string>& headings,
                                         Section* parent_section,
                                         bool sortable,
                                         unsigned int sort_column,
                                         Qt::SortOrder sort_order)
:   SectionContent(ContentType::Table, id, name, parent_section)
,   num_columns_  (num_columns)
,   headings_     (headings)
,   column_styles_(num_columns, 0)
,   sortable_     (sortable)
,   sort_column_  (sort_column)
,   sort_order_   (sort_order)
{
}

/**
 */
SectionContentTable::SectionContentTable(Section* parent_section)
:   SectionContent(ContentType::Table, parent_section)
{
}

/**
 */
SectionContentTable::~SectionContentTable() = default;

/**
 */
void SectionContentTable::enableTooltips()
{
    show_tooltips_ = true;
}

/**
 */
void SectionContentTable::addRow (const nlohmann::json::array_t& row,
                                  const SectionContentViewable& viewable,
                                  const std::string& section_link,
                                  const std::string& section_figure,
                                  const QVariant& viewable_index,
                                  unsigned int row_style)
{
    assert (row.size() == num_columns_);

    rows_.push_back(row);

    //configure attached annotation
    RowAnnotation anno;
    anno.index          = viewable_index;
    anno.section_link   = section_link;
    anno.section_figure = section_figure;
    anno.on_demand      = viewable.on_demand;
    anno.style          = row_style;

    logdbg<< "SectionContentTable " << name() << ": addRow: viewable has callback " << viewable.hasCallback();

    if (!viewable.on_demand && viewable.hasCallback())
    {
        //add figure to containing section and remember id
        anno.figure_id = addFigure(viewable);
    }

    annotations_.push_back(anno);
}

/**
 */
const nlohmann::json& SectionContentTable::getData(int row, int column) const
{
    return rows_.at(row).at(column);
}

/**
 */
const nlohmann::json& SectionContentTable::getData(int row, const std::string& col_name) const
{
    int col = columnIndex(col_name);
    assert(col >= 0);

    return getData(row, col);
}

/**
 */
bool SectionContentTable::hasColumn(const std::string& col_name) const
{
    return columnIndex(col_name) >= 0;
}

/**
 */
int SectionContentTable::columnIndex(const std::string& col_name) const
{
    auto it = std::find_if(headings_.begin(), headings_.end(), [ & ] (const std::string& cname) { return col_name == cname; });
    if (it == headings_.end())
        return -1;

    return (int)std::distance(headings_.begin(), it);
}

/**
 */
void SectionContentTable::setColumnStyle(int column, unsigned int style)
{
    column_styles_.at(column) = style;
}

/**
 */
void SectionContentTable::setCellStyle(int row, int column, unsigned int style)
{
    cell_styles_[ std::make_pair(column, row) ] = style;
}

/**
 */
unsigned int SectionContentTable::cellStyle(int row, int column) const
{
    unsigned int style = 0;

    // first add column style
    style |= column_styles_.at(column);

    // overide with row style
    style |= annotations_.at(row).style;

    // override with cell stlye if available
    if (!cell_styles_.empty())
    {
        auto it = cell_styles_.find(std::make_pair(column, row));
        if (it != cell_styles_.end())
            style |= it->second;
    }

    return style;
}

/**
 */
void SectionContentTable::setMaxRowCount(const boost::optional<int>& max_row_count)
{
    max_row_count_ = max_row_count;
}

/**
 */
const boost::optional<int>& SectionContentTable::maxRowCount() const
{
    return max_row_count_;
}

/**
 */
unsigned int SectionContentTable::addFigure(const SectionContentViewable& viewable)
{
    return parentSection()->addHiddenFigure(viewable);
}

/**
 */
SectionContentTableWidget* SectionContentTable::createTableWidget() const
{
    assert(!isLocked());
    assert(!table_widget_);

    SectionContentTable* tmp = const_cast<SectionContentTable*>(this); // hacky
    table_widget_ = new SectionContentTableWidget(tmp, 
                                                  show_unused_, 
                                                  sortable_ ? sort_column_ : -1, 
                                                  sort_order_);
    return table_widget_;
}

/**
 */
const SectionContentTableWidget* SectionContentTable::tableWidget() const
{
    assert(table_widget_);
    return table_widget_;
}

/**
 */
SectionContentTableWidget* SectionContentTable::tableWidget()
{
    assert(table_widget_);
    return table_widget_;
}

/**
 */
SectionContentTableWidget* SectionContentTable::getOrCreateTableWidget()
{
    if (table_widget_)
        return table_widget_;

    return createTableWidget();
}

/**
 */
const SectionContentTableWidget* SectionContentTable::getOrCreateTableWidget() const
{
    if (table_widget_)
        return table_widget_;

    return createTableWidget();
}

/**
 */
std::string SectionContentTable::resourceExtension() const
{
    return ReportExporter::ExportTableFormat;
}

/**
 */
void SectionContentTable::addContentUI(QVBoxLayout* layout, 
                                       bool force_ui_reset)
{
    loginf << "SectionContentTable: addToLayout";

    assert (layout);

    //finalize some custom stuff before showing the table in a layout
    taskResult()->postprocessTable(this);

    //force recreation of table widget?
    if (force_ui_reset)
        table_widget_ = nullptr;

    if (isLocked())
    {
        layout->addWidget(lockStatePlaceholderWidget());
    }
    else
    {
        //add widget to layout
        auto widget = getOrCreateTableWidget();
        layout->addWidget(widget);
    }
}

/**
 */
void SectionContentTable::clearContent_impl()
{
    auto func = [ & ] ()
    {
        rows_.clear();
        annotations_.clear();
        cell_styles_.clear();
    };

    if (table_widget_)
        table_widget_->itemModel()->executeAndReset(func);
    else
        func();
}

/**
 */
bool SectionContentTable::loadOnDemand()
{
    bool ok = false;

    auto func = [ & ] ()
    {
        ok = SectionContent::loadOnDemand();
    };

    if (table_widget_)
        table_widget_->itemModel()->executeAndReset(func);
    else
        func();

    return ok;
}

namespace
{
    /**
     * Converts a json value to a QVariant for display purpose.
     */
    QVariant qVariantFromJSON(const nlohmann::json& j)
    {
        if (j.is_boolean())
            return QVariant::fromValue(j.get<bool>());
        else if (j.is_number_integer())
            return QVariant::fromValue(j.get<qint64>());  // or int, depending on your use case
        else if (j.is_number_unsigned())
            return QVariant::fromValue(j.get<quint64>());
        else if (j.is_number_float())
            return QVariant::fromValue(j.get<double>());
        else if (j.is_string())
            return QVariant::fromValue(QString::fromStdString(j.get<std::string>()));
        else if (j.is_null())
            return QVariant();
        else
            return QVariant();
    };
}

/**
 */
boost::optional<QColor> SectionContentTable::cellTextColor(unsigned int style)
{
    if (style & CellStyleTextColorRed)
        return ColorTextRed;
    else if (style & CellStyleTextColorOrange)
        return ColorTextOrange;
    else if (style & CellStyleTextColorGreen)
        return ColorTextGreen;
    else if (style & CellStyleTextColorGray)
        return ColorTextGray;

    return boost::optional<QColor>();
}

/**
 */
boost::optional<QColor> SectionContentTable::cellBGColor(unsigned int style)
{
    if (style & CellStyleBGColorRed)
        return ColorBGRed;
    else if (style & CellStyleBGColorOrange)
        return ColorBGOrange;
    else if (style & CellStyleBGColorGreen)
        return ColorBGGreen;
    else if (style & CellStyleBGColorGray)
        return ColorBGGray;
    else if (style & CellStyleBGColorYellow)
        return ColorBGYellow;

    return boost::optional<QColor>();
}

/**
 */
boost::optional<QIcon> SectionContentTable::cellIcon(const nlohmann::json& data)
{
    if (!data.is_string())
        return boost::optional<QIcon>();

    std::string icon_name;
    
    try
    {
        icon_name = data;
    }
    catch(...)
    {
        return boost::optional<QIcon>();
    }
    
    return Utils::Files::IconProvider::getIcon(icon_name);
}

/**
 */
boost::optional<bool> SectionContentTable::cellChecked(const nlohmann::json& data)
{
    if (!data.is_boolean())
        return boost::optional<bool>();

    bool check_state;

    try
    {
        check_state = data;
    }
    catch(...)
    {
        return boost::optional<bool>();
    }

    return check_state;
}

/**
 */
void SectionContentTable::cellFont(QFont& font, unsigned int style)
{
    font.setBold(style & CellStyleTextBold);
    font.setItalic(style & CellStyleTextItalic);
    font.setStrikeOut(style & CellStyleTextStrikeOut);
}

/**
 */
bool SectionContentTable::cellShowsText(unsigned int style)
{
    return !cellShowsCheckBox(style) &&
           !cellShowsIcon(style);
}

/**
 */
bool SectionContentTable::cellShowsCheckBox(unsigned int style)
{
    return (style & CellStyleCheckable) != 0;
}

/**
 */
bool SectionContentTable::cellShowsIcon(unsigned int style)
{
    return (style & CellStyleIcon) != 0;
}

/**
 */
bool SectionContentTable::cellShowsSpecialFont(unsigned int style)
{
    return cellFontIsBold(style)   ||
           cellFontIsItalic(style) ||
           cellFontIsStrikeOut(style);
}

/**
 */
bool SectionContentTable::cellFontIsBold(unsigned int style)
{
    return (style & CellStyleTextBold) != 0;
}

/**
 */
bool SectionContentTable::cellFontIsItalic(unsigned int style)
{
    return (style & CellStyleTextItalic) != 0;
}

/**
 */
bool SectionContentTable::cellFontIsStrikeOut(unsigned int style)
{
    return (style & CellStyleTextStrikeOut) != 0;
}

/**
 */
std::string SectionContentTable::cellStyle2String(unsigned int style)
{
    std::string str;

    if (style & CellStyleCheckable)
        str += (str.empty() ? "" : " | ") + std::string("Checkable");
    if (style & CellStyleIcon)
        str += (str.empty() ? "" : " | ") + std::string("Icon");

    if (style & CellStyleTextBold)
        str += (str.empty() ? "" : " | ") + std::string("Bold");
    if (style & CellStyleTextItalic)
        str += (str.empty() ? "" : " | ") + std::string("Italic");
    if (style & CellStyleTextStrikeOut)
        str += (str.empty() ? "" : " | ") + std::string("StrikeOut");

    if (style & CellStyleTextColorRed)
        str += (str.empty() ? "" : " | ") + std::string("TextRed");
    if (style & CellStyleTextColorOrange)
        str += (str.empty() ? "" : " | ") + std::string("TextOrange");
    if (style & CellStyleTextColorGreen)
        str += (str.empty() ? "" : " | ") + std::string("TextGreen");
    if (style & CellStyleTextColorGray)
        str += (str.empty() ? "" : " | ") + std::string("TextGray");

    if (style & CellStyleBGColorRed)
        str += (str.empty() ? "" : " | ") + std::string("BGRed");
    if (style & CellStyleBGColorOrange)
        str += (str.empty() ? "" : " | ") + std::string("BGOrange");
    if (style & CellStyleBGColorGreen)
        str += (str.empty() ? "" : " | ") + std::string("BGGreen");
    if (style & CellStyleBGColorGray)
        str += (str.empty() ? "" : " | ") + std::string("BGGray");
    if (style & CellStyleBGColorYellow)
        str += (str.empty() ? "" : " | ") + std::string("BGYellow");

    return str;
}

/**
 */
QVariant SectionContentTable::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role)
    {
        case Qt::DisplayRole:
        {
            logdbg << "SectionContentTable: data: display role: row " << index.row() << " col " << index.column();

            assert (index.row() >= 0);
            assert (index.row() < (int)rows_.size());
            assert (index.column() < (int)num_columns_);

            auto style = cellStyle(index.row(), index.column());

            if (cellShowsText(style))
                return qVariantFromJSON(rows_.at(index.row()).at(index.column()));

            return QVariant(); 
        }
        case Qt::BackgroundRole:
        {
            assert (index.row() >= 0);
            assert (index.row() < (int)rows_.size());

            auto style = cellStyle(index.row(), index.column());

            if (cellShowsText(style))
            {
                auto c = cellBGColor(style);
                if (c.has_value())
                    return QBrush(c.value());
            }

            return QVariant();
        }
        case Qt::ForegroundRole:
        {
            assert (index.row() >= 0);
            assert (index.row() < (int)rows_.size());
            assert (index.column() < (int)num_columns_);

            auto style = cellStyle(index.row(), index.column());

            if (style != 0)
            {
                auto c = cellTextColor(style);
                if (c.has_value())
                    return QBrush(c.value());
            }

            //custom heuristics for evaluation
            //@TODO: configure tables accordingly in evaluation results
            if (cellShowsText(style) &&
                taskResult()->type() == task::TaskResultType::Evaluation)
            {
                const auto& data = rows_.at(index.row()).at(index.column());

                if (data.is_string())
                {
                    auto txt = data.get<std::string>();

                    if (txt == "Passed")
                        return QBrush(ColorTextGreen);
                    else if (txt == "Failed")
                        return QBrush(ColorTextRed);
                }
                if (data.is_boolean())
                {
                    bool ok = data.get<bool>();

                    if (ok == true)
                        return QBrush(ColorTextGreen);
                    else if (ok == false)
                        return QBrush(ColorTextRed);
                }
            }

            return QVariant();
        }
        case Qt::DecorationRole:
        {
            auto style = cellStyle(index.row(), index.column());

            if (cellShowsIcon(style))
            {
                const auto& j = rows_.at(index.row()).at(index.column());
                auto icon = cellIcon(j);
                if (icon.has_value())
                    return icon.value();
            }

            return QVariant();
        }
        case Qt::CheckStateRole:
        {
            auto style = cellStyle(index.row(), index.column());

            if (cellShowsCheckBox(style))
            {
                const auto& j = rows_.at(index.row()).at(index.column());
                auto c = cellChecked(j);
                if (c.has_value())
                    return c.value() ? Qt::Checked : Qt::Unchecked;
            }

            return QVariant();
        }
        case Qt::FontRole:
        {
            auto style = cellStyle(index.row(), index.column());

            if (cellShowsSpecialFont(style))
            {
                QFont f;
                cellFont(f, style);
                return f;
            }

            return QVariant();
        }
        case Qt::ToolTipRole:
        {
            if (show_tooltips_)
            {
                auto tResult = taskResult();
                if (tResult->hasCustomTooltip(this, (unsigned int)index.row(), (unsigned int)index.column()))
                {
                    auto ttip = tResult->customTooltip(this, (unsigned int)index.row(), (unsigned int)index.column());
                    if (!ttip.empty())
                        return QString::fromStdString(ttip);
                }
            }

            return QVariant();
        }
        default:
        {
            return QVariant();
        }
    }
}

/**
 */
Qt::ItemFlags SectionContentTable::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto style = cellStyle(index.row(), index.column());
    if (style & CellStyleCheckable)
        return Qt::ItemIsUserCheckable;

    return Qt::NoItemFlags;
}

/**
 */
size_t SectionContentTable::numRows() const
{
    return rows_.size();
}

/**
 */
size_t SectionContentTable::numColumns() const
{
    return num_columns_;
}

/**
 */
const std::vector<std::string>& SectionContentTable::headings() const
{
    return headings_;
}

/**
 */
unsigned int SectionContentTable::filteredRowCount() const
{
    return getOrCreateTableWidget()->proxyModel()->rowCount();
}

/**
 */
std::vector<std::string> SectionContentTable::sortedRowStrings(unsigned int row, bool latex) const
{
    return getOrCreateTableWidget()->sortedRowStrings(row, latex);
}

/**
 */
bool SectionContentTable::hasReference (unsigned int row) const
{
    //obtain original data row
    unsigned int row_index = getOrCreateTableWidget()->fromProxy(row);

    const auto& annotation = annotations_.at(row_index);

    return !annotation.section_link.empty();
}

/**
 */
std::string SectionContentTable::reference(unsigned int row) const
{
    //obtain original data row
    unsigned int row_index = getOrCreateTableWidget()->fromProxy(row);

    const auto& annotation = annotations_.at(row_index);

    return annotation.section_link;
}

/**
 */
bool SectionContentTable::showUnused() const
{
    return show_unused_;
}

/**
 */
void SectionContentTable::showUnused(bool value)
{
    loginf << "SectionContentTable: showUnused: value " << value;

    getOrCreateTableWidget()->showUnused(value);

    show_unused_ = value;
}


/**
 */
void SectionContentTable::registerCallBack (const std::string& name, const std::function<void()>& func)
{
    assert (!callback_map_.count(name));
    callback_map_.emplace(name, func);
}

/**
 */
void SectionContentTable::executeCallback(const std::string& name)
{
    assert (callback_map_.count(name));
    callback_map_.at(name)();
}

/**
 */
void SectionContentTable::clicked(unsigned int row)
{
    const auto& annotation = annotations_.at(row);

    //generate on-demand viewable?
    if (annotation.on_demand)
    {
        //only show on demand figure if result is not locked
        if (!taskResult()->isLocked())
        {
            SectionContentViewable viewable;
            bool ok = taskResult()->loadOnDemandViewable(*this, viewable, annotation.index, row);

            if (ok)
            {
                auto content = viewable.viewable_func();
                report_->setCurrentViewable(*content);
            }
            else
            {
                report_->unsetCurrentViewable();
                logerr << "SectionContentTable: clicked: on-demand viewable could not be retrieved";
            }
        }

        return;
    }

    //obtain figure from annotation
    bool has_valid_link = false;
    SectionContentFigure* figure = nullptr;

    if (annotation.figure_id.has_value())
    {
        loginf << "SectionContentTable: clicked: index has associated viewable via id " << annotation.figure_id.value();
        has_valid_link = true;

        //figure from content in parent section
        auto c = parentSection()->retrieveContent(annotation.figure_id.value(), true);
        figure = dynamic_cast<SectionContentFigure*>(c.get());
    }
    else if (!annotation.section_link.empty() && !annotation.section_figure.empty())
    {
        loginf << "SectionContentTable: clicked: index has associated viewable via" 
               << " section '" << annotation.section_link << "'"
               << " figure '" << annotation.section_figure << "'";
        has_valid_link = true;

        //figure from section link + figure name
        auto& section = report_->getSection(annotation.section_link);

        if (section.hasFigure(annotation.section_figure))
            figure = &section.getFigure(annotation.section_figure);
    }

    //valid figure link found?
    if (has_valid_link)
    {
        if (figure)
        {
            // if (result_ptrs_.at(row_index)->viewableDataReady())
            // {
            //     //view data ready, just get it
            //     viewable = result_ptrs_.at(row_index)->viewableData(*this, annotations_.at(row_index));
            // }
            // else
            // {
            //     //recompute async and show wait dialog, this may take a while...
            //     auto func = [ & ] (const AsyncTaskState& state, AsyncTaskProgressWrapper& progress)
            //     {
            //         viewable = result_ptrs_.at(row_index)->viewableData(*this, annotations_.at(row_index));
            //         return Result::succeeded();
            //     };

            //     AsyncFuncTask task(func, "Updating Contents", "Updating contents...", false);
            //     task.runAsyncDialog();
            // }

            //show viewable (will now recompute internally if needed)
            //              (might get cancelled if the figure is locked)
            figure->view();
        }
        else
        {
            logerr << "SectionContentTable: clicked: figure could not be retrieved";
        }
    }
}

/**
 */
void SectionContentTable::doubleClicked(unsigned int row)
{
    const auto& annotation = annotations_.at(row);

    //link to other section stored in row?
    if (!annotation.section_link.empty())
    {
        loginf << "SectionContentTable: doubleClicked: index has associated reference '"
               << annotation.section_link << "'";

        //jump to row link
        report_->setCurrentSection(annotation.section_link);
    }
    else
    {
        loginf << "SectionContentTable: doubleClicked: index has no associated reference";
    }
}

/**
 */
void SectionContentTable::customContextMenu(unsigned int row, const QPoint& pos)
{
    QMenu menu;
    if (!taskResult()->customContextMenu(menu, this, row))
        return;

    menu.exec(pos);
}

/**
 */
void SectionContentTable::addActionsToMenu(QMenu* menu)
{
    //add general actions
    QAction* unused_action = menu->addAction("Toggle Show Unused");
    QObject::connect (unused_action, &QAction::triggered, [ this ] { this->toggleShowUnused(); });

    QAction* copy_action = menu->addAction("Copy Content");
    QObject::connect (copy_action, &QAction::triggered, [ this ] { this->copyContent(); });

    //add custom callbacks stored in map
    if (callback_map_.size() > 0)
    {
        menu->addSeparator();

        for (auto& cb_it : callback_map_)
        {
            QAction* action = menu->addAction(cb_it.first.c_str());
            QObject::connect(action, &QAction::triggered, [ = ] () { this->executeCallback(cb_it.first); });
        }
    }

    //add custom entries provided by task result
    taskResult()->customMenu(*menu, this);
}

/**
 */
void SectionContentTable::toggleShowUnused()
{
    showUnused(!show_unused_);
}

/**
 */
void SectionContentTable::copyContent()
{
    loginf << "SectionContentTable: copyContent";

    std::stringstream ss;

    unsigned int num_cols = headings_.size();

    // headings
    for (unsigned int cnt=0; cnt < num_cols; ++cnt)
    {
        if (cnt == 0)
            ss << headings_.at(cnt);
        else
            ss <<  ";" << headings_.at(cnt);
    }
    ss << "\n";

    auto proxy_model = getOrCreateTableWidget()->proxyModel();

    unsigned int num_rows = proxy_model->rowCount();

    std::vector<std::string> row_strings;

    for (unsigned int row=0; row < num_rows; ++row)
    {
        row_strings = sortedRowStrings(row, false);
        assert (row_strings.size() == num_cols);

        for (unsigned int cnt=0; cnt < num_cols; ++cnt)
        {
            if (cnt == 0)
                ss << row_strings.at(cnt);
            else
                ss <<  ";" << row_strings.at(cnt);
        }
        ss << "\n";
    }

    QApplication::clipboard()->setText(ss.str().c_str());
}

/**
 */
Utils::StringTable SectionContentTable::toStringTable() const
{
    return Utils::StringTable(getOrCreateTableWidget()->itemModel());
}

/**
 */
nlohmann::json SectionContentTable::toJSONTable(bool rowwise,
                                                const std::vector<int>& cols) const
{
    return toStringTable().toJSON(rowwise, cols);
}

/**
 */
void SectionContentTable::toJSON_impl(nlohmann::json& j) const
{
    //call base
    SectionContent::toJSON_impl(j);

    j[ FieldHeadings      ] = headings_;
    j[ FieldSortable      ] = sortable_;
    j[ FieldSortColumn    ] = sort_column_;
    j[ FieldSortOrder     ] = sort_order_ == Qt::AscendingOrder ? "ascending" : "descending";
    j[ FieldRows          ] = std::vector<nlohmann::json>();
    j[ FieldAnnotations   ] = nlohmann::json::array();
    j[ FieldColumnStyles  ] = column_styles_;
    j[ FieldCellStyles    ] = cell_styles_;
    j[ FieldShowTooltips  ] = show_tooltips_;

    if (max_row_count_.has_value())
        j[ FieldMaxRowCount ] = max_row_count_.value();

    //write content only if not on demand
    if (!isOnDemand())
    {
        //write rows
        j[ FieldRows ] = rows_;

        //write annotations
        nlohmann::json j_annos = nlohmann::json::array();

        for (const auto& a : annotations_)
        {
            nlohmann::json j_anno;

            if (a.figure_id.has_value())
                j_anno[ FieldAnnoFigureID ] = a.figure_id.value();

            j_anno[ FieldAnnoSectionLink   ] = a.section_link;
            j_anno[ FieldAnnoSectionFigure ] = a.section_figure;
            j_anno[ FieldAnnoOnDemand      ] = a.on_demand;
            j_anno[ FieldAnnoStyle         ] = a.style;

            j_annos.push_back(j_anno);
        }

        j[ FieldAnnotations ] = j_annos;
    }
}

/**
 */
bool SectionContentTable::fromJSON_impl(const nlohmann::json& j)
{
    //call base
    if (!SectionContent::fromJSON_impl(j))
        return false;
    
    if (!j.is_object()                 ||
        !j.contains(FieldHeadings)     ||
        !j.contains(FieldSortable)     ||
        !j.contains(FieldSortColumn)   ||
        !j.contains(FieldSortOrder)    ||
        !j.contains(FieldRows)         ||
        !j.contains(FieldAnnotations)  ||
        !j.contains(FieldColumnStyles) ||
        !j.contains(FieldCellStyles)   ||
        !j.contains(FieldShowTooltips))
    {
        logerr << "SectionContentTable: fromJSON: Error: Section content table does not obtain needed fields";
        return false;
    }

    headings_      = j[ FieldHeadings     ].get<std::vector<std::string>>();
    column_styles_ = j[ FieldColumnStyles ].get<std::vector<unsigned int>>();
    sortable_      = j[ FieldSortable     ];
    sort_column_   = j[ FieldSortColumn   ];
    show_tooltips_ = j[ FieldShowTooltips ];

    std::string sort_order = j[ FieldSortOrder ];
    sort_order_ = sort_order == "ascending" ? Qt::AscendingOrder : Qt::DescendingOrder;

    if (j.contains(FieldMaxRowCount))
    {
        int v = j[ FieldMaxRowCount ];
        max_row_count_ = v;
    }

    num_columns_ = headings_.size();

    rows_ = j[ FieldRows ].get<std::vector<nlohmann::json>>();

    auto& j_annos = j[ FieldAnnotations ];
    if (!j_annos.is_array() || j_annos.size() != rows_.size())
    {
        logerr << "SectionContentTable: fromJSON: Error: Annotation array invalid";
        return false;
    }

    for (const auto& j_anno : j_annos)
    {
        if (!j_anno.contains(FieldAnnoSectionLink)   ||
            !j_anno.contains(FieldAnnoSectionFigure) ||
            !j_anno.contains(FieldAnnoOnDemand)      ||
            !j_anno.contains(FieldAnnoStyle))
        {
            logerr << "SectionContentTable: fromJSON: Error: Could not read annotation";
            return false;
        }

        RowAnnotation anno;
        anno.section_link   = j_anno[ FieldAnnoSectionLink   ];
        anno.section_figure = j_anno[ FieldAnnoSectionFigure ];
        anno.on_demand      = j_anno[ FieldAnnoOnDemand      ];
        anno.style          = j_anno[ FieldAnnoStyle         ];

        if (j_anno.contains(FieldAnnoFigureID))
        {
            unsigned int id = j_anno[ FieldAnnoFigureID ];
            anno.figure_id = id;
        }

        annotations_.push_back(anno);
    }

    cell_styles_ = j[ FieldCellStyles ].get<CellStyles>();

    assert(rows_.size() == annotations_.size());
    assert(num_columns_ == column_styles_.size());

    return true;
}

/**
 */
Result SectionContentTable::toJSONDocument_impl(nlohmann::json& j,
                                                const std::string* resource_dir) const
{
    //call base
    auto r = SectionContent::toJSONDocument_impl(j, resource_dir);
    if (!r.ok())
        return r;

    bool write_to_file = (ReportExporter::TableMaxRows    >= 0 && numRows()    > (size_t)ReportExporter::TableMaxRows   ) ||
                         (ReportExporter::TableMaxColumns >= 0 && numColumns() > (size_t)ReportExporter::TableMaxColumns);

    if (resource_dir && write_to_file)
    {
        auto res = prepareResource(*resource_dir, ResourceDir::Tables);
        if (!res.ok())
            return res;

        nlohmann::json j_ext;
        j_ext[ FieldDocColumns ] = headings_;
        j_ext[ FieldDocData    ] = rows_;

        std::ofstream of(res.result().path);
        if (!of.is_open())
            return Result::failed("Could not store resource for content '" + name() + "'");

        of << j_ext.dump(4);
        if (!of)
            return Result::failed("Could not store resource for content '" + name() + "'");

        of.close();

        j[ FieldDocPath ] = res.result().link;
    }
    else
    {
        j[ FieldDocColumns ] = headings_;
        j[ FieldDocData    ] = rows_;
    }

    return Result::succeeded();
}

/***************************************************************************************************
 * SectionContentTableModel
 ***************************************************************************************************/

/**
 */
SectionContentTableModel::SectionContentTableModel(SectionContentTable* content_table, QObject* parent)
:   QAbstractItemModel(parent)
,   content_table_    (content_table)
{
    assert(content_table_);
}

/**
 */
QVariant SectionContentTableModel::data(const QModelIndex& index, int role) const
{
    return content_table_->data(index, role);
}

/**
 */
QVariant SectionContentTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        assert (section < (int)content_table_->numColumns());
        return content_table_->headings().at(section).c_str();
    }

    return QVariant();
}

/**
 */
QModelIndex SectionContentTableModel::index(int row, int column, const QModelIndex& parent) const
{
    return createIndex(row, column);
}

/**
 */
int SectionContentTableModel::rowCount(const QModelIndex& parent) const
{
    return (int)content_table_->numRows();
}

/**
 */
int SectionContentTableModel::columnCount(const QModelIndex& parent) const
{
    return (int)content_table_->numColumns();
}

/**
 */
QModelIndex SectionContentTableModel::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

/**
 */
Qt::ItemFlags SectionContentTableModel::flags(const QModelIndex &index) const
{
    auto f = QAbstractItemModel::flags(index);
    f |= content_table_->flags(index);

    return f;
}

/**
 */
bool SectionContentTableModel::canFetchMore(const QModelIndex &parent) const
{
    return content_table_->isOnDemand() && !content_table_->isComplete();
}

/**
 */
void SectionContentTableModel::fetchMore(const QModelIndex &parent)
{
    content_table_->loadOnDemandIfNeeded();
}

/**
 */
void SectionContentTableModel::executeAndReset(const std::function<void()>& func)
{
    if (!func)
        return;

    beginResetModel();

    func();

    endResetModel();
}

/***************************************************************************************************
 * SectionContentTableWidget
 ***************************************************************************************************/

const int SectionContentTableWidget::DoubleClickCheckIntervalMSecs = 300;

/**
 */
SectionContentTableWidget::SectionContentTableWidget(SectionContentTable* content_table, 
                                                     bool show_unused,
                                                     int sort_column,
                                                     Qt::SortOrder sort_order,
                                                     QWidget* parent)
:   QWidget       (parent       )
,   content_table_(content_table)
{
    assert(content_table_);

    QVBoxLayout* main_layout = new QVBoxLayout();
    setLayout(main_layout);

    QHBoxLayout* upper_layout = new QHBoxLayout();

    upper_layout->addWidget(new QLabel(("Table: " + content_table_->name()).c_str()));
    upper_layout->addStretch();

    options_menu_ = new QMenu(options_button_);
    connect(options_menu_, &QMenu::aboutToShow, this, &SectionContentTableWidget::updateOptionsMenu);

    options_button_ = new QPushButton("Options");
    options_button_->setMenu(options_menu_);

    upper_layout->addWidget(options_button_);

    main_layout->addLayout(upper_layout);

    table_view_ = new QTableView;

    model_ = new SectionContentTableModel(content_table, this);

    proxy_model_ = new TableQSortFilterProxyModel(this);
    proxy_model_->showUnused(show_unused);
    proxy_model_->setSourceModel(model_);

    table_view_->setModel(proxy_model_);

    if (sort_column >= 0)
    {
        table_view_->setSortingEnabled(true);
        table_view_->sortByColumn(sort_column, sort_order);
    }

    table_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_view_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_view_->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    table_view_->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table_view_->setContextMenuPolicy(Qt::CustomContextMenu);
    table_view_->setWordWrap(true);

    table_view_->reset();

    connect(table_view_, &QTableView::customContextMenuRequested,
            this, &SectionContentTableWidget::customContextMenu);
    connect(table_view_->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &SectionContentTableWidget::clicked);
    connect(table_view_, &QTableView::pressed,
            this, &SectionContentTableWidget::clicked);
    connect(table_view_, &QTableView::doubleClicked,
            this, &SectionContentTableWidget::doubleClicked);
        
    //    if (num_columns_ > 5)
    //        table_view_->horizontalHeader()->setMaximumSectionSize(150);
    
    table_view_->resizeColumnsToContents();
    table_view_->resizeRowsToContents();

    main_layout->addWidget(table_view_);

    click_action_timer_.setSingleShot(true);
    click_action_timer_.setInterval(DoubleClickCheckIntervalMSecs);

    QObject::connect(&click_action_timer_, &QTimer::timeout, this, &SectionContentTableWidget::performClickAction);
}

/**
 */
SectionContentTableWidget::~SectionContentTableWidget()
{
}

/**
 */
void SectionContentTableWidget::showUnused(bool show)
{
    auto func = [ this, show ] ()
    {
        this->proxy_model_->showUnused(show);
    };

    model_->executeAndReset(func);
}

/**
 */
void SectionContentTableWidget::resizeColumns()
{
    table_view_->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

/**
 */
SectionContentTableModel* SectionContentTableWidget::itemModel()
{
    return model_;
}

/**
 */
const SectionContentTableModel* SectionContentTableWidget::itemModel() const
{
    return model_;
}

/**
 */
TableQSortFilterProxyModel* SectionContentTableWidget::proxyModel()
{
    return proxy_model_;
}

/**
 */
const TableQSortFilterProxyModel* SectionContentTableWidget::proxyModel() const
{
    return proxy_model_;
}

/**
 */
QTableView* SectionContentTableWidget::tableView()
{
    return table_view_;
}

/**
 */
const QTableView* SectionContentTableWidget::tableView() const
{
    return table_view_;
}

/**
 * Returns the original data row from the given proxy model's row.
 */
int SectionContentTableWidget::fromProxy(int proxy_row) const
{
    assert (proxy_row < proxy_model_->rowCount());
    assert (proxy_row < (int)content_table_->numRows());

    QModelIndex index = proxy_model_->index(proxy_row, 0);
    assert (index.isValid());

    auto const source_index = proxy_model_->mapToSource(index);
    assert (source_index.isValid());

    unsigned int row_index = source_index.row();

    return row_index;
}

/**
 */
void SectionContentTableWidget::clicked(const QModelIndex& index)
{
    loginf << "SectionContentTableWidget: clicked";

    if (!index.isValid())
    {
        loginf << "SectionContentTableWidget: clicked: invalid index";
        return;
    }

    if (QApplication::mouseButtons() & Qt::RightButton)
    {
        loginf << "SectionContentTableWidget: clicked: RMB click ignored";
        return;
    }

    auto const source_index = proxy_model_->mapToSource(index);
    assert (source_index.isValid());

    assert (source_index.row() >= 0);
    assert (source_index.row() < (int)content_table_->numRows());

    last_clicked_row_index_ = source_index.row();

    //fire timer to perform delayed click action
    click_action_timer_.start();
}

/**
 */
void SectionContentTableWidget::performClickAction()
{
    loginf << "SectionContentTableWidget: performClickAction";

    //double click did not interrupt click action => perform
    if (!last_clicked_row_index_.has_value())
        return;

    unsigned int row_index = last_clicked_row_index_.value();
    last_clicked_row_index_.reset();

    //pass row to table
    content_table_->clicked(row_index);
}

/**
 */
void SectionContentTableWidget::doubleClicked(const QModelIndex& index)
{
    loginf << "SectionContentTableWidget: doubleClicked";

    //double click detected => interrupt any previously triggered click action
    click_action_timer_.stop();

    if (!index.isValid())
    {
        loginf << "SectionContentTableWidget: doubleClicked: invalid index";
        return;
    }

    auto const source_index = proxy_model_->mapToSource(index);
    assert (source_index.isValid());

    assert (source_index.row() >= 0);
    assert (source_index.row() < (int)content_table_->numRows());

    loginf << "SectionContentTableWidget: doubleClicked: row " << source_index.row();

    unsigned int row_index = source_index.row();

    //pass row to table
    content_table_->doubleClicked(row_index);
}

/**
 */
void SectionContentTableWidget::customContextMenu(const QPoint& p)
{
    logdbg << "SectionContentTableWidget: customContextMenu";

    QModelIndex index = table_view_->indexAt(p);
    if (!index.isValid())
        return;

    auto const source_index = proxy_model_->mapToSource(index);
    assert (source_index.isValid());

    loginf << "SectionContentTableWidget: customContextMenu: row " << index.row() << " src " << source_index.row();

    assert (source_index.row() >= 0);
    assert (source_index.row() < (int)content_table_->numRows());

    unsigned int row_index = source_index.row();

    auto pos = table_view_->viewport()->mapToGlobal(p);

    //pass row to table
    content_table_->customContextMenu(row_index, pos);
}

/**
 */
std::vector<std::string> SectionContentTableWidget::sortedRowStrings(unsigned int row, bool latex) const
{
    logdbg << "SectionContentTableWidget: sortedRowStrings: row " << row << " rows " << proxy_model_->rowCount()
           << " data rows " << content_table_->numRows();
    
    assert ((int)row < proxy_model_->rowCount());
    assert (row < content_table_->numRows());

    std::vector<std::string> result;

    size_t nc = content_table_->numColumns();

    for (size_t col=0; col < nc; ++col)
    {
        QModelIndex index = proxy_model_->index(row, col);
        assert (index.isValid());

        // get string can convert to latex
        if (latex)
            result.push_back(Utils::String::latexString(proxy_model_->data(index).toString().toStdString()));
        else
            result.push_back(proxy_model_->data(index).toString().toStdString());
    }

    return result;
}

/**
 */
void SectionContentTableWidget::updateOptionsMenu()
{
    if (options_menu_ && content_table_)
    {
        options_menu_->clear();
        content_table_->addActionsToMenu(options_menu_);
    }
}

}
