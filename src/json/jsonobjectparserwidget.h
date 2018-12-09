#ifndef JSONOBJECTPARSERWIDGET_H
#define JSONOBJECTPARSERWIDGET_H

#include <QWidget>

class JSONObjectParser;
class QLineEdit;
class QCheckBox;
class QGridLayout;
class FormatSelectionWidget;

class JSONObjectParserWidget : public QWidget
{
    Q_OBJECT
public slots:
    void jsonContainerKeyChangedSlot();
    void jsonKeyChangedSlot ();
    void jsonValueChangedSlot ();

    void overrideKeyVariabledChangedSlot ();

    void overrideDataSourceChangedSlot ();
    void dataSourceVariableChangedSlot ();

    void addNewMappingSlot();

    void mappingActiveChangedSlot();
    void mappingKeyChangedSlot();
    void mappingDBOVariableChangedSlot();
    void mappingMandatoryChangedSlot();
    void mappingDeleteSlot();

public:
    explicit JSONObjectParserWidget(JSONObjectParser& parser, QWidget *parent = nullptr);

    void setParser (JSONObjectParser& parser);
private:
    JSONObjectParser* parser_ {nullptr};

    QLineEdit* json_container_key_edit_ {nullptr};  // location of container with target report data
    QLineEdit* json_key_edit_ {nullptr}; // * for all
    QLineEdit* json_value_edit_ {nullptr};

    QCheckBox* override_key_variable_check_ {nullptr};

    QCheckBox* override_data_source_check_ {nullptr};
    QLineEdit* data_source_variable_name_edit_ {nullptr};

    QGridLayout* mappings_grid_ {nullptr};

    std::map<unsigned int, FormatSelectionWidget*> format_selections_;

    void update ();

    void updateMappingsGrid();
};

#endif // JSONOBJECTPARSERWIDGET_H
