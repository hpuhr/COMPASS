#include "asterixjsonparserdetailwidget.h"
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
#include "dbovariableeditdialog.h"
#include "dbovariablecreatedialog.h"
#include "stringconv.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>

using namespace std;
using namespace Utils;

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

    //    QLabel* json_key_label_ {nullptr};
    QLabel* asterix_label = new QLabel("ASTERIX");
    asterix_label->setFont(font_bold);
    form_layout->addRow(asterix_label);

    json_key_box_ = new QComboBox();
    json_key_box_->setEditable(false);
    //json_key_box_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    connect(json_key_box_, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(mappingJSONKeyChangedSlot(const QString &)));
    form_layout->addRow("JSON Key", json_key_box_);

    asterix_desc_label_ = new QLabel();
    asterix_desc_label_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    asterix_desc_label_->setTextFormat(Qt::TextFormat::PlainText);
    asterix_desc_label_->setWordWrap(true);
    form_layout->addRow("Description", asterix_desc_label_);

    // QLabel* asterix_editions_label_ {nullptr};

    asterix_editions_label_ = new QLabel();
    asterix_editions_label_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    asterix_editions_label_->setTextFormat(Qt::TextFormat::PlainText);
    asterix_editions_label_->setWordWrap(true);
    form_layout->addRow("Editions", asterix_editions_label_);

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

    dbo_var_comment_edit_ = new QTextEdit();
    dbo_var_comment_edit_->setWordWrapMode(QTextOption::WrapMode::WrapAnywhere);

    connect(dbo_var_comment_edit_, &QTextEdit::textChanged, this,
        &ASTERIXJSONParserDetailWidget::dboVariableCommentChangedSlot);

    form_layout->addRow("Comment", dbo_var_comment_edit_);

    main_layout->addLayout(form_layout);

    // buttons

    new_dbovar_button_ = new QPushButton("New DBOVariable");
    new_dbovar_button_->setHidden(true);
    connect(new_dbovar_button_, &QPushButton::clicked,
            this, &ASTERIXJSONParserDetailWidget::createNewDBVariableSlot);
    main_layout->addWidget(new_dbovar_button_);

    dbovar_edit_button_ = new QPushButton("Edit DBOVariable");
    dbovar_edit_button_->setHidden(true);
    connect(dbovar_edit_button_, &QPushButton::clicked,
            this, &ASTERIXJSONParserDetailWidget::editDBVariableSlot);
    main_layout->addWidget(dbovar_edit_button_);

    dbovar_delete_button_ = new QPushButton("Delete DBOVariable");
    dbovar_delete_button_->setToolTip("Deletes the DBOVariable from the DBObject, not just from the Mapping.");
    dbovar_delete_button_->setHidden(true);
    connect(dbovar_delete_button_, &QPushButton::clicked,
            this, &ASTERIXJSONParserDetailWidget::deleteDBVariableSlot);
    main_layout->addWidget(dbovar_delete_button_);

    delete_mapping_button_ = new QPushButton("Delete Mapping");
    delete_mapping_button_->setHidden(true);
    connect(delete_mapping_button_, &QPushButton::clicked,
            this, &ASTERIXJSONParserDetailWidget::deleteMappingSlot);
    main_layout->addWidget(delete_mapping_button_);

    setLayout(main_layout);

    connect(&parser_, &ASTERIXJSONParser::rowContentChangedSignal,
            this, &ASTERIXJSONParserDetailWidget::rowContentChangedSlot);
}

