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

#include "datasourcesloadwidget.h"
#include "datasourcemanager.h"

#include "compass.h"
#include "dbcontent/dbcontentmanager.h"

#include "global.h"
#include "stringconv.h"
#include "number.h"
#include "files.h"
#include "timeconv.h"

#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>

/**************************************************************************************************
 * DataSourceLineButton
 **************************************************************************************************/

/**
 */
DataSourceLineButton::DataSourceLineButton(DataSourcesLoadTreeWidget* widget, 
                                           unsigned int ds_id, 
                                           unsigned int line_id,
                                           unsigned int button_size_px)
:   widget_ (widget )
,   ds_id_  (ds_id  )
,   line_id_(line_id)
{
    assert(widget_);

    line_str_ = "L" + std::to_string(line_id_ + 1);

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
    auto& ds_src = ds_man.dbDataSource(ds_id_);

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

                boost::posix_time::ptime current_time = Time::currentUTCTime();

                if (ds_src.hasLiveData(line_id_, current_time))
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
        std::map<unsigned int, unsigned int> inserted_lines = ds_src.numInsertedLinesMap();

        setChecked(widget_->getUseDSLine(ds_id_, line_id_));
        setHidden(!inserted_lines.count(line_id_)); // hide if no data
    }
}

/**************************************************************************************************
 * DataSourcesLoadTreeWidget
 **************************************************************************************************/

/**
 */
DataSourcesLoadTreeWidget::DataSourcesLoadTreeWidget(DataSourceManager& ds_man, 
                                                     bool show_counts)
:   ds_man_     (ds_man     )
,   show_counts_(show_counts)
{
    createUI();
}

/**
 */
DataSourcesLoadTreeWidget::~DataSourcesLoadTreeWidget() = default;

/**
 */
void DataSourcesLoadTreeWidget::createUI()
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* main_layout = new QVBoxLayout();
    setLayout(main_layout);

    // buttons
    QHBoxLayout* button_layout = new QHBoxLayout();
    button_layout->addStretch();

    QPushButton* edit_button = new QPushButton();
    edit_button->setIcon(QIcon(Files::getIconFilepath("edit.png").c_str()));
    edit_button->setFixedSize(UI_ICON_SIZE);
    edit_button->setFlat(UI_ICON_BUTTON_FLAT);
    edit_button->setToolTip(tr("Data Source Options"));

    connect(edit_button, &QPushButton::clicked, this, &DataSourcesLoadTreeWidget::editClickedSlot);
    button_layout->addWidget(edit_button);

    main_layout->addLayout(button_layout);

    // tree widget
    tree_widget_ = new QTreeWidget;
    tree_widget_->setColumnCount(5);

    QStringList header_labels;
    header_labels << "Use";
    header_labels << "Name";
    header_labels << "Lines";
    header_labels << "Loaded";
    header_labels << "Count";

    tree_widget_->setHeaderLabels(header_labels);

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
void DataSourcesLoadTreeWidget::createMenu()
{
    QAction* sel_dstyp_action = edit_menu_.addAction("Select All DSTypes");
    connect(sel_dstyp_action, &QAction::triggered, this, &DataSourcesLoadTreeWidget::selectAllDSTypesSlot);

    QAction* desel_dstyp_action = edit_menu_.addAction("Deselect All DSTypes");
    connect(desel_dstyp_action, &QAction::triggered, this, &DataSourcesLoadTreeWidget::deselectAllDSTypesSlot);

    edit_menu_.addSeparator();

    QMenu* select_ds = edit_menu_.addMenu("Select Data Sources");

    QAction* sel_ds_action = select_ds->addAction("All");
    connect(sel_ds_action, &QAction::triggered, this, &DataSourcesLoadTreeWidget::selectAllDataSourcesSlot);

    for (const auto& ds_type_it : ds_man_.data_source_types_)
    {
        QAction* action = select_ds->addAction(("From "+ds_type_it).c_str());
        action->setProperty("ds_type", ds_type_it.c_str());
        connect(action, &QAction::triggered, this, &DataSourcesLoadTreeWidget::selectDSTypeSpecificDataSourcesSlot);
    }

    QMenu* deselect_ds = edit_menu_.addMenu("Deselect Data Sources");

    QAction* desel_ds_action = deselect_ds->addAction("All");
    connect(desel_ds_action, &QAction::triggered, this, &DataSourcesLoadTreeWidget::deselectAllDataSourcesSlot);

    for (const auto& ds_type_it : ds_man_.data_source_types_)
    {
        QAction* action = deselect_ds->addAction(("From "+ds_type_it).c_str());
        action->setProperty("ds_type", ds_type_it.c_str());
        connect(action, &QAction::triggered, this, &DataSourcesLoadTreeWidget::deselectDSTypeSpecificDataSourcesSlot);
    }

    edit_menu_.addSeparator();

    QMenu* set_lines = edit_menu_.addMenu("Set Line");

    QAction* desel_line_action = set_lines->addAction("Deselect All");
    connect(desel_line_action, &QAction::triggered, this, &DataSourcesLoadTreeWidget::deselectAllLinesSlot);

    for (unsigned int cnt=0; cnt < 4; ++cnt)
    {
        QAction* desel_line_action = set_lines->addAction(("Select "+String::lineStrFrom(cnt)).c_str());
        desel_line_action->setProperty("line_id", cnt);
        connect(desel_line_action, &QAction::triggered, this, &DataSourcesLoadTreeWidget::selectSpecificLineSlot);
    }

    edit_menu_.addSeparator();

    QAction* show_cnt_action = edit_menu_.addAction("Toggle Show Counts");
    connect(show_cnt_action, &QAction::triggered, this, &DataSourcesLoadTreeWidget::toogleShowCountsSlot);
}

