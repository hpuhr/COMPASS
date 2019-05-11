#include "asterixconfigwidget.h"
#include "asteriximportertask.h"
#include "asterixframingcombobox.h"
#include "asterixeditioncombobox.h"
#include "logger.h"
#include "files.h"

#include <jasterix/category.h>

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QListWidget>
#include <QDesktopServices>
#include <QGridLayout>
#include <QComboBox>
#include <QCheckBox>

using namespace std;
using namespace Utils;
using namespace jASTERIX;

ASTERIXConfigWidget::ASTERIXConfigWidget(ASTERIXImporterTask& task, QWidget *parent)
     : QWidget(parent), task_(task)
{
    jasterix_ = task_.jASTERIX();

    QFont font_bold;
    font_bold.setBold(true);

    main_layout_ = new QVBoxLayout ();

    QLabel *main_label = new QLabel ("ASTERIX Configuration");
    main_label->setFont (font_bold);
    main_layout_->addWidget (main_label);

    // framing stuff
    {
        QGridLayout* framing_grid = new QGridLayout ();

        QLabel* framing_label = new QLabel ("Framing");
        framing_grid->addWidget (framing_label, 0, 0);

        framing_combo_ = new ASTERIXFramingComboBox(task_);
        connect(framing_combo_, SIGNAL(changedFraming()), this, SLOT(framingChangedSlot()));
        framing_grid->addWidget (framing_combo_, 0, 1);

        framing_edit_ = new QPushButton("Edit");
        connect(framing_edit_, SIGNAL( clicked() ), this, SLOT(framingEditSlot()));
        framing_grid->addWidget (framing_edit_, 0, 2);

        updateFraming();

        main_layout_->addLayout(framing_grid);
    }

    // categories stuff
    {
//        QLabel *cat_label = new QLabel ("Categories");
//        main_layout_->addWidget (cat_label);

        categories_grid_ = new QGridLayout ();
        updateCategories();

        main_layout_->addLayout(categories_grid_);
    }

    // button stuff
    {
        QGridLayout* button_grid = new QGridLayout ();

        QPushButton* edit_db_button = new QPushButton ("Edit Data Block");
        button_grid->addWidget (edit_db_button, 0, 0);

        QPushButton* edit_cat_button = new QPushButton ("Edit Categories");
        button_grid->addWidget (edit_cat_button, 0, 1);

        QPushButton* refresh_button = new QPushButton ("Refresh");
        button_grid->addWidget (refresh_button, 0, 2);

        main_layout_->addLayout(button_grid);
    }

    setLayout (main_layout_);

    //QDesktopServices::openUrl(QUrl("file:///home/sk/.atsdb/data/jasterix_definitions/categories/categories.json"));
}

ASTERIXConfigWidget::~ASTERIXConfigWidget()
{

}

void ASTERIXConfigWidget::updateSlot()
{

}

void ASTERIXConfigWidget::framingChangedSlot()
{
    assert (framing_combo_);
    loginf << "ASTERIXConfigWidget: framingChangedSlot: " << framing_combo_->getFraming();

    task_.currentFraming(framing_combo_->getFraming());

    if (task_.currentFraming() == "")
        framing_edit_->setDisabled(true);
    else
        framing_edit_->setDisabled(false);
}

void ASTERIXConfigWidget::framingEditSlot()
{
    std::string framing_path = "file:///"+jasterix_->framingsFolderPath()+"/"+task_.currentFraming();
    loginf << "ASTERIXConfigWidget: framingEditSlot: path '" << framing_path << "'";
    QDesktopServices::openUrl(QUrl(framing_path.c_str()));
}

void ASTERIXConfigWidget::updateFraming ()
{
    assert (framing_combo_);
    assert (framing_edit_);
    framing_combo_->setFraming(task_.currentFraming());

    if (task_.currentFraming() == "")
        framing_edit_->setDisabled(true);
    else
        framing_edit_->setDisabled(false);

}

void ASTERIXConfigWidget::updateCategories()
{
    assert (categories_grid_);

    QLayoutItem *child;
    while ((child = categories_grid_->takeAt(0)) != 0)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }

    QFont font_bold;
    font_bold.setBold(true);

//    QLabel *use_label = new QLabel ("Use");
//    use_label->setFont (font_bold);
//    categories_grid_->addWidget (use_label, 0, 0);

    QLabel *cat_label = new QLabel ("Category");
    cat_label->setFont (font_bold);
    categories_grid_->addWidget (cat_label, 0, 0);

    QLabel *edition_label = new QLabel ("Edition");
    edition_label->setFont (font_bold);
    categories_grid_->addWidget (edition_label, 0, 1);

    QLabel *edit_label = new QLabel ("Edit");
    edit_label->setFont (font_bold);
    categories_grid_->addWidget (edit_label, 0, 2);

    QIcon edit_icon(Files::getIconFilepath("edit.png").c_str());
    //QIcon del_icon(Files::getIconFilepath("delete.png").c_str());

    unsigned int row=1;

    for (auto& cat_it : jasterix_->categories())
    {
        QCheckBox* cat_check = new QCheckBox (cat_it.first.c_str());
        connect(cat_check, SIGNAL(clicked()), this, SLOT(categoryCheckedSlot()));
        cat_check->setProperty("category", cat_it.first.c_str());
        categories_grid_->addWidget (cat_check, row, 0);

        const Category& cat = cat_it.second;

        ASTERIXEditionComboBox* ed_combo = new ASTERIXEditionComboBox(task_, cat);
        connect(ed_combo, SIGNAL(changedEdition(const std::string&,const std::string&)),
                this, SLOT(editionChangedSlot(const std::string&,const std::string&)));
        categories_grid_->addWidget (ed_combo, row, 1);

        QPushButton *edit = new QPushButton ();
        edit->setIcon(edit_icon);
        edit->setFixedSize ( UI_ICON_SIZE );
        edit->setFlat(UI_ICON_BUTTON_FLAT);
        connect(edit, SIGNAL(clicked()), this, SLOT(categoryEditSlot()));
        edit->setProperty("category", cat_it.first.c_str());
        categories_grid_->addWidget (edit, row, 2);

        row++;
    }
}

void ASTERIXConfigWidget::categoryCheckedSlot ()
{
    QCheckBox* widget = static_cast<QCheckBox*>(sender());
    assert (widget);

    QVariant cat = widget->property("category");
    std::string cat_str = cat.toString().toStdString();

    loginf << "ASTERIXConfigWidget: categoryCheckedSlot: cat " << cat_str;
}

void ASTERIXConfigWidget::editionChangedSlot(const std::string& cat_str, const std::string& ed_str)
{
    loginf << "ASTERIXConfigWidget: editionChangedSlot: cat " << cat_str << " edition " << ed_str;
}

void ASTERIXConfigWidget::categoryEditSlot ()
{
    QPushButton* widget = static_cast<QPushButton*>(sender());
    assert (widget);

    QVariant cat = widget->property("category");
    std::string cat_str = cat.toString().toStdString();

    loginf << "ASTERIXConfigWidget: categoryEditSlot: cat " << cat_str;
}