void ASTERIXJSONParserDetailWidget::currentIndexChangedSlot (unsigned int index)
{
    loginf << "ASTERIXJSONParserDetailWidget: currentIndexChangedSlot: index " << index;

    setting_new_content_ = true;

    assert (index < parser_.totalEntrySize());

    assert (info_label_);
    assert (active_check_);
    assert (json_key_box_);
    assert (unit_sel_);
    assert (data_format_widget_);

    has_current_entry_ = true;
    entry_type_ = parser_.entryType(index);
    entry_index_ = index;

    if (entry_type_ == ASTERIXJSONParser::EntryType::ExistingMapping)
    {
        JSONDataMapping& mapping = parser_.mapping(entry_index_);

        loginf << "ASTERIXJSONParserDetailWidget: currentIndexChangedSlot: mapping " << entry_index_
               << " key '" << mapping.jsonKey() << "'";

        if (!parser_.existsJSONKeyInCATInfo(mapping.jsonKey()))
            info_label_->setText("JSON Key not found in ASTERIX Info");
        else
            info_label_->setText("Existing Mapping");

        active_check_->setDisabled(false);
        active_check_->setChecked(mapping.active());

        showJSONKey(mapping.jsonKey(), !parser_.existsJSONKeyInCATInfo(mapping.jsonKey()));

        unit_sel_->update(mapping.dimensionRef(), mapping.unitRef());
        data_format_widget_->update(mapping.formatDataTypeRef(), mapping.jsonValueFormatRef());

        showDBOVariable(mapping.dboVariableName(), true);

        delete_mapping_button_->setHidden(false);

        setting_new_content_ = false;

        return;
    }
    else if (entry_type_ == ASTERIXJSONParser::EntryType::UnmappedJSONKey)
    {

        info_label_->setText("Unmapped JSON Key");

        active_check_->setDisabled(true);
        active_check_->setChecked(false);

        string key = parser_.unmappedJSONKey(entry_index_);

        loginf << "ASTERIXJSONParserDetailWidget: currentIndexChangedSlot: not added JSON " << entry_index_
               << " key '" << key << "'";

        showJSONKey(key, false);
        unit_sel_->clear();
        data_format_widget_->clear();
        showDBOVariable("");

        delete_mapping_button_->setHidden(true);

        setting_new_content_ = false;

        return;
    }
    else if (entry_type_ == ASTERIXJSONParser::EntryType::UnmappedDBOVariable)
    {
        info_label_->setText("Unmapped DBO Variable");

        active_check_->setDisabled(true);
        active_check_->setChecked(false);

        string dbovar = parser_.unmappedDBOVariable(entry_index_);

        loginf << "ASTERIXJSONParserDetailWidget: currentIndexChangedSlot: not added dbovar " << entry_index_
               << " key '" << dbovar << "'";

        showJSONKey("", false);
        unit_sel_->clear();
        data_format_widget_->clear();
        showDBOVariable(dbovar);

        delete_mapping_button_->setHidden(true);

        setting_new_content_ = false;
    }
    else
        throw runtime_error("ASTERIXJSONParserDetailWidget: currentIndexChangedSlot: unknown entry type");

}

void ASTERIXJSONParserDetailWidget::rowContentChangedSlot (unsigned int index)
{
    loginf << "ASTERIXJSONParserDetailWidget: rowChangedSlot: index " << index;

    if (has_current_entry_ && entry_index_ == index)
    {
        currentIndexChangedSlot(index);
    }
}

void ASTERIXJSONParserDetailWidget::showJSONKey (const std::string& key, bool unmapped_selectable)
{
    assert (json_key_box_);
    assert (asterix_desc_label_);

    json_key_box_->clear();

    if (!key.size()) // shown none
    {
        assert (!unmapped_selectable);

        json_key_box_->addItem("");
        json_key_box_->setCurrentText("");
        json_key_box_->setDisabled(true);

        asterix_desc_label_->setText("");
        asterix_editions_label_->setText("");
    }
    else
    {
        json_key_box_->addItem(key.c_str());
        json_key_box_->setCurrentText(key.c_str());

        if (unmapped_selectable)
        {
            for (const auto& jkey_it : parser_.notAddedJSONKeys())
            {
                if (jkey_it != key)
                    json_key_box_->addItem(jkey_it.c_str());

            }

            json_key_box_->setDisabled(false);
        }
        else
            json_key_box_->setDisabled(true);

        const jASTERIX::CategoryItemInfo& item_info = parser_.categoryItemInfo();

        string editions;

        if (item_info.count(key))
        {
            asterix_desc_label_->setText(item_info.at(key).description_.c_str());

            for (auto& ed_it : item_info.at(key).editions_)
            {
                if (editions.size())
                    editions += ",";

                editions += ed_it;
            }

        }
        else
            asterix_desc_label_->setText("");

        asterix_editions_label_->setText(editions.c_str());
    }
}

void ASTERIXJSONParserDetailWidget::showDBOVariable (const std::string& var_name, bool mapping_exists)
{
    assert (dbo_var_sel_);
    assert (dbo_var_comment_edit_);

    if (var_name.size())
    {
        dbo_var_sel_->updateMenuEntries();

        dbo_var_sel_->setDisabled(false);

        assert (parser_.dbObject().hasVariable(var_name));
        dbo_var_sel_->selectedVariable(parser_.dbObject().variable(var_name));

        dbo_var_comment_edit_->setDisabled(false);

        dbo_var_comment_edit_->setText(parser_.dbObject().variable(var_name).description().c_str());

        if (mapping_exists)
        {
            new_dbovar_button_->setText("New DBOVariable");
            new_dbovar_button_->setHidden(false);
        }
        else
        {
            new_dbovar_button_->setHidden(true);
        }

        dbovar_edit_button_->setHidden(false);
        dbovar_delete_button_->setHidden(false);
    }
    else
    {
        dbo_var_sel_->setDisabled(false);
        dbo_var_sel_->selectEmptyVariable();

        dbo_var_comment_edit_->setDisabled(true);

        dbo_var_comment_edit_->setText("");

        new_dbovar_button_->setHidden(false);

        if (mapping_exists)
            new_dbovar_button_->setText("New DBOVariable");
        else
            new_dbovar_button_->setText("New DBOVariable && Mapping");

        dbovar_edit_button_->setHidden(true);
        dbovar_delete_button_->setHidden(true);
    }
}