/**
 */
void DataSourcesLoadTreeWidget::clear()
{
    ds_type_items_.clear();
    ds_items_.clear();

    item_infos_.clear();

    tree_widget_->clear();
}

/**
 */
QWidget* DataSourcesLoadTreeWidget::createLinesWidget(unsigned int ds_id)
{
    QWidget* widget = new QWidget();
    widget->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* button_layout = new QHBoxLayout();
    button_layout->setContentsMargins(0, 0, 0, 0);

    string line_str;

    unsigned int button_size = 26;
    widget->setMinimumHeight(button_size);

    bool dark_mode = COMPASS::instance().darkMode();

    for (unsigned int cnt=0; cnt < 4; ++cnt)
    {
        auto w = new DataSourceLineButton(this, ds_id, cnt, button_size);
        button_layout->addWidget(w);
    }

    widget->setLayout(button_layout);

    return widget;
}

/**
 */
void DataSourcesLoadTreeWidget::createContent()
{
    logdbg << "DataSourcesLoadTreeWidget: createContent";
    
    clear();

    unsigned int dstype_cnt = 0;
    int item_id = 0;
    for (auto& ds_type_name : DataSourceManager::data_source_types_)
    {
        logdbg << "DataSourcesLoadTreeWidget: createContent: typ " << ds_type_name << " cnt " << dstype_cnt++;

        createDataSourceType(ds_type_name);
    }
}

/**
 */
void DataSourcesLoadTreeWidget::createDataSourceType(const std::string& ds_type_name)
{
    //create item for ds type
    QTreeWidgetItem* dstype_item = new QTreeWidgetItem;

    auto font_bold = dstype_item->font(0);
    font_bold.setBold(true);

    ItemInfo info;
    info.id   = item_ids_++;
    info.name = ds_type_name;
    info.type = ItemType::DataSourceType;

    bool use_item = getUseDSType(ds_type_name);

    dstype_item->setCheckState(0, use_item ? Qt::Checked : Qt::Unchecked);
    dstype_item->setText(1, QString::fromStdString(ds_type_name));
    dstype_item->setFont(1, font_bold);
    dstype_item->setData(0, Qt::UserRole, QVariant(info.id));

    assert (ds_type_items_.count(ds_type_name) == 0);
    
    ds_type_items_[ ds_type_name ] = dstype_item;
    item_infos_   [ info.id      ] = info;

    tree_widget_->addTopLevelItem(dstype_item);

    //add data sources
    bool ds_found = false;
    for (const auto& ds_it : ds_man_.dbDataSources())
    {
        if (ds_it->dsType() != ds_type_name)
            continue;

        ds_found = true;

        createDataSource(dstype_item, *ds_it);
    }

    if (!ds_found)
    {
        //deactivate item without content
        dstype_item->setCheckState(0, Qt::Unchecked);
        dstype_item->setFlags(dstype_item->flags() & ~Qt::ItemFlag::ItemIsEnabled);
    }
}

/**
 */
void DataSourcesLoadTreeWidget::createDataSource(QTreeWidgetItem* parent_item, const dbContent::DBDataSource& data_source)
{
    unsigned int ds_id   = Number::dsIdFrom(data_source.sac(), data_source.sic());
    std::string  ds_name = data_source.name();

    logdbg << "DataSourcesLoadTreeWidget: createDataSource: create '" << data_source.dsType() << "' '" << ds_name << "'";

    //create item for data source
    QTreeWidgetItem* ds_item = new QTreeWidgetItem;

    ItemInfo info;
    info.id   = item_ids_++;
    info.name = ds_name;
    info.type = ItemType::DataSource;

    bool use_item = getUseDS(ds_id);

    ds_item->setCheckState(0, use_item ? Qt::Checked : Qt::Unchecked);
    ds_item->setText(1, QString::fromStdString(ds_name));
    ds_item->setData(0, Qt::UserRole, QVariant(info.id));

    assert (ds_items_.count(ds_name) == 0);

    ds_items_  [ ds_name ] = ds_item;
    item_infos_[ info.id ] = info;

    parent_item->addChild(ds_item);

    auto lines_w = createLinesWidget(ds_id);
    tree_widget_->setItemWidget(ds_item, 2, lines_w);
}

/**
 */
void DataSourcesLoadTreeWidget::createDBContent(QTreeWidgetItem* parent_item)
{

}

/**
 */
