#include "datasourceeditwidget.h"
#include "datasourcemanager.h"
#include "configurationdatasource.h"
#include "dbdatasource.h"
#include "logger.h"

#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>

using namespace std;

DataSourceEditWidget::DataSourceEditWidget(DataSourceManager& ds_man, DataSourcesConfigurationDialog& dialog)
    : ds_man_(ds_man), dialog_(dialog)
{
    setMaximumWidth(300);

//    QFont font_bold;
//    font_bold.setBold(true);

//    QFont font_big;
//    font_big.setPointSize(18);

    QVBoxLayout* main_layout = new QVBoxLayout();

//    QLabel* main_label = new QLabel("Edit Data Source");
//    main_label->setFont(font_big);
//    main_layout->addWidget(main_label);

    QGridLayout* properties_layout_ = new QGridLayout();

    unsigned int row = 0;
    QLabel* name_label = new QLabel("Name");
    properties_layout_->addWidget(name_label, row, 0);

    name_edit_ = new QLineEdit();
    //connect(name_edit_, SIGNAL(returnPressed()), this, SLOT(editNameSlot()));
    properties_layout_->addWidget(name_edit_, row, 1);
    row++;

    //name_edit_
    //short_name_edit_

    QLabel* short_name_label = new QLabel("Short Name");
    properties_layout_->addWidget(short_name_label, row, 0);

    short_name_edit_ = new QLineEdit();
    //connect(name_edit_, SIGNAL(returnPressed()), this, SLOT(editNameSlot()));
    properties_layout_->addWidget(short_name_edit_, row, 1);

    updateContent();

    main_layout->addLayout(properties_layout_);

    main_layout->addStretch();

    setLayout(main_layout);
}

void DataSourceEditWidget::showID(unsigned int ds_id)
{
    loginf << "DataSourceEditWidget: showID: id " << ds_id;

    has_current_ds_ = true;
    current_ds_id_ = ds_id;
    current_ds_in_db_ = ds_man_.hasDBDataSource(current_ds_id_);
    assert (ds_man_.hasConfigDataSource(current_ds_id_));

    updateContent();
}

void DataSourceEditWidget::clear()
{
    loginf << "DataSourceEditWidget: clear";

    has_current_ds_ = false;
    current_ds_id_ = 0;
    current_ds_in_db_ = false;

    updateContent();
}


void DataSourceEditWidget::updateContent()
{
    assert (name_edit_);
    assert (short_name_edit_);

    if (!has_current_ds_)
    {
        name_edit_->setText("");
        name_edit_->setDisabled(true);

        short_name_edit_->setText("");
        short_name_edit_->setDisabled(true);
    }
    else
    {
        if (current_ds_in_db_)
        {
            assert (ds_man_.hasDBDataSource(current_ds_id_));

            dbContent::DBDataSource& ds = ds_man_.dbDataSource(current_ds_id_);
            assert (ds_man_.hasConfigDataSource(current_ds_id_));

            name_edit_->setText(ds.name().c_str());
            name_edit_->setDisabled(false);

            if (ds.hasShortName())
                short_name_edit_->setText(ds.shortName().c_str());
            else
                short_name_edit_->setText("");

            short_name_edit_->setDisabled(false);
        }
        else // cfg only
        {
            assert (ds_man_.hasConfigDataSource(current_ds_id_));

            dbContent::ConfigurationDataSource& ds = ds_man_.configDataSource(current_ds_id_);

            name_edit_->setText(ds.name().c_str());
            name_edit_->setDisabled(false);

            if (ds.hasShortName())
                short_name_edit_->setText(ds.shortName().c_str());
            else
                short_name_edit_->setText("");

            short_name_edit_->setDisabled(false);
        }
    }
}
