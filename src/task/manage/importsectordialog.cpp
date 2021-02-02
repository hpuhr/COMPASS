#include "importsectordialog.h"

#include <QLineEdit>
#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

ImportSectorDialog::ImportSectorDialog(const std::string& layer_name,
                                       QWidget* parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{
    setWindowFlags(Qt::Window | Qt::WindowTitleHint); //  | Qt::CustomizeWindowHint

    setWindowTitle("Import Sector");

    setModal(true);

    QVBoxLayout* main_layout = new QVBoxLayout();

    int row = 0;
    QGridLayout* grid = new QGridLayout();

    // sector layer
    grid->addWidget(new QLabel("Sector Layer"), row, 0);

    layer_name_edit_edit_ = new QLineEdit ();
    layer_name_edit_edit_->setText(layer_name.c_str());
    grid->addWidget(layer_name_edit_edit_, row, 1);

    ++row;

    // exclude
    grid->addWidget(new QLabel("Exclude"), row, 0);

    exclude_check_ = new QCheckBox();
    exclude_check_->setChecked(false);
    grid->addWidget(exclude_check_, row, 1);

    main_layout->addLayout(grid);

    main_layout->addStretch();

    QHBoxLayout* button_layout = new QHBoxLayout();

    QPushButton* quit_button_ = new QPushButton("Cancel");
    connect(quit_button_, &QPushButton::clicked, this, &ImportSectorDialog::cancelSlot);
    button_layout->addWidget(quit_button_);

    QPushButton* import_button_ = new QPushButton("Import");
    connect(import_button_, &QPushButton::clicked, this, &ImportSectorDialog::importSlot);
    button_layout->addWidget(import_button_);

    main_layout->addLayout(button_layout);

    setLayout(main_layout);
}


std::string ImportSectorDialog::layerName()
{
    assert (layer_name_edit_edit_);
    return layer_name_edit_edit_->text().toStdString();
}

bool ImportSectorDialog::exclude ()
{
    assert (exclude_check_);
    return exclude_check_->checkState() == Qt::Checked;
}

void ImportSectorDialog::cancelSlot()
{
    //import_ = false;
    reject();
}

void ImportSectorDialog::importSlot()
{
    //import_ = true;
    accept();
}