void DataSourcesLoadTreeWidget::updateContent(bool recreate_required)
{
    logdbg << "DataSourcesLoadWidget: updateContent: recreate_required " << recreate_required
           << " num data sources " << ds_man_.dbDataSources().size();

    createContent();
}

/**
 */
void DataSourcesLoadTreeWidget::loadDSTypeChangedSlot()
{
    QCheckBox* box = dynamic_cast<QCheckBox*>(QObject::sender());
    assert (box);

    string ds_type_name = box->property("DSType").toString().toStdString();

    bool load = box->checkState() == Qt::Checked;

    loginf << "DataSourcesLoadWidget: loadDSTypeChangedSlot: ds_type " << ds_type_name << " load " << load;

    COMPASS::instance().dataSourceManager().dsTypeLoadingWanted(ds_type_name, load);
}

/**
 */
void DataSourcesLoadTreeWidget::editClickedSlot()
{
    loginf << "DataSourcesLoadWidget: editClickedSlot";

    edit_menu_.exec(QCursor::pos());
}

/**
 */
void DataSourcesLoadTreeWidget::selectAllDSTypesSlot()
{
    loginf << "DataSourcesLoadWidget: selectAllDSTypesSlot";

    ds_man_.selectAllDSTypes();
}

/**
 */
void DataSourcesLoadTreeWidget::deselectAllDSTypesSlot()
{
    loginf << "DataSourcesLoadWidget: deselectAllDSTypesSlot";

    ds_man_.deselectAllDSTypes();
}

/**
 */
void DataSourcesLoadTreeWidget::selectAllDataSourcesSlot()
{
    loginf << "DataSourcesLoadWidget: selectAllDataSourcesSlot";

    ds_man_.selectAllDataSources();
}

/**
 */
void DataSourcesLoadTreeWidget::deselectAllDataSourcesSlot()
{
    loginf << "DataSourcesLoadWidget: deselectAllDataSourcesSlot";

    ds_man_.deselectAllDataSources();
}

/**
 */
void DataSourcesLoadTreeWidget::selectDSTypeSpecificDataSourcesSlot()
{
    QAction* action = dynamic_cast<QAction*>(sender());
    assert (action);

    string ds_type = action->property("ds_type").toString().toStdString();

    loginf << "DataSourcesLoadWidget: selectDSTypeSpecificDataSourcesSlot: ds_type '" << ds_type << "'";

    ds_man_.selectDSTypeSpecificDataSources(ds_type);
}

/**
 */
void DataSourcesLoadTreeWidget::deselectDSTypeSpecificDataSourcesSlot()
{
    QAction* action = dynamic_cast<QAction*>(sender());
    assert (action);

    string ds_type = action->property("ds_type").toString().toStdString();

    loginf << "DataSourcesLoadWidget: deselectDSTypeSpecificDataSourcesSlot: ds_type '" << ds_type << "'";

    ds_man_.deselectDSTypeSpecificDataSources(ds_type);
}

/**
 */
void DataSourcesLoadTreeWidget::deselectAllLinesSlot()
{
    loginf << "DataSourcesLoadWidget: deselectAllLinesSlot";

    ds_man_.deselectAllLines();
}

/**
 */
void DataSourcesLoadTreeWidget::selectSpecificLineSlot()
{
    QAction* action = dynamic_cast<QAction*>(sender());
    assert (action);

    unsigned int line_id = action->property("line_id").toUInt();

    loginf << "DataSourcesLoadWidget: selectSpecificLineSlot: line_id " << line_id;

    ds_man_.selectSpecificLineSlot(line_id);
}

/**
 */
void DataSourcesLoadTreeWidget::toogleShowCountsSlot()
{
    loginf << "DataSourcesLoadWidget: toogleShowCountsSlot";

    ds_man_.config().load_widget_show_counts_ = !ds_man_.config().load_widget_show_counts_;

    updateContent();
}

/**
 */
void DataSourcesLoadTreeWidget::setUseDSType(const std::string& ds_type_name, bool use)
{
    ds_man_.dsTypeLoadingWanted(ds_type_name, use);
}

/**
 */
bool DataSourcesLoadTreeWidget::getUseDSType(const std::string& ds_type_name) const
{
    return ds_man_.dsTypeLoadingWanted(ds_type_name);
}

/**
 */
void DataSourcesLoadTreeWidget::setUseDS(unsigned int ds_id, bool use)
{
    ds_man_.dbDataSource(ds_id).loadingWanted(use);
}

/**
 */
bool DataSourcesLoadTreeWidget::getUseDS(unsigned int ds_id) const
{
    return ds_man_.dbDataSource(ds_id).loadingWanted();
}

/**
 */
void DataSourcesLoadTreeWidget::setUseDSLine(unsigned int ds_id, unsigned int ds_line, bool use)
{
    ds_man_.dbDataSource(ds_id).lineLoadingWanted(ds_line, use);
}

/**
 */
bool DataSourcesLoadTreeWidget::getUseDSLine(unsigned int ds_id, unsigned int ds_line) const
{
    return ds_man_.dbDataSource(ds_id).lineLoadingWanted(ds_line);
}