void ASTERIXJSONParserDetailWidget::mappingActiveChangedSlot()
{
    if (setting_new_content_)
        return;

    loginf << "ASTERIXJSONParserDetailWidget: mappingActiveChangedSlot";

    assert (has_current_entry_);
    assert (entry_type_ == ASTERIXJSONParser::EntryType::ExistingMapping);
    assert (active_check_);

    parser_.mapping(entry_index_).active(active_check_->checkState() == Qt::Checked);

    parser_.doMappingChecks();

    parser_.selectMapping(entry_index_);
}

void ASTERIXJSONParserDetailWidget::mappingJSONKeyChangedSlot (const QString& text)
{
    if (setting_new_content_)
        return;

    loginf << "ASTERIXJSONParserDetailWidget: mappingJSONKeyChangedSlot";

    assert (has_current_entry_);
    assert (entry_type_ == ASTERIXJSONParser::EntryType::ExistingMapping);
    assert (in_array_check_);

    parser_.mapping(entry_index_).jsonKey(text.toStdString());

    parser_.doMappingChecks();

    parser_.selectMapping(entry_index_);
}

void ASTERIXJSONParserDetailWidget::mappingInArrayChangedSlot()
{
    if (setting_new_content_)
        return;

    loginf << "ASTERIXJSONParserDetailWidget: mappingInArrayChangedSlot";

    assert (has_current_entry_);
    assert (entry_type_ == ASTERIXJSONParser::EntryType::ExistingMapping);
    assert (in_array_check_);

    parser_.mapping(entry_index_).inArray(in_array_check_->checkState() == Qt::Checked);
}

void ASTERIXJSONParserDetailWidget::mappingAppendChangedSlot()
{
    if (setting_new_content_)
        return;

    loginf << "ASTERIXJSONParserDetailWidget: mappingAppendChangedSlot";

    assert (has_current_entry_);
    assert (entry_type_ == ASTERIXJSONParser::EntryType::ExistingMapping);
    assert (append_check);

    parser_.mapping(entry_index_).appendValue(append_check->checkState() == Qt::Checked);
}

void ASTERIXJSONParserDetailWidget::mappingDBOVariableChangedSlot()
{
    if (setting_new_content_)
        return;

    loginf << "ASTERIXJSONParserDetailWidget: mappingDBOVariableChangedSlot";

    assert (has_current_entry_);
    assert (entry_type_ == ASTERIXJSONParser::EntryType::ExistingMapping
            || entry_type_ == ASTERIXJSONParser::EntryType::UnmappedJSONKey);
    assert (dbo_var_sel_);

    if (entry_type_ == ASTERIXJSONParser::EntryType::ExistingMapping)
    {
        // setting variable in existing mapping

        if (dbo_var_sel_->hasVariable())
            parser_.mapping(entry_index_).dboVariableName(dbo_var_sel_->selectedVariable().name());
        else
            parser_.mapping(entry_index_).dboVariableName("");

        parser_.doMappingChecks();

        parser_.selectMapping(entry_index_);
    }
    else if (dbo_var_sel_->hasVariable())
    {
        // create new mapping

        string json_key = parser_.unmappedJSONKey(entry_index_);

        Configuration& new_cfg = parser_.configuration().addNewSubConfiguration("JSONDataMapping");
        new_cfg.addParameterString("json_key", json_key);
        new_cfg.addParameterString("db_object_name", parser_.dbObjectName());
        new_cfg.addParameterString("dbovariable_name", dbo_var_sel_->selectedVariable().name());

        parser_.generateSubConfigurable("JSONDataMapping", new_cfg.getInstanceId());

        parser_.doMappingChecks();

        assert (parser_.hasJSONKeyInMapping(json_key));

        parser_.selectMapping(parser_.indexOfJSONKeyInMapping(json_key));
    }
}

