/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "asterixconfigwidget.h"
#include "asteriximporttask.h"
#include "asterixframingcombobox.h"
#include "asterixeditioncombobox.h"
#include "asterixrefeditioncombobox.h"
#include "asterixspfeditioncombobox.h"
#include "logger.h"
#include "files.h"
#include "stringconv.h"

#include <jasterix/category.h>
#include <jasterix/edition.h>
#include <jasterix/refedition.h>

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

ASTERIXConfigWidget::ASTERIXConfigWidget(ASTERIXImportTask& task, QWidget *parent)
     : QWidget(parent), task_(task)
{
    main_layout_ = new QVBoxLayout ();

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
        categories_grid_ = new QGridLayout ();
        updateCategories();

        main_layout_->addLayout(categories_grid_);
    }

    main_layout_->addStretch();

    // button stuff
    {
        QGridLayout* button_grid = new QGridLayout ();

        QPushButton* edit_db_button = new QPushButton ("Edit Data Block");
        connect(edit_db_button, SIGNAL( clicked() ), this, SLOT(editDataBlockSlot()));
        button_grid->addWidget (edit_db_button, 0, 0);

        QPushButton* edit_cat_button = new QPushButton ("Edit Categories");
        connect(edit_cat_button, SIGNAL( clicked() ), this, SLOT(editCategoriesSlot()));
        button_grid->addWidget (edit_cat_button, 0, 1);

        QPushButton* refresh_button = new QPushButton ("Refresh");
        connect(refresh_button, SIGNAL( clicked() ), this, SLOT(refreshjASTERIXSlot()));
        button_grid->addWidget (refresh_button, 0, 2);

        main_layout_->addLayout(button_grid);
    }

    setLayout (main_layout_);
}

ASTERIXConfigWidget::~ASTERIXConfigWidget()
{

}

void ASTERIXConfigWidget::editDataBlockSlot()
{
    loginf << "ASTERIXConfigWidget: editDataBlockSlot: open '" << task_.jASTERIX()->dataBlockDefinitionPath() << "'";
    QDesktopServices::openUrl(QUrl(task_.jASTERIX()->dataBlockDefinitionPath().c_str()));
}
void ASTERIXConfigWidget::editCategoriesSlot()
{
    loginf << "ASTERIXConfigWidget: editCategoriesSlot: open '" << task_.jASTERIX()->categoriesDefinitionPath() << "'";
    QDesktopServices::openUrl(QUrl(task_.jASTERIX()->categoriesDefinitionPath().c_str()));
}

void ASTERIXConfigWidget::refreshjASTERIXSlot()
{
    loginf << "ASTERIXConfigWidget: refreshjASTERIXSlot";

    task_.refreshjASTERIX();
    updateSlot();
}

void ASTERIXConfigWidget::updateSlot()
{
    loginf << "ASTERIXConfigWidget: updateSlot";
    updateFraming();
    updateCategories();
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
    std::string framing_path = "file:///"+task_.jASTERIX()->framingsFolderPath()+"/"+task_.currentFraming()+".json";
    loginf << "ASTERIXConfigWidget: framingEditSlot: path '" << framing_path << "'";
    QDesktopServices::openUrl(QUrl(framing_path.c_str()));
}

