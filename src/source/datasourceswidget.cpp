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

#include "datasourceswidget.h"
#include "datasourcemanager.h"

#include "compass.h"
#include "dbcontent/dbcontentmanager.h"

#include "global.h"
#include "stringconv.h"
#include "number.h"
#include "files.h"
#include "timeconv.h"

#include <QLabel>
#include <QCheckBox>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>

/**************************************************************************************************
 * DataSourcesWidgetItem
 **************************************************************************************************/

/**
 */
DataSourcesWidgetItem::DataSourcesWidgetItem(DataSourcesWidget* widget,
                                             DataSourcesWidgetItem* parent,
                                             Type type)
:   widget_(widget)
,   parent_(parent) 
,   type_  (type  )
{
    assert(widget_);
}

/**
 */
void DataSourcesWidgetItem::setItemWidget(int column, QWidget* w)
{
    assert(widget_);
    assert(widget_->tree_widget_);

    widget_->tree_widget_->setItemWidget(this, column, w);
}

/**************************************************************************************************
 * DataSourceTypeItem
 **************************************************************************************************/

/**
 */
DataSourceTypeItem::DataSourceTypeItem(DataSourcesWidget* widget,
                                       DataSourcesWidgetItem* parent,
                                       const std::string& ds_type)
:   DataSourcesWidgetItem(widget, parent, Type::DataSourceType)
,   ds_type_(ds_type) 
{
    auto font_bold = font(0);
    font_bold.setBold(true);

    setCheckState(0, widget_->getUseDSType(ds_type_) ? Qt::Checked : Qt::Unchecked);
    setText(0, QString::fromStdString(ds_type_));
    setFont(0, font_bold);

    bool has_sources = false;

    auto& ds_man = widget_->dsManager();
    for (const auto& ds_it : ds_man.dbDataSources())
        if (ds_it->dsType() == ds_type_)
            has_sources = true;

    //deactivate items without content
    if (!has_sources)
    {
        setCheckState(0, Qt::Unchecked);
        setFlags(flags() & ~Qt::ItemFlag::ItemIsEnabled);
    }
}

/**
 */
void DataSourceTypeItem::updateContent()
{
    setCheckState(0, widget_->getUseDSType(ds_type_) ? Qt::Checked : Qt::Unchecked);
}

/**************************************************************************************************
 * DataSourceItem
 **************************************************************************************************/

/**
 */
DataSourceItem::DataSourceItem(DataSourcesWidget* widget,
                               DataSourcesWidgetItem* parent,
                               unsigned int ds_id)
:   DataSourcesWidgetItem(widget, parent, Type::DataSource)
,   ds_id_(ds_id) 
{
    auto& ds_man = widget_->dsManager();
    assert(ds_man.hasDBDataSource(ds_id));

    const auto& data_source = ds_man.dbDataSource(ds_id);
    ds_ = &data_source;

    std::string ds_name = data_source.name();

    setCheckState(0, widget_->getUseDS(ds_id_) ? Qt::Checked : Qt::Unchecked);
    setText(0, QString::fromStdString(ds_name));
}

/**
 */
void DataSourceItem::init_impl()
{
    auto w = createLinesWidget();

    setItemWidget(1, w);
}

/**
 */
void DataSourceItem::updateContent()
{
    setCheckState(0, widget_->getUseDS(ds_id_) ? Qt::Checked : Qt::Unchecked);

    for (auto lb : line_buttons_)
        lb->updateContent();
}

/**
 */
QWidget* DataSourceItem::createLinesWidget()
{
    QWidget* widget = new QWidget();
    widget->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* button_layout = new QHBoxLayout();
    button_layout->setContentsMargins(0, 1, 0, 1);

    std::string line_str;

    for (unsigned int cnt=0; cnt < 4; ++cnt)
    {
        auto w = new DataSourceLineButton(widget_, ds_id_, cnt, DataSourcesWidget::LineButtonSize);
        button_layout->addWidget(w);

        line_buttons_.push_back(w);
    }

    widget->setLayout(button_layout);

    return widget;
}

/**************************************************************************************************
 * DataSourceCountItem
 **************************************************************************************************/

/**
 */
