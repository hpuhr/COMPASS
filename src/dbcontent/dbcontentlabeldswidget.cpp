#include "dbcontentlabeldswidget.h"
#include "compass.h"
#include "datasourcemanager.h"
#include "dbcontentlabelgenerator.h"
#include "logger.h"
#include "files.h"

#include <QHBoxLayout>
#include <QMenu>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QCheckBox>
#include <QLabel>
#include <QVariant>

using namespace std;
using namespace Utils;


DBContentLabelDSWidget::DBContentLabelDSWidget(DBContentLabelGenerator& label_generator, QWidget* parent,
                                                       Qt::WindowFlags f)
    : QWidget(parent, f), label_generator_(label_generator)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

//    list_widget_ = new QListWidget();
//    list_widget_->setSelectionMode(QListWidget::MultiSelection);
//    updateListSlot();

//    connect(list_widget_, &QListWidget::itemClicked,
//            this, &DBContentLabelDSWidget::itemClickedSlot);

//    main_layout->addWidget(list_widget_);

    ds_grid_ = new QGridLayout();
    ds_grid_->setContentsMargins(2,2,2,2);
    updateListSlot();
    main_layout->addLayout(ds_grid_);

//    QIcon arrow_lu_;
//    QIcon arrow_ru_;
//    QIcon arrow_ld_;
//    QIcon arrow_rd_;

    arrow_lu_ = QIcon(Files::getIconFilepath("arrow_lu.png").c_str());
    arrow_ru_ = QIcon(Files::getIconFilepath("arrow_ru.png").c_str());
    arrow_ld_ = QIcon(Files::getIconFilepath("arrow_ld.png").c_str());
    arrow_rd_ = QIcon(Files::getIconFilepath("arrow_rd.png").c_str());


    setLayout(main_layout);


    connect(&COMPASS::instance().dataSourceManager(), &DataSourceManager::dataSourcesChangedSignal,
            this, &DBContentLabelDSWidget::updateListSlot);
}

DBContentLabelDSWidget::~DBContentLabelDSWidget()
{

}

void DBContentLabelDSWidget::updateListSlot()
{
    loginf << "OSGViewConfigLabelDSWidget: updateListSlot";

    assert(ds_grid_);

    QLayoutItem* child;
    while ((child = ds_grid_->takeAt(0)) != nullptr)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }

    direction_buttons_.clear();

    unsigned int row=0;

    QFont font_bold;
    font_bold.setBold(true);

    QLabel* src_label = new QLabel("Data Source");
    src_label->setFont(font_bold);
    ds_grid_->addWidget(src_label, row, 0);

//    QLabel* line_label = new QLabel("Line");
//    line_label->setFont(font_bold);
//    ds_grid_->addWidget(line_label, row, 1);

    QLabel* dir_label = new QLabel("Direction");
    dir_label->setFont(font_bold);
    ds_grid_->addWidget(dir_label, row, 1);;

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    set<unsigned int> selected = label_generator_.labelDSIDs();

    for (const auto& ds_it : ds_man.dbDataSources())
    {
        ++row;

        QCheckBox* box = new QCheckBox(ds_it->name().c_str());
        box->setProperty("ds_id", ds_it->id());
        box->setChecked(selected.count(ds_it->id()));
        connect(box, &QCheckBox::clicked, this, &DBContentLabelDSWidget::sourceClickedSlot);
        ds_grid_->addWidget(box, row, 0);

//        QLabel* test = new QLabel("L1");
//        ds_grid_->addWidget(test, row, 1);

        QPushButton* direction = new QPushButton();
        direction->setProperty("ds_id", ds_it->id());
        direction->setIcon(arrow_lu_);
        direction->setFixedWidth(2*UI_ICON_SIZE.width());
        //direction->setFixedSize(UI_ICON_SIZE);
        direction->setFlat(UI_ICON_BUTTON_FLAT);
        connect(direction, &QPushButton::clicked, this, &DBContentLabelDSWidget::changeDirectionSlot);
        ds_grid_->addWidget(direction, row, 1);

        direction_buttons_[ds_it->id()] = direction;

//        QListWidgetItem* new_item = new QListWidgetItem;
//        new_item->setText(ds_man.dbDataSource(ds_id_it).name().c_str());
//        new_item->setData(Qt::UserRole, ds_id_it);

//        if (selected.count(ds_id_it))
//            new_item->setSelected(true);

//        list_widget_->addItem(new_item);
    }