void ASTERIXJSONParserDetailWidget::dboVariableCommentChangedSlot()
{
    if (setting_new_content_)
        return;

    assert (has_current_entry_);
    assert (dbo_var_sel_);
    assert (dbo_var_sel_->hasVariable());
    assert (dbo_var_comment_edit_);

    dbo_var_sel_->selectedVariable().description(dbo_var_comment_edit_->document()->toPlainText().toStdString());
}


void ASTERIXJSONParserDetailWidget::createNewDBVariableSlot()
{
    loginf << "ASTERIXJSONParserDetailWidget: createNewDBVariableSlot";

    assert (has_current_entry_);
    assert (entry_type_ == ASTERIXJSONParser::EntryType::ExistingMapping
            || entry_type_ == ASTERIXJSONParser::EntryType::UnmappedJSONKey);
    assert (dbo_var_sel_);

    string json_key;

    if (entry_type_ == ASTERIXJSONParser::EntryType::ExistingMapping)
        json_key = parser_.mapping(entry_index_).jsonKey();
    else
        json_key = parser_.unmappedJSONKey(entry_index_);

    // set description
    string description {"From "+json_key};

    string name {json_key};

    if (parser_.categoryItemInfo().count(name))
    {
        if (parser_.categoryItemInfo().at(name).description_.size())
            description += "\n\n"+parser_.categoryItemInfo().at(name).description_;

//        if (parser_.categoryItemInfo().at(name).editions_.size())
//        {
//            string editions;

//            if (description.size())
//                description += "\n\n";

//            for (auto& ed_it : parser_.categoryItemInfo().at(name).editions_)
//            {
//                if (editions.size())
//                    editions += ",";

//                editions += ed_it;
//            }

//            description += "Editions: " + editions;
//        }
    }

    vector<string> parts = String::split(name, '.');
    if (parts.size())
        name = *parts.rbegin();


    DBOVariableCreateDialog dialog (parser_.dbObject(), name, description, this);

    int ret = dialog.exec();

    if (ret == QDialog::Accepted)
    {
        loginf << "ASTERIXJSONParserDetailWidget: createNewDBVariableSlot: accept";

        // create new dbo var
        {
            assert (!parser_.dbObject().hasVariable(dialog.name()));

            Configuration& new_cfg = parser_.dbObject().configuration().addNewSubConfiguration("DBOVariable");
            new_cfg.addParameterString("name", dialog.name());
            new_cfg.addParameterString("short_name", dialog.shortName());
            new_cfg.addParameterString("description", dialog.description());
            new_cfg.addParameterString("db_column_name", dialog.dbColumnName());
            new_cfg.addParameterString("data_type_str", dialog.dataTypeStr());
            new_cfg.addParameterString("representation_str", dialog.representationStr());
            new_cfg.addParameterString("dimension", dialog.dimension());
            new_cfg.addParameterString("unit", dialog.unit());

            parser_.dbObject().generateSubConfigurable("DBOVariable", new_cfg.getInstanceId());

            assert (parser_.dbObject().hasVariable(dialog.name()));
        }


        if (entry_type_ == ASTERIXJSONParser::EntryType::ExistingMapping) // set in existing mapping
        {
            parser_.mapping(entry_index_).dboVariableName(dialog.name());

            parser_.doMappingChecks();

            parser_.selectMapping(entry_index_);
        }
        else // create new mapping
        {
            assert (!parser_.hasJSONKeyInMapping(json_key));

            Configuration& new_cfg = parser_.configuration().addNewSubConfiguration("JSONDataMapping");
            new_cfg.addParameterString("json_key", json_key);
            new_cfg.addParameterString("db_object_name", parser_.dbObjectName());
            new_cfg.addParameterString("dbovariable_name", dialog.name());

            parser_.generateSubConfigurable("JSONDataMapping", new_cfg.getInstanceId());

            parser_.doMappingChecks();

            assert (parser_.hasJSONKeyInMapping(json_key));

            parser_.selectMapping(parser_.indexOfJSONKeyInMapping(json_key));
        }
    }
    else
    {
        loginf << "ASTERIXJSONParserDetailWidget: createNewDBVariableSlot: reject";
    }

//    if (dialog.variableEdited())
//    {
//        if (dialog.variable().name() != current_var_name) // renaming done
//        {
//            if (entry_type_ == ASTERIXJSONParser::EntryType::ExistingMapping) // change in mapping
//                parser_.mapping(entry_index_).dboVariableName(dialog.variable().name());
//        }

//        parser_.doMappingChecks();

//        if (entry_type_ == ASTERIXJSONParser::EntryType::ExistingMapping)
//            parser_.selectMapping(entry_index_); // ok since order in mappings is the same
//        else if (entry_type_ == ASTERIXJSONParser::EntryType::UnmappedDBOVariable)
//            parser_.selectUnmappedDBOVariable(dialog.variable().name()); // search for new name
//    }

//    loginf << "ASTERIXJSONParserDetailWidget: editDBVariableSlot: done";

}