void ASTERIXConfigWidget::updateFraming ()
{
    assert (framing_combo_);
    assert (framing_edit_);

    framing_combo_->loadFramings();
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
    while ((child = categories_grid_->takeAt(0)) != nullptr)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }

    QFont font_bold;
    font_bold.setBold(true);

    QLabel *cat_label = new QLabel ("Category");
    cat_label->setFont (font_bold);
    categories_grid_->addWidget (cat_label, 0, 0);

    QLabel *edition_label = new QLabel ("Edition");
    edition_label->setFont (font_bold);
    categories_grid_->addWidget (edition_label, 0, 1);

    QLabel *edition_edit_label = new QLabel ("Edit");
    edition_edit_label->setFont (font_bold);
    categories_grid_->addWidget (edition_edit_label, 0, 2);

    QLabel *ref_label = new QLabel ("REF");
    ref_label->setFont (font_bold);
    categories_grid_->addWidget (ref_label, 0, 3);

    QLabel *ref_edit_label = new QLabel ("Edit");
    ref_edit_label->setFont (font_bold);
    categories_grid_->addWidget (ref_edit_label, 0, 4);

    QLabel *spf_label = new QLabel ("SPF");
    spf_label->setFont (font_bold);
    categories_grid_->addWidget (spf_label, 0, 5);

    QLabel *spf_edit_label = new QLabel ("Edit");
    spf_edit_label->setFont (font_bold);
    categories_grid_->addWidget (spf_edit_label, 0, 6);

    QIcon edit_icon(Files::getIconFilepath("edit.png").c_str());

    int row=1;

    ref_edit_buttons_.clear();

    for (auto& cat_it : task_.jASTERIX()->categories())
    {
        unsigned int category = cat_it.first;

        QCheckBox* cat_check = new QCheckBox (String::categoryString(category).c_str());
        connect(cat_check, &QCheckBox::clicked, this, &ASTERIXConfigWidget::categoryCheckedSlot);
        cat_check->setProperty("category", category);
        if (task_.hasConfiguratonFor(category))
            cat_check->setChecked(task_.decodeCategory(category));
        categories_grid_->addWidget (cat_check, row, 0);

        const std::shared_ptr<Category> cat = cat_it.second;

        ASTERIXEditionComboBox* ed_combo = new ASTERIXEditionComboBox(task_, cat);
        if (task_.hasConfiguratonFor(category))
            ed_combo->setEdition(task_.editionForCategory(category));
        connect(ed_combo, &ASTERIXEditionComboBox::changedEdition, this, &ASTERIXConfigWidget::editionChangedSlot);
        categories_grid_->addWidget (ed_combo, row, 1);

        QPushButton *ed_edit = new QPushButton ();
        ed_edit->setIcon(edit_icon);
        ed_edit->setFixedSize(UI_ICON_SIZE);
        ed_edit->setFlat(UI_ICON_BUTTON_FLAT);
        connect(ed_edit, &QPushButton::clicked, this, &ASTERIXConfigWidget::categoryEditionEditSlot);
        ed_edit->setProperty("category", category);
        categories_grid_->addWidget (ed_edit, row, 2);

        // ref
        ASTERIXREFEditionComboBox* ref_combo = new ASTERIXREFEditionComboBox(task_, cat);
        if (task_.hasConfiguratonFor(category))
            ref_combo->setREFEdition(task_.refEditionForCategory(category));
        connect(ref_combo, &ASTERIXREFEditionComboBox::changedREFSignal,
                this, &ASTERIXConfigWidget::refEditionChangedSlot);
        categories_grid_->addWidget (ref_combo, row, 3);

        QPushButton *ref_edit = new QPushButton ();
        ref_edit->setIcon(edit_icon);
        ref_edit->setFixedSize(UI_ICON_SIZE);
        ref_edit->setFlat(UI_ICON_BUTTON_FLAT);
        connect(ref_edit, &QPushButton::clicked, this, &ASTERIXConfigWidget::categoryREFEditionEditSlot);
        ref_edit->setProperty("category", category);

        if (!ref_combo->isEnabled()) // is disabled when no refs exist
            ref_edit->setDisabled(true);

        categories_grid_->addWidget (ref_edit, row, 4);
        assert (!ref_edit_buttons_.count(category));
        ref_edit_buttons_[category] = ref_edit;

        // spf
        ASTERIXSPFEditionComboBox* spf_combo = new ASTERIXSPFEditionComboBox(task_, cat);
        if (task_.hasConfiguratonFor(category))
            spf_combo->setSPFEdition(task_.spfEditionForCategory(category));
        connect(spf_combo, &ASTERIXSPFEditionComboBox::changedSPFSignal,
                this, &ASTERIXConfigWidget::spfEditionChangedSlot);
        categories_grid_->addWidget (spf_combo, row, 5);

        QPushButton *spf_edit = new QPushButton ();
        spf_edit->setIcon(edit_icon);
        spf_edit->setFixedSize(UI_ICON_SIZE);
        spf_edit->setFlat(UI_ICON_BUTTON_FLAT);
        connect(spf_edit, &QPushButton::clicked, this, &ASTERIXConfigWidget::categorySPFEditionEditSlot);
        spf_edit->setProperty("category", category);

        if (!spf_combo->isEnabled()) // is disabled when no spf exist
            spf_edit->setDisabled(true);

        categories_grid_->addWidget (spf_edit, row, 6);
        assert (!spf_edit_buttons_.count(category));
        spf_edit_buttons_[category] = spf_edit;

        row++;
    }
}

void ASTERIXConfigWidget::categoryCheckedSlot ()
{
    QCheckBox* widget = static_cast<QCheckBox*>(sender());
    assert (widget);

    QVariant cat_var = widget->property("category");
    bool decode = widget->checkState() == Qt::Checked;
    unsigned int cat = cat_var.toUInt();

    loginf << "ASTERIXConfigWidget: categoryCheckedSlot: cat " << cat;

    task_.decodeCategory(cat, decode);
}

void ASTERIXConfigWidget::editionChangedSlot(const std::string& cat_str, const std::string& ed_str)
{
    loginf << "ASTERIXConfigWidget: editionChangedSlot: cat " << cat_str << " edition " << ed_str;

    unsigned int cat = std::stoul(cat_str);
    task_.editionForCategory(cat, ed_str);
}