//    assert(list_widget_);

//    list_widget_->clear();

//    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

//    set<unsigned int> selected = label_generator_.labelDSIDs();

//    for (auto ds_id_it : ds_man.getAllDsIDs())
//    {
//        if (!ds_man.hasDBDataSource(ds_id_it))
//            continue;

//        QListWidgetItem* new_item = new QListWidgetItem;
//        new_item->setText(ds_man.dbDataSource(ds_id_it).name().c_str());
//        new_item->setData(Qt::UserRole, ds_id_it);

//        if (selected.count(ds_id_it))
//            new_item->setSelected(true);

//        list_widget_->addItem(new_item);
//    }

//    // list_widget_->setMaximumHeight(items.size()*list_widget_->sizeHintForRow(0));

//    list_widget_->sortItems();
}

void DBContentLabelDSWidget::sourceClickedSlot()
{
    QCheckBox* widget = static_cast<QCheckBox*>(sender());
    assert(widget);

    QVariant ds_id_var = widget->property("ds_id");
    unsigned int ds_id = ds_id_var.value<unsigned int>();

    loginf << "OSGViewConfigLabelDSWidget: sourceClickedSlot: ds_id " << ds_id;

    if (label_generator_.labelDSIDs().count(ds_id))
        label_generator_.removeLabelDSID(ds_id);
    else
        label_generator_.addLabelDSID(ds_id);
}

void DBContentLabelDSWidget::changeDirectionSlot()
{
    QPushButton* widget = static_cast<QPushButton*>(sender());
    assert(widget);

    QVariant ds_id_var = widget->property("ds_id");
    unsigned int ds_id = ds_id_var.value<unsigned int>();

    loginf << "OSGViewConfigLabelDSWidget: changeDirectionSlot: ds_id " << ds_id;

    QMenu menu;

    QAction* lu_action = menu.addAction("Left Up");
    lu_action->setProperty("ds_id", ds_id);
    lu_action->setProperty("direction", 0);
    lu_action->setIcon(arrow_lu_);
    connect(lu_action, &QAction::triggered, this, &DBContentLabelDSWidget::selectDirectionSlot);

    QAction* ru_action = menu.addAction("Right Up");
    ru_action->setProperty("ds_id", ds_id);
    ru_action->setProperty("direction", 1);
    ru_action->setIcon(arrow_ru_);
    connect(ru_action, &QAction::triggered, this, &DBContentLabelDSWidget::selectDirectionSlot);

    QAction* ld_action = menu.addAction("Left Down");
    ld_action->setProperty("ds_id", ds_id);
    ld_action->setProperty("direction", 2);
    ld_action->setIcon(arrow_ld_);
    connect(ld_action, &QAction::triggered, this, &DBContentLabelDSWidget::selectDirectionSlot);

    QAction* rd_action = menu.addAction("Right Down");
    rd_action->setProperty("ds_id", ds_id);
    rd_action->setProperty("direction", 3);
    rd_action->setIcon(arrow_rd_);
    connect(rd_action, &QAction::triggered, this, &DBContentLabelDSWidget::selectDirectionSlot);

    menu.exec(QCursor::pos());
}

QIcon& DBContentLabelDSWidget::iconForDirection(LabelDirection direction)
{
    if (direction == LabelDirection::LEFT_UP)
        return arrow_lu_;
    else if (direction == LabelDirection::RIGHT_UP)
        return arrow_ru_;
    else if (direction == LabelDirection::LEFT_DOWN)
        return arrow_ld_;
    else
        return arrow_rd_;
}

void DBContentLabelDSWidget::selectDirectionSlot()
{
    QVariant ds_id_var = sender()->property("ds_id");
    unsigned int ds_id = ds_id_var.value<unsigned int>();

    QVariant dir_var = sender()->property("direction");
    unsigned int dir = dir_var.value<unsigned int>();
    assert (dir <= 3);

    loginf << "DBContentLabelDSWidget: selectDirectionSlot: ds_id " << ds_id << " dir " << dir;

    LabelDirection direction = LabelDirection(dir);
    assert (direction_buttons_.count(ds_id));
    direction_buttons_.at(ds_id)->setIcon(iconForDirection(direction));
}
