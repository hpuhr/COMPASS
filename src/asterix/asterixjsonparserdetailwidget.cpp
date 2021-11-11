#include "asterixjsonparserdetailwidget.h"

#include "asterixjsonparser.h"
#include "datatypeformatselectionwidget.h"
#include "dbovariable.h"
#include "dbovariableselectionwidget.h"
#include "files.h"
#include "jsonobjectparser.h"
#include "logger.h"
#include "unitselectionwidget.h"
#include "compass.h"
#include "dbobjectmanager.h"
#include "dbobject.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

using namespace std;

ASTERIXJSONParserDetailWidget::ASTERIXJSONParserDetailWidget(ASTERIXJSONParser& parser, QWidget *parent)
    : QWidget(parent), parser_(parser)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    QFormLayout* form_layout = new QFormLayout;
    form_layout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);

    QFont font_bold;
    font_bold.setBold(true);

    //    QLabel* info_label_ {nullptr}; // shows type of mapping, or missing details
    info_label_ = new QLabel();
    form_layout->addRow("Info", info_label_);

    //    QCheckBox* active_check_ {nullptr};
    active_check_ = new QCheckBox();
    active_check_->setDisabled(true);
    //active_check->setChecked(map_it.second.second->active());
    connect(active_check_, SIGNAL(stateChanged(int)), this, SLOT(mappingActiveChangedSlot()));
    form_layout->addRow("Active", active_check_);

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    form_layout->addRow(line);



    //    QLineEdit* comment_edit_ {nullptr};

    //    QPushButton* mapping_button_ {nullptr}; // displays current action: add/delete mapping
    //    QPushButton* dbovar_button_ {nullptr}; // displays current action: create new dbovar

    //    QLabel* json_key_label_ {nullptr};
    QLabel* asterix_label = new QLabel("ASTERIX");
    asterix_label->setFont(font_bold);
    form_layout->addRow(asterix_label);

    json_key_label_ = new QLabel();
    json_key_label_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    form_layout->addRow("JSON Key", json_key_label_);

    asterix_desc_label_ = new QLabel();
    asterix_desc_label_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    asterix_desc_label_->setTextFormat(Qt::TextFormat::PlainText);
    asterix_desc_label_->setWordWrap(true);
    form_layout->addRow("Description", asterix_desc_label_);

    //    QCheckBox* in_array_check_ {nullptr};

    in_array_check_ = new QCheckBox();
    in_array_check_->setDisabled(true);
    connect(in_array_check_, SIGNAL(stateChanged(int)), this, SLOT(mappingInArrayChangedSlot()));
    form_layout->addRow("In Array", in_array_check_);

    //    QCheckBox* append_check {nullptr};
    append_check = new QCheckBox();
    append_check->setDisabled(true);
    connect(append_check, SIGNAL(stateChanged(int)), this, SLOT(mappingAppendChangedSlot()));
    form_layout->addRow("Append", append_check);


    //    UnitSelectionWidget* unit_sel_ {nullptr};
    unit_sel_ = new UnitSelectionWidget();
    form_layout->addRow("Unit", unit_sel_);



    //    DataTypeFormatSelectionWidget* data_format_widget_ {nullptr};
    data_format_widget_ = new DataTypeFormatSelectionWidget();
    form_layout->addRow("Format", data_format_widget_);

    QFrame* line2 = new QFrame();
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);
    form_layout->addRow(line2);

    // dbo var

    QLabel* dbovar_label = new QLabel("DBOVariable");
    dbovar_label->setFont(font_bold);
    form_layout->addRow(dbovar_label);

    //    DBOVariableSelectionWidget* dbo_var_sel_ {nullptr};
    dbo_var_sel_ = new DBOVariableSelectionWidget();
    dbo_var_sel_->showMetaVariables(false);
    dbo_var_sel_->showDBOOnly(parser_.dbObjectName());
    dbo_var_sel_->showEmptyVariable(true);
    dbo_var_sel_->setDisabled(true);

    connect(dbo_var_sel_, &DBOVariableSelectionWidget::selectionChanged,
            this, &ASTERIXJSONParserDetailWidget::mappingDBOVariableChangedSlot);
    form_layout->addRow("Name", dbo_var_sel_);

    main_layout->addLayout(form_layout);

    // buttons

    dbovar_new_button_ = new QPushButton("New DBOVariable");
    dbovar_new_button_->setHidden(true);
    connect(dbovar_new_button_, &QPushButton::clicked,
            this, &ASTERIXJSONParserDetailWidget::createNewDBVariableSlot);
    main_layout->addWidget(dbovar_new_button_);

    dbovar_rename_button_ = new QPushButton("Rename DBOVariable");
    dbovar_rename_button_->setHidden(true);
    connect(dbovar_rename_button_, &QPushButton::clicked,
            this, &ASTERIXJSONParserDetailWidget::renameDBVariableSlot);
    main_layout->addWidget(dbovar_rename_button_);

    dbovar_delete_button_ = new QPushButton("Delete DBOVariable");
    dbovar_delete_button_->setHidden(true);
    connect(dbovar_delete_button_, &QPushButton::clicked,
            this, &ASTERIXJSONParserDetailWidget::deleteDBVariableSlot);
    main_layout->addWidget(dbovar_delete_button_);

    mapping_button_ = new QPushButton();
    mapping_button_->setHidden(true);
    connect(mapping_button_, &QPushButton::clicked,
            this, &ASTERIXJSONParserDetailWidget::mappingActionSlot);
    main_layout->addWidget(mapping_button_);

    setLayout(main_layout);
}

