#include "metadbovariableconfigurationdialog.h"
#include "dbobjectmanager.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

MetaDBOVariableConfigurationDialog::MetaDBOVariableConfigurationDialog(DBObjectManager& dbo_man)
    : QDialog(), dbo_man_(dbo_man)
{
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    setModal(true);

    setMinimumSize(QSize(600, 800));

    QFont font_bold;
    font_bold.setBold(true);


    QVBoxLayout* main_layout = new QVBoxLayout();


    QHBoxLayout* button_layout = new QHBoxLayout();

    button_layout->addStretch();

    QPushButton* ok_button_ = new QPushButton("OK");
    connect(ok_button_, &QPushButton::clicked, this, &MetaDBOVariableConfigurationDialog::okClickedSlot);
    button_layout->addWidget(ok_button_);

    main_layout->addLayout(button_layout);

    setLayout(main_layout);
}


void MetaDBOVariableConfigurationDialog::okClickedSlot()
{
    emit okSignal();
}
