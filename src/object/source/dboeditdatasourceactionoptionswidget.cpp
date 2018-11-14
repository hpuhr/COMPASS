#include "dboeditdatasourceactionoptionswidget.h"
#include "logger.h"

#include <QHBoxLayout>
#include <QCheckBox>
#include <QComboBox>

DBOEditDataSourceActionOptionsWidget::DBOEditDataSourceActionOptionsWidget(DBOEditDataSourceActionOptions& options,
                                     QWidget* parent, Qt::WindowFlags f)
: QWidget (parent, f), options_(&options)
{
    QHBoxLayout* main_layout = new QHBoxLayout ();

    perform_check_ = new QCheckBox (options_->sourceId().c_str());
    connect(perform_check_, SIGNAL(clicked()), this, SLOT(changedPerformSlot ()));
    main_layout->addWidget(perform_check_);

    action_box_ = new QComboBox();
    connect(action_box_, SIGNAL(activated(int)), this, SLOT(changedActionSlot (int)));
    main_layout->addWidget(action_box_);

    update();

    setLayout(main_layout);
}

void DBOEditDataSourceActionOptionsWidget::update ()
{
    assert (options_);
    assert (perform_check_);
    perform_check_->setChecked(options_->performFlag());

    assert (action_box_);
    action_box_->clear();

    for (auto& opt_it : *options_)
    {
        action_box_->addItem (opt_it.second.getActionString().c_str());
    }
    action_box_->setCurrentIndex(options_->currentActionId());
}

void DBOEditDataSourceActionOptionsWidget::changedPerformSlot ()
{
    assert (options_);
    assert (perform_check_);
    options_->performFlag(perform_check_->checkState() == Qt::Checked);
    loginf << "DBOEditDataSourceActionOptionsWidget: changedPerformSlot: " << options_->performFlag();
}

void DBOEditDataSourceActionOptionsWidget::changedActionSlot (int index)
{
    loginf << "DBOEditDataSourceActionOptionsWidget: changedActionSlot: index " << index;
    assert (options_);
    assert (index >= 0);
    options_->currentActionId(index);

    assert (perform_check_);
    if (index == 0)
    {
        perform_check_->setChecked(false);
        changedPerformSlot();
    }
    else if (perform_check_->checkState() != Qt::Checked)
    {
        perform_check_->setChecked(true);
        changedPerformSlot();
    }
}