void ASTERIXJSONParserDetailWidget::currentIndexChangedSlot (unsigned int index)
{
    loginf << "ASTERIXJSONParserDetailWidget: currentIndexChangedSlot: index " << index;

    assert (index < parser_.totalEntrySize());

    assert (info_label_);
    assert (active_check_);
    assert (json_key_label_);
    assert (unit_sel_);
    assert (data_format_widget_);

    // existing mapping

    if (index < parser_.dataMappings().size()) // is actual mapping
    {
        JSONDataMapping& mapping = parser_.dataMappings().at(index);

        loginf << "ASTERIXJSONParserDetailWidget: currentIndexChangedSlot: mapping " << index
               << " key '" << mapping.jsonKey() << "'";

        if (!parser_.existsJSONKeyInCATInfo(mapping.jsonKey()))
            info_label_->setText("JSON Key not found in ASTERIX Info");
        else
            info_label_->setText("Existing Mapping");

        active_check_->setDisabled(false);
        active_check_->setChecked(mapping.active());

        showJSONKey(mapping.jsonKey());

        unit_sel_->update(mapping.dimensionRef(), mapping.unitRef());
        data_format_widget_->update(mapping.formatDataTypeRef(), mapping.jsonValueFormatRef());

        showDBOVariable(mapping.dboVariableName(), true);

        mapping_button_->setText("Delete Mapping");
        mapping_button_->setHidden(false);

        return;
    }

    active_check_->setDisabled(true);
    active_check_->setChecked(false);

    // unmapped JSON key

    index -= parser_.dataMappings().size();

    if (index < parser_.notAddedJSONKeys().size())
    {
        info_label_->setText("Unmapped JSON Key");

        string key = parser_.notAddedJSONKeys().at(index);

        loginf << "ASTERIXJSONParserDetailWidget: currentIndexChangedSlot: not added JSON " << index
               << " key '" << key << "'";

        showJSONKey(key);
        unit_sel_->clear();
        data_format_widget_->clear();
        showDBOVariable("");

        mapping_button_->setText("Add Mapping");
        mapping_button_->setHidden(false);

        return;
    }

    // not mapped DBOVariable

    index -= parser_.notAddedJSONKeys().size();

    assert (index < parser_.notAddedDBOVariables().size());

    info_label_->setText("Unmapped DBO Variable");
    string dbovar = parser_.notAddedDBOVariables().at(index);

    loginf << "ASTERIXJSONParserDetailWidget: currentIndexChangedSlot: not added dbovar " << index
           << " key '" << dbovar << "'";


    showJSONKey("");
    unit_sel_->clear();
    data_format_widget_->clear();
    showDBOVariable(dbovar);

    mapping_button_->setText("");
    mapping_button_->setHidden(true);
}

