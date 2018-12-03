#include "jsonobjectparserwidget.h"
#include "jsonobjectparser.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>


JSONObjectParserWidget::JSONObjectParserWidget(JSONObjectParser& parser, QWidget *parent)
    : QWidget(parent), parser_ (&parser)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout *main_layout = new QVBoxLayout ();

    std::string tmp ="JSON Object Parser " + parser_->dbObjectName();
    QLabel *main_label = new QLabel (tmp.c_str());
    main_label->setFont (font_bold);
    main_layout->addWidget (main_label);

    QGridLayout* grid = new QGridLayout ();

    int row = 0;

    grid->addWidget(new QLabel("JSON Container Key"), row, 0);

    json_container_key_edit_ = new QLineEdit ();
    connect(json_container_key_edit_, SIGNAL(textEdited(const QString&)), this, SLOT(jsonContainerKeyChangedSlot()));
    grid->addWidget(json_container_key_edit_, row++, 1);

    grid->addWidget(new QLabel("JSON Key"));

    json_key_edit_ = new QLineEdit ();
    connect(json_key_edit_, SIGNAL(textEdited(const QString&)), this, SLOT(jsonKeyChangedSlot()));
    grid->addWidget(json_key_edit_, row++, 1);

    grid->addWidget(new QLabel("JSON Value"));

    json_value_edit_ = new QLineEdit ();
    connect(json_value_edit_, SIGNAL(textEdited(const QString&)), this, SLOT(jsonValueChangedSlot()));
    grid->addWidget(json_value_edit_, row++, 1);

    grid->addWidget(new QLabel("Override Key Variable"));

    override_key_variable_check_ = new QCheckBox ();
    connect(override_key_variable_check_, SIGNAL(stateChanged(int)), this, SLOT(overrideKeyVariabledChangedSlot()));
    grid->addWidget(override_key_variable_check_, row++, 1);

    grid->addWidget(new QLabel("Override Data Source"));

    override_data_source_check_ = new QCheckBox ();
    connect(override_data_source_check_, SIGNAL(stateChanged(int)), this, SLOT(overrideDataSourceChangedSlot()));
    grid->addWidget(override_data_source_check_, row++, 1);

    grid->addWidget(new QLabel("Data Source Variable"));

    data_source_variable_name_edit_ = new QLineEdit ();
    connect(data_source_variable_name_edit_, SIGNAL(textEdited(const QString&)),
            this, SLOT(dataSourceVariableChangedSlot()));
    grid->addWidget(data_source_variable_name_edit_, row++, 1);

    main_layout->addLayout(grid);
    setLayout (main_layout);

    update ();

    show();
}

void JSONObjectParserWidget::update ()
{
    assert (parser_);
    assert (json_container_key_edit_);
    assert (json_key_edit_);
    assert (json_value_edit_);
    assert (override_key_variable_check_);
    assert (override_data_source_check_);
    assert (data_source_variable_name_edit_);

    json_container_key_edit_->setText(parser_->JSONContainerKey().c_str());
    json_key_edit_->setText(parser_->JSONKey().c_str());
    json_value_edit_->setText(parser_->JSONValue().c_str());

    override_key_variable_check_->setChecked(parser_->overrideKeyVariable());

    override_data_source_check_->setChecked(parser_->overrideDataSource());
    data_source_variable_name_edit_->setText(parser_->dataSourceVariableName().c_str());
}

void JSONObjectParserWidget::setParser (JSONObjectParser& parser)
{
    parser_ = &parser;
}

void JSONObjectParserWidget::jsonContainerKeyChangedSlot()
{
    assert (parser_);
    assert (json_container_key_edit_);

    parser_->JSONContainerKey(json_container_key_edit_->text().toStdString());
}

void JSONObjectParserWidget::jsonKeyChangedSlot ()
{
    assert (parser_);
    assert (json_key_edit_);

    parser_->JSONKey(json_key_edit_->text().toStdString());
}

void JSONObjectParserWidget::jsonValueChangedSlot ()
{
    assert (parser_);
    assert (json_value_edit_);

    parser_->JSONValue(json_value_edit_->text().toStdString());
}

void JSONObjectParserWidget::overrideKeyVariabledChangedSlot ()
{
    assert (parser_);
    assert (override_key_variable_check_);

    parser_->overrideKeyVariable(override_key_variable_check_->isChecked());
}

void JSONObjectParserWidget::overrideDataSourceChangedSlot ()
{
    assert (parser_);
    assert (override_data_source_check_);

    parser_->overrideDataSource(override_data_source_check_->isChecked());
}

void JSONObjectParserWidget::dataSourceVariableChangedSlot ()
{
    assert (parser_);
    assert (data_source_variable_name_edit_);

    parser_->dataSourceVariableName(data_source_variable_name_edit_->text().toStdString());
}