DataSourceCountItem::DataSourceCountItem(DataSourcesWidget* widget,
                                         DataSourcesWidgetItem* parent,
                                         unsigned int ds_id,
                                         const std::string& dbc_name)
:   DataSourcesWidgetItem(widget, parent, Type::DataSourceCount)
,   ds_id_   (ds_id   )
,   dbc_name_(dbc_name)
{
    auto& ds_man = widget_->dsManager();
    assert(ds_man.hasDBDataSource(ds_id));

    const auto& data_source = ds_man.dbDataSource(ds_id);
    ds_ = &data_source;

    setText(0, QString::fromStdString(dbc_name));
    setText(2, QString::number(0));
    setText(3, QString::number(0));
}

/**
*/
void DataSourceCountItem::updateContent()
{
    auto num_inserted = ds_->numInsertedSummedLinesMap();
    auto it = num_inserted.find(dbc_name_);
    assert(it != num_inserted.end());

    setText(2, QString::number(ds_->numLoaded(dbc_name_)));
    setText(3, QString::number(it->second));
}

/**************************************************************************************************
 * DataSourceLineButton
 **************************************************************************************************/

/**
 */
DataSourceLineButton::DataSourceLineButton(DataSourcesWidget* widget, 
                                           unsigned int ds_id, 
                                           unsigned int line_id,
                                           unsigned int button_size_px)
:   widget_ (widget )
,   ds_id_  (ds_id  )
,   line_id_(line_id)
{
    assert(widget_);

    auto& ds_man = widget_->dsManager();
    assert(ds_man.hasDBDataSource(ds_id_));

    auto& ds_src = ds_man.dbDataSource(ds_id_);
    ds_ = &ds_src;

    line_str_ = "L" + std::to_string(line_id_ + 1);

    setText(QString::fromStdString(line_str_));

    setFixedSize(button_size_px, button_size_px);
    setCheckable(true);

    bool dark_mode = COMPASS::instance().darkMode();

    if (dark_mode)
        setStyleSheet(" QPushButton:pressed { border: 3px outset white; } " \
                      " QPushButton:checked { border: 3px outset white; }");
    else
        setStyleSheet(" QPushButton:pressed { border: 3px outset; } " \
                      " QPushButton:checked { border: 3px outset; }");

    setProperty("Line ID", line_id_);

    connect(this, &QPushButton::toggled, [ = ] (bool ok) { widget_->setUseDSLine(ds_id_, line_id_, ok); });

    QSizePolicy sp_retain = widget->sizePolicy();
    sp_retain.setRetainSizeWhenHidden(true);
    setSizePolicy(sp_retain);

    updateContent();
}

/**
 */
void DataSourceLineButton::updateContent()
{
    AppMode app_mode = COMPASS::instance().appMode();

    bool live_mode = app_mode == AppMode::LivePaused || app_mode == AppMode::LiveRunning;
    bool dark_mode = COMPASS::instance().darkMode();

    auto& ds_man = widget_->dsManager();

    if (live_mode)
    {
        std::map<unsigned int, std::map<std::string, std::shared_ptr<DataSourceLineInfo>>> net_lines = ds_man.getNetworkLines();

        bool hidden = !net_lines.count(ds_id_) || !net_lines.at(ds_id_).count(line_str_); // hide if not defined

        setHidden(hidden);

        if (!hidden)
        {
            bool disabled = app_mode == AppMode::LivePaused ? !ds_man.dbDataSource(ds_id_).hasNumInserted(line_id_) : false;
            setDisabled(disabled);

            if (disabled)
            {
                setChecked(false);

                QPalette pal = palette();
                if (dark_mode)
                    pal.setColor(QPalette::Button, QColor(Qt::darkGray));
                else
                    pal.setColor(QPalette::Button, QColor(Qt::lightGray));

                setAutoFillBackground(true);
                setPalette(pal);
                update();
            }
            else
            {
                setChecked(widget_->getUseDSLine(ds_id_, line_id_));

                boost::posix_time::ptime current_time = Utils::Time::currentUTCTime();

                if (ds_->hasLiveData(line_id_, current_time))
                {
                    QPalette pal = palette();

                    if (dark_mode)
                        pal.setColor(QPalette::Button, QColor(Qt::darkGreen));
                    else
                        pal.setColor(QPalette::Button, QColor(Qt::green));

                    setAutoFillBackground(true);
                    setPalette(pal);
                    update();
                }
                else
                {
                    QPalette pal = palette();

                    if (dark_mode)
                        pal.setColor(QPalette::Button, QColor(Qt::darkGray));
                    else
                        pal.setColor(QPalette::Button, QColor(Qt::lightGray));

                    setAutoFillBackground(true);
                    setPalette(pal);
                    update();
                }
            }
        }
    }
    else
    {
        std::map<unsigned int, unsigned int> inserted_lines = ds_->numInsertedLinesMap();

        setChecked(widget_->getUseDSLine(ds_id_, line_id_));
        setHidden(!inserted_lines.count(line_id_)); // hide if no data
    }
}