void ASTERIXConfigWidget::refEditionChangedSlot(const std::string& cat_str, const std::string& ed_str)
{
    loginf << "ASTERIXConfigWidget: refEditionChangedSlot: cat " << cat_str << " ref '" << ed_str << "'";

    unsigned int cat = std::stoul(cat_str);
    task_.refEditionForCategory(cat, ed_str);

    assert (ref_edit_buttons_.count(cat));
    if (ed_str.size()) // enable or disable button if edition is empty
        ref_edit_buttons_.at(cat)->setDisabled(false);
    else
        ref_edit_buttons_.at(cat)->setDisabled(true);
}

void ASTERIXConfigWidget::spfEditionChangedSlot(const std::string& cat_str, const std::string& ed_str)
{
    loginf << "ASTERIXConfigWidget: spfEditionChangedSlot: cat " << cat_str << " ref '" << ed_str << "'";

    unsigned int cat = std::stoul(cat_str);
    task_.spfEditionForCategory(cat, ed_str);

    assert (spf_edit_buttons_.count(cat));
    if (ed_str.size()) // enable or disable button if edition is empty
        spf_edit_buttons_.at(cat)->setDisabled(false);
    else
        spf_edit_buttons_.at(cat)->setDisabled(true);
}

void ASTERIXConfigWidget::categoryEditionEditSlot ()
{
    loginf << "ASTERIXConfigWidget: categoryEditionEditSlot";

    QPushButton* widget = static_cast<QPushButton*>(sender());
    assert (widget);

    QVariant cat_var = widget->property("category");
    unsigned int cat = cat_var.toUInt();
    std::string edition_str;
    if (task_.hasConfiguratonFor(cat))
        edition_str = task_.editionForCategory(cat);
    else
        edition_str = task_.jASTERIX()->category(cat)->defaultEdition();

    assert (task_.jASTERIX()->hasCategory(cat));
    assert (task_.jASTERIX()->category(cat)->hasEdition(edition_str));
    std::string def_path = task_.jASTERIX()->category(cat)->editionPath(edition_str);

    loginf << "ASTERIXConfigWidget: categoryEditSlot: cat " << cat << " path '" << def_path << "'";

    QDesktopServices::openUrl(QUrl(def_path.c_str()));
}

void ASTERIXConfigWidget::categoryREFEditionEditSlot ()
{
    loginf << "ASTERIXConfigWidget: categoryREFEditionEditSlot";

    QPushButton* widget = static_cast<QPushButton*>(sender());
    assert (widget);

    QVariant cat_var = widget->property("category");
    unsigned int cat = cat_var.toUInt();
    std::string ref_edition_str;
    if (task_.hasConfiguratonFor(cat))
        ref_edition_str = task_.refEditionForCategory(cat);
    else
        ref_edition_str = task_.jASTERIX()->category(cat)->defaultREFEdition();

    loginf << "ASTERIXConfigWidget: categoryREFEditionEditSlot: ref '" << ref_edition_str << "'";

    assert (task_.jASTERIX()->hasCategory(cat));
    assert (task_.jASTERIX()->category(cat)->hasREFEdition(ref_edition_str));
    std::string def_path = task_.jASTERIX()->category(cat)->refEditionPath(ref_edition_str);

    loginf << "ASTERIXConfigWidget: categoryREFEditionEditSlot: cat " << cat << " ref path '" << def_path << "'";

    QDesktopServices::openUrl(QUrl(def_path.c_str()));
}

void ASTERIXConfigWidget::categorySPFEditionEditSlot ()
{
    loginf << "ASTERIXConfigWidget: categorySPFEditionEditSlot";

    QPushButton* widget = static_cast<QPushButton*>(sender());
    assert (widget);

    QVariant cat_var = widget->property("category");
    unsigned int cat = cat_var.toUInt();
    std::string spf_edition_str;
    if (task_.hasConfiguratonFor(cat))
        spf_edition_str = task_.spfEditionForCategory(cat);
    else
        spf_edition_str = task_.jASTERIX()->category(cat)->defaultSPFEdition();

    loginf << "ASTERIXConfigWidget: categorySPFEditionEditSlot: spf '" << spf_edition_str << "'";

    assert (task_.jASTERIX()->hasCategory(cat));
    assert (task_.jASTERIX()->category(cat)->hasSPFEdition(spf_edition_str));
    std::string def_path = task_.jASTERIX()->category(cat)->spfEditionPath(spf_edition_str);

    loginf << "ASTERIXConfigWidget: categorySPFEditionEditSlot: cat " << cat << " ref path '" << def_path << "'";

    QDesktopServices::openUrl(QUrl(def_path.c_str()));
}
