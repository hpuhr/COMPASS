#include "dbcontentlabeldswidget.h"
#include "compass.h"
#include "datasourcemanager.h"
#include "dbcontentlabelgenerator.h"
#include "logger.h"

#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

using namespace std;


DBContentLabelDSWidget::DBContentLabelDSWidget(DBContentLabelGenerator& label_generator, QWidget* parent,
                                                       Qt::WindowFlags f)
    : QWidget(parent, f), label_generator_(label_generator)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    list_widget_ = new QListWidget();
    list_widget_->setSelectionMode(QListWidget::MultiSelection);
    updateListSlot();

    connect(list_widget_, &QListWidget::itemClicked,
            this, &DBContentLabelDSWidget::itemClickedSlot);

    main_layout->addWidget(list_widget_);

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

    assert(list_widget_);

    list_widget_->clear();

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    set<unsigned int> selected = label_generator_.labelDSIDs();

    for (auto ds_id_it : ds_man.getAllDsIDs())
    {
        if (!ds_man.hasDBDataSource(ds_id_it))
            continue;

        QListWidgetItem* new_item = new QListWidgetItem;
        new_item->setText(ds_man.dbDataSource(ds_id_it).name().c_str());
        new_item->setData(Qt::UserRole, ds_id_it);

        if (selected.count(ds_id_it))
            new_item->setSelected(true);

        list_widget_->addItem(new_item);
    }

    // list_widget_->setMaximumHeight(items.size()*list_widget_->sizeHintForRow(0));

    list_widget_->sortItems();
}

void DBContentLabelDSWidget::itemClickedSlot(QListWidgetItem* item)
{
    QVariant v = item->data(Qt::UserRole);
    unsigned int ds_id = v.value<unsigned int>();

    loginf << "OSGViewConfigLabelDSWidget: itemClickedSlot: ds_id " << ds_id;

    if (label_generator_.labelDSIDs().count(ds_id))
        label_generator_.removeLabelDSID(ds_id);
    else
        label_generator_.addLabelDSID(ds_id);
}