void ASTERIXJSONParserDetailWidget::showJSONKey (const std::string& key)
{
    assert (json_key_label_);
    assert (asterix_desc_label_);

    if (!key.size())
    {
        json_key_label_->setText("");
        asterix_desc_label_->setText("");
    }
    else
    {
        json_key_label_->setText(key.c_str());

        const jASTERIX::CategoryItemInfo& item_info = parser_.itemInfo();

        if (item_info.count(key))
            asterix_desc_label_->setText(item_info.at(key).description_.c_str());
        else
            asterix_desc_label_->setText("");
    }
}

void ASTERIXJSONParserDetailWidget::showDBOVariable (const std::string& var_name, bool mapping_exists)
{
    assert (dbo_var_sel_);

    if (var_name.size())
    {
        dbo_var_sel_->setDisabled(false);

        assert (parser_.dbObject().hasVariable(var_name));
        dbo_var_sel_->selectedVariable(parser_.dbObject().variable(var_name));

        dbovar_new_button_->setHidden(true);
        dbovar_rename_button_->setHidden(false);
        dbovar_delete_button_->setHidden(false);
    }
    else
    {
        dbo_var_sel_->setDisabled(true);
        dbo_var_sel_->selectEmptyVariable();

        dbovar_new_button_->setHidden(!mapping_exists);
        dbovar_rename_button_->setHidden(true);
        dbovar_delete_button_->setHidden(true);
    }
}


void ASTERIXJSONParserDetailWidget::mappingActiveChangedSlot()
{
    loginf << "ASTERIXJSONParserDetailWidget: mappingActiveChangedSlot";
}

void ASTERIXJSONParserDetailWidget::mappingInArrayChangedSlot()
{
    loginf << "ASTERIXJSONParserDetailWidget: mappingInArrayChangedSlot";
}

void ASTERIXJSONParserDetailWidget::mappingAppendChangedSlot()
{
    loginf << "ASTERIXJSONParserDetailWidget: mappingAppendChangedSlot";
}

void ASTERIXJSONParserDetailWidget::createNewDBVariableSlot()
{
    loginf << "ASTERIXJSONParserDetailWidget: createNewDBVariableSlot";

}

void ASTERIXJSONParserDetailWidget::deleteDBVariableSlot()
{
    loginf << "ASTERIXJSONParserDetailWidget: deleteDBVariableSlot";
}
void ASTERIXJSONParserDetailWidget::renameDBVariableSlot()
{
    loginf << "ASTERIXJSONParserDetailWidget: renameDBVariableSlot";
}

void ASTERIXJSONParserDetailWidget::mappingDBOVariableChangedSlot()
{
    loginf << "ASTERIXJSONParserDetailWidget: mappingDBOVariableChangedSlot";
}


void ASTERIXJSONParserDetailWidget::mappingActionSlot()
{
    assert (mapping_button_);

    if (mapping_button_->text().toStdString() == "Add Mapping")
    {
        loginf << "ASTERIXJSONParserDetailWidget: mappingActionSlot: add";
    }
    else if (mapping_button_->text().toStdString() == "Delete Mapping")
    {
        loginf << "ASTERIXJSONParserDetailWidget: mappingActionSlot: delete";
    }
    else
        throw runtime_error("ASTERIXJSONParserDetailWidget: mappingActionSlot: unknown action'"
            + mapping_button_->text().toStdString() + "'");
}