void ASTERIXJSONParserDetailWidget::deleteDBVariableSlot()
{
    loginf << "ASTERIXJSONParserDetailWidget: deleteDBVariableSlot";

    assert (has_current_entry_);
    assert (entry_type_ == ASTERIXJSONParser::EntryType::ExistingMapping
            || entry_type_ == ASTERIXJSONParser::EntryType::UnmappedDBOVariable);
    assert (dbo_var_sel_);

    assert (dbo_var_sel_->hasVariable());


    string dbovar_name = dbo_var_sel_->selectedVariable().name();

    loginf << "ASTERIXJSONParserDetailWidget: deleteDBVariableSlot: deleting var '" << dbovar_name << "'";

    // delete variable
    assert (parser_.dbObject().hasVariable(dbovar_name));
    parser_.dbObject().deleteVariable(dbovar_name);loginf << "ASTERIXJSONParserDetailWidget: deleteDBVariableSlot";


    if (entry_type_ == ASTERIXJSONParser::EntryType::ExistingMapping)
    {
        // remove from mapping
        parser_.mapping(entry_index_).dboVariableName("");

        parser_.doMappingChecks();

        parser_.selectMapping(entry_index_);
    }
    else // deleted unmapped dbovar, clear widget
    {
        parser_.doMappingChecks();

        // clear widget
        has_current_entry_ = false;

        setting_new_content_ = true;

        active_check_->setDisabled(true);
        active_check_->setChecked(false);

        showJSONKey("", false);
        unit_sel_->clear();
        data_format_widget_->clear();
        showDBOVariable("");

        delete_mapping_button_->setHidden(true);
        new_dbovar_button_->setHidden(true);
        dbovar_edit_button_->setHidden(true);
        dbovar_delete_button_->setHidden(true);

        setting_new_content_ = false;
    }
}
void ASTERIXJSONParserDetailWidget::editDBVariableSlot()
{
    loginf << "ASTERIXJSONParserDetailWidget: editDBVariableSlot";

    assert (has_current_entry_);
    assert (entry_type_ == ASTERIXJSONParser::EntryType::ExistingMapping
            || entry_type_ == ASTERIXJSONParser::EntryType::UnmappedDBOVariable);
    assert (dbo_var_sel_);

    assert (dbo_var_sel_->hasVariable());

    DBOVariableEditDialog dialog (dbo_var_sel_->selectedVariable(), this);

    string current_var_name = dbo_var_sel_->selectedVariable().name();

    dialog.exec();

    if (dialog.variableEdited())
    {
        if (dialog.variable().name() != current_var_name) // renaming done
        {
            if (entry_type_ == ASTERIXJSONParser::EntryType::ExistingMapping) // change in mapping
                parser_.mapping(entry_index_).dboVariableName(dialog.variable().name());
        }

        parser_.doMappingChecks();

        if (entry_type_ == ASTERIXJSONParser::EntryType::ExistingMapping)
            parser_.selectMapping(entry_index_); // ok since order in mappings is the same
        else if (entry_type_ == ASTERIXJSONParser::EntryType::UnmappedDBOVariable)
            parser_.selectUnmappedDBOVariable(dialog.variable().name()); // search for new name
    }

    loginf << "ASTERIXJSONParserDetailWidget: editDBVariableSlot: done";
}

void ASTERIXJSONParserDetailWidget::deleteMappingSlot()
{
    assert (delete_mapping_button_);
    loginf << "ASTERIXJSONParserDetailWidget: mappingActionSlot: delete";

    assert (has_current_entry_);
    assert (entry_type_ == ASTERIXJSONParser::EntryType::ExistingMapping);

    assert (parser_.hasMapping(entry_index_));
    parser_.removeMapping(entry_index_);

    parser_.doMappingChecks();

    // clear widget
    has_current_entry_ = false;

    setting_new_content_ = true;

    active_check_->setDisabled(true);
    active_check_->setChecked(false);

    showJSONKey("", false);
    unit_sel_->clear();
    data_format_widget_->clear();
    showDBOVariable("");

    delete_mapping_button_->setHidden(true);
    new_dbovar_button_->setHidden(true);
    dbovar_edit_button_->setHidden(true);
    dbovar_delete_button_->setHidden(true);

    setting_new_content_ = false;

}