/**************************************************************************************************
 * DataSourcesLoadTreeWidget
 **************************************************************************************************/

 const int DataSourcesWidget::LineButtonSize = 25;

/**
 */
DataSourcesWidget::DataSourcesWidget(DataSourceManager& ds_man)
:   ds_man_      (ds_man)
,   shows_counts_(false )
{
    createUI();
}

/**
 */
DataSourcesWidget::~DataSourcesWidget() = default;

/**
 */
void DataSourcesWidget::createUI()
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* main_layout = new QVBoxLayout();
    setLayout(main_layout);

    // buttons
    QHBoxLayout* button_layout = new QHBoxLayout();
    button_layout->addStretch();

    QPushButton* edit_button = new QPushButton();
    edit_button->setIcon(QIcon(Utils::Files::getIconFilepath("edit.png").c_str()));
    edit_button->setFixedSize(UI_ICON_SIZE);
    edit_button->setFlat(UI_ICON_BUTTON_FLAT);
    edit_button->setToolTip(tr("Data Source Options"));

    connect(edit_button, &QPushButton::clicked, this, &DataSourcesWidget::editClicked);
    button_layout->addWidget(edit_button);

    main_layout->addLayout(button_layout);

    // tree widget
    tree_widget_ = new QTreeWidget;
    tree_widget_->setColumnCount(4);

    QStringList header_labels;
    header_labels << "Name";
    header_labels << "Lines";
    header_labels << "Loaded";
    header_labels << "Count";

    tree_widget_->setHeaderLabels(header_labels);
    tree_widget_->header()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);

    connect(tree_widget_, &QTreeWidget::itemChanged, this, &DataSourcesWidget::itemChanged);

    main_layout->addWidget(tree_widget_);

    QHBoxLayout* assoc_layout = new QHBoxLayout();

    // time
    QLabel* ts_label = new QLabel("Timestamps");
    ts_label->setFont(font_bold);
    assoc_layout->addWidget(ts_label);

    assoc_layout->addWidget(new QLabel("Min"));

    ts_min_label_ = new QLabel("None");
    assoc_layout->addWidget(ts_min_label_);

    assoc_layout->addWidget(new QLabel("Max"));

    ts_max_label_ = new QLabel("None");
    assoc_layout->addWidget(ts_max_label_);

    assoc_layout->addStretch();

    // assoc
    QLabel* assoc_label = new QLabel("Associations");
    assoc_label->setFont(font_bold);
    assoc_layout->addWidget(assoc_label);

    associations_label_ = new QLabel("None");
    assoc_layout->addWidget(associations_label_);

    assoc_layout->addStretch();
    main_layout->addLayout(assoc_layout);

    // update
    updateContent();

    // create menu
    createMenu();
}

/**
 */
