#include "asterixjsonparserdetailwidget.h"

#include "asterixjsonparser.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QFormLayout>

using namespace std;

ASTERIXJSONParserDetailWidget::ASTERIXJSONParserDetailWidget(ASTERIXJSONParser& parser, QWidget *parent)
    : QWidget(parent), parser_(parser)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    QFormLayout* form_layout = new QFormLayout;
    form_layout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);

    info_label_ = new QLabel();
    form_layout->addRow("Info", info_label_);

    form_layout->addRow(new QLabel("ASTERIX"));

    json_key_label_ = new QLabel();
    form_layout->addRow("JSON Key", json_key_label_);

    form_layout->addRow(new QLabel("DBOVariable"));

    dbo_var_label_ = new QLabel();
    form_layout->addRow("Name", dbo_var_label_);

    main_layout->addLayout(form_layout);

    setLayout(main_layout);
}

void ASTERIXJSONParserDetailWidget::currentIndexChangedSlot (unsigned int index)
{
    loginf << "ASTERIXJSONParserDetailWidget: currentIndexChangedSlot: index " << index;

    assert (index < parser_.totalEntrySize());

    assert (info_label_);
    assert (json_key_label_);
    assert (dbo_var_label_);


    if (index < parser_.dataMappings().size()) // is actual mapping
    {
        JSONDataMapping& mapping = parser_.dataMappings().at(index);

        loginf << "ASTERIXJSONParserDetailWidget: currentIndexChangedSlot: mapping " << index
               << " key '" << mapping.jsonKey() << "'";

        if (!parser_.existsJSONKeyInCATInfo(mapping.jsonKey()))
            info_label_->setText("JSON Key not found in ASTERIX Info");
        else
            info_label_->setText("Existing Mapping");

        json_key_label_->setText(mapping.jsonKey().c_str());
        dbo_var_label_->setText(mapping.dboVariableName().c_str());

        return;
    }

    index -= parser_.dataMappings().size();

    if (index < parser_.notAddedJSONKeys().size())
    {
        info_label_->setText("Unmapped JSON Key");

        string key = parser_.notAddedJSONKeys().at(index);

        loginf << "ASTERIXJSONParserDetailWidget: currentIndexChangedSlot: not added JSON " << index
               << " key '" << key << "'";

        json_key_label_->setText(key.c_str());
        dbo_var_label_->setText("");

        return;
    }

    index -= parser_.notAddedJSONKeys().size();

    assert (index < parser_.notAddedDBOVariables().size());

    info_label_->setText("Unmapped DBO Variable");
    string dbovar = parser_.notAddedDBOVariables().at(index);

    loginf << "ASTERIXJSONParserDetailWidget: currentIndexChangedSlot: not added dbovar " << index
           << " key '" << dbovar << "'";

    json_key_label_->setText("");
    dbo_var_label_->setText(dbovar.c_str());

}