void DataSourcesWidget::createMenu()
{
    QAction* sel_dstyp_action = edit_menu_.addAction("Select All DSTypes");
    connect(sel_dstyp_action, &QAction::triggered, this, &DataSourcesWidget::selectAllDSTypes);

    QAction* desel_dstyp_action = edit_menu_.addAction("Deselect All DSTypes");
    connect(desel_dstyp_action, &QAction::triggered, this, &DataSourcesWidget::deselectAllDSTypes);

    edit_menu_.addSeparator();

    QMenu* select_ds = edit_menu_.addMenu("Select Data Sources");

    QAction* sel_ds_action = select_ds->addAction("All");
    connect(sel_ds_action, &QAction::triggered, this, &DataSourcesWidget::selectAllDataSources);

    for (const auto& ds_type_it : ds_man_.data_source_types_)
    {
        QAction* action = select_ds->addAction(("From "+ds_type_it).c_str());
        action->setProperty("ds_type", ds_type_it.c_str());
        connect(action, &QAction::triggered, this, &DataSourcesWidget::selectDSTypeSpecificDataSources);
    }

    QMenu* deselect_ds = edit_menu_.addMenu("Deselect Data Sources");

    QAction* desel_ds_action = deselect_ds->addAction("All");
    connect(desel_ds_action, &QAction::triggered, this, &DataSourcesWidget::deselectAllDataSources);

    for (const auto& ds_type_it : ds_man_.data_source_types_)
    {
        QAction* action = deselect_ds->addAction(("From "+ds_type_it).c_str());
        action->setProperty("ds_type", ds_type_it.c_str());
        connect(action, &QAction::triggered, this, &DataSourcesWidget::deselectDSTypeSpecificDataSources);
    }

    edit_menu_.addSeparator();

    QMenu* set_lines = edit_menu_.addMenu("Set Line");

    QAction* desel_line_action = set_lines->addAction("Deselect All");
    connect(desel_line_action, &QAction::triggered, this, &DataSourcesWidget::deselectAllLines);

    for (unsigned int cnt=0; cnt < 4; ++cnt)
    {
        QAction* desel_line_action = set_lines->addAction(("Select " + Utils::String::lineStrFrom(cnt)).c_str());
        desel_line_action->setProperty("line_id", cnt);
        connect(desel_line_action, &QAction::triggered, this, &DataSourcesWidget::selectSpecificLines);
    }

    edit_menu_.addSeparator();

    QAction* show_cnt_action = edit_menu_.addAction("Toggle Show Counts");
    connect(show_cnt_action, &QAction::triggered, this, &DataSourcesWidget::toogleShowCounts);
}

/**
 */
void DataSourcesWidget::clear()
{
    tree_widget_->clear();
}

/**
 */
void DataSourcesWidget::createContent()
{
    logdbg << "DataSourcesWidget: createContent";
    
    clear();

    unsigned int dstype_cnt = 0;
    for (auto& ds_type_name : DataSourceManager::data_source_types_)
    {
        logdbg << "DataSourcesWidget: createContent: typ " << ds_type_name << " cnt " << dstype_cnt++;

        createDataSourceType(ds_type_name);
    }

    shows_counts_ = getShowCounts();

    tree_widget_->expandAll();

    updateAdditionalInfo();
}

/**
 */
void DataSourcesWidget::createDataSourceType(const std::string& ds_type_name)
{
    auto dstype_item = new DataSourceTypeItem(this, nullptr, ds_type_name);

    tree_widget_->addTopLevelItem(dstype_item);
    dstype_item->init();

    //add data sources
    for (const auto& ds_it : ds_man_.dbDataSources())
        if (ds_it->dsType() == ds_type_name)
            createDataSource(dstype_item, *ds_it);
}

/**
 */
void DataSourcesWidget::createDataSource(DataSourcesWidgetItem* parent_item, 
                                         const dbContent::DBDataSource& data_source)
{
    unsigned int ds_id   = Utils::Number::dsIdFrom(data_source.sac(), data_source.sic());
    std::string  ds_name = data_source.name();

    logdbg << "DataSourcesWidget: createDataSource: create '" << data_source.dsType() << "' '" << ds_name << "'";

    auto ds_item = new DataSourceItem(this, parent_item, ds_id);

    parent_item->addChild(ds_item);
    ds_item->init();

    bool show_counts = getShowCounts();

    //add count items?
    if (show_counts)
    {
        for (auto& cnt_it : data_source.numInsertedSummedLinesMap())
            createDBContent(ds_item, data_source, cnt_it.first);
    }
}

/**
 */
void DataSourcesWidget::createDBContent(DataSourcesWidgetItem* parent_item,
                                        const dbContent::DBDataSource& data_source,
                                        const std::string& dbc_name)
{
    auto ds_cnt_item = new DataSourceCountItem(this, parent_item, data_source.id(), dbc_name);
    
    parent_item->addChild(ds_cnt_item);
    ds_cnt_item->init();
}

/**
 */
void DataSourcesWidget::updateContent(bool recreate_required)
{
    logdbg << "DataSourcesWidget: updateContent: recreate_required " << recreate_required
           << " num data sources " << ds_man_.dbDataSources().size();

    createContent();
}

/**
 */
void DataSourcesWidget::itemChanged(QTreeWidgetItem *item, int column)
{
    auto w_item = dynamic_cast<DataSourcesWidgetItem*>(item);
    if (!w_item)
        return;

    //react on item changes
    if (w_item->type() == DataSourcesWidgetItem::Type::DataSourceType)
    {   
        auto ds_type_item = dynamic_cast<DataSourceTypeItem*>(w_item);
        assert(ds_type_item);

        if (column == 0)
        {
            bool load = ds_type_item->checkState(0) == Qt::Checked;
            setUseDSType(ds_type_item->dsType(), load);

            loginf << "DataSourcesWidget: itemChanged: ds_type " << ds_type_item->dsType() << " load " << load;
        }
    }
    else if (w_item->type() == DataSourcesWidgetItem::Type::DataSource)
    {   
        auto ds_item = dynamic_cast<DataSourceItem*>(w_item);
        assert(ds_item);

        if (column == 0)
        {
            bool load = ds_item->checkState(0) == Qt::Checked;
            setUseDS(ds_item->dsID(), load);

            loginf << "DataSourcesWidget: itemChanged: ds_id " << ds_item->dsID() << " load " << load;
        }
    }
    else if (w_item->type() == DataSourcesWidgetItem::Type::DataSourceCount)
    {   
        auto ds_cnt_item = dynamic_cast<DataSourceCountItem*>(w_item);
        assert(ds_cnt_item);

        //nothing to do yet
    }
}

namespace
{
    void updateContentRecursive(QTreeWidgetItem* item)
    {
        auto w_item = dynamic_cast<DataSourcesWidgetItem*>(item);
        if (w_item)
            w_item->updateContent();

        for (int i = 0; i < item->childCount(); ++i)
            updateContentRecursive(item->child(i));
    }
}

/**
 */
void DataSourcesWidget::updateAllContent()
{
    for (int i = 0; i < tree_widget_->topLevelItemCount(); ++i)
        updateContentRecursive(tree_widget_->topLevelItem(i));

    updateAdditionalInfo();
}

/**
 */
void DataSourcesWidget::updateAdditionalInfo()
{
    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    assert (ts_min_label_);
    assert (ts_max_label_);

    if (dbcont_man.hasMinMaxTimestamp())
    {
        ts_min_label_->setText(Utils::Time::toString(std::get<0>(dbcont_man.minMaxTimestamp()), 0).c_str());
        ts_max_label_->setText(Utils::Time::toString(std::get<1>(dbcont_man.minMaxTimestamp()), 0).c_str());
    }
    else
    {
        ts_min_label_->setText("None");
        ts_max_label_->setText("None");
    }

    assert(associations_label_);
    if (dbcont_man.hasAssociations())
    {
        std::string tmp = "From " + dbcont_man.associationsID();
        associations_label_->setText(tmp.c_str());
    }
    else
    {
        associations_label_->setText("None");
    }
}

/**
 */
void DataSourcesWidget::editClicked()
{
    loginf << "DataSourcesWidget: editClicked";

    edit_menu_.exec(QCursor::pos());
}

/**
 */
void DataSourcesWidget::selectAllDSTypes()
{
    loginf << "DataSourcesWidget: selectAllDSTypes";

    for (auto& ds_type_name : DataSourceManager::data_source_types_)
        setUseDSType(ds_type_name, true);

    updateAllContent();
}

/**
 */
void DataSourcesWidget::deselectAllDSTypes()
{
    loginf << "DataSourcesWidget: deselectAllDSTypes";

    for (auto& ds_type_name : DataSourceManager::data_source_types_)
        setUseDSType(ds_type_name, false);

    updateAllContent();
}

/**
 */
void DataSourcesWidget::selectAllDataSources()
{
    loginf << "DataSourcesWidget: selectAllDataSources";

    for (const auto& ds_it : ds_man_.dbDataSources())
        setUseDS(ds_it->id(), true);

    updateAllContent();
}

/**
 */
void DataSourcesWidget::deselectAllDataSources()
{
    loginf << "DataSourcesWidget: deselectAllDataSources";

    for (const auto& ds_it : ds_man_.dbDataSources())
        setUseDS(ds_it->id(), false);

    updateAllContent();
}

/**
 */
void DataSourcesWidget::selectDSTypeSpecificDataSources()
{
    QAction* action = dynamic_cast<QAction*>(sender());
    assert (action);

    std::string ds_type = action->property("ds_type").toString().toStdString();

    loginf << "DataSourcesWidget: selectDSTypeSpecificDataSources: ds_type '" << ds_type << "'";

    for (const auto& ds_it : ds_man_.dbDataSources())
        if (ds_it->dsType() == ds_type)
            setUseDS(ds_it->id(), true);

    updateAllContent();
}

/**
 */
void DataSourcesWidget::deselectDSTypeSpecificDataSources()
{
    QAction* action = dynamic_cast<QAction*>(sender());
    assert (action);

    std::string ds_type = action->property("ds_type").toString().toStdString();

    loginf << "DataSourcesWidget: deselectDSTypeSpecificDataSources: ds_type '" << ds_type << "'";

    for (const auto& ds_it : ds_man_.dbDataSources())
        if (ds_it->dsType() == ds_type)
            setUseDS(ds_it->id(), false);

    updateAllContent();
}

/**
 */
void DataSourcesWidget::deselectAllLines()
{
    loginf << "DataSourcesWidget: deselectAllLines";

    for (const auto& ds_it : ds_man_.dbDataSources())
        for (int line = 0; line < 4; ++line)
            setUseDSLine(ds_it->id(), line, false);

    updateAllContent();
}

/**
 */
void DataSourcesWidget::selectSpecificLines()
{
    QAction* action = dynamic_cast<QAction*>(sender());
    assert (action);

    unsigned int line_id = action->property("line_id").toUInt();

    loginf << "DataSourcesWidget: selectSpecificLine: line_id " << line_id;

    for (const auto& ds_it : ds_man_.dbDataSources())
        setUseDSLine(ds_it->id(), line_id, true);

    updateAllContent();
}

/**
 */
void DataSourcesWidget::toogleShowCounts()
{
    loginf << "DataSourcesWidget: toogleShowCounts";

    setShowCounts(!getShowCounts());

    updateContent();
}

/**
 */
void DataSourcesWidget::setUseDSType(const std::string& ds_type_name, bool use)
{
    ds_man_.dsTypeLoadingWanted(ds_type_name, use);
}

/**
 */
bool DataSourcesWidget::getUseDSType(const std::string& ds_type_name) const
{
    return ds_man_.dsTypeLoadingWanted(ds_type_name);
}

/**
 */
void DataSourcesWidget::setUseDS(unsigned int ds_id, bool use)
{
    ds_man_.dbDataSource(ds_id).loadingWanted(use);
}

/**
 */
bool DataSourcesWidget::getUseDS(unsigned int ds_id) const
{
    return ds_man_.dbDataSource(ds_id).loadingWanted();
}

/**
 */
void DataSourcesWidget::setUseDSLine(unsigned int ds_id, unsigned int ds_line, bool use)
{
    ds_man_.dbDataSource(ds_id).lineLoadingWanted(ds_line, use);
}

/**
 */
bool DataSourcesWidget::getUseDSLine(unsigned int ds_id, unsigned int ds_line) const
{
    return ds_man_.dbDataSource(ds_id).lineLoadingWanted(ds_line);
}

/**
 */
void DataSourcesWidget::setShowCounts(bool show) const
{
    ds_man_.config().load_widget_show_counts_ = show;
}

/**
 */
bool DataSourcesWidget::getShowCounts() const
{
    return ds_man_.config().load_widget_show_counts_;
}
