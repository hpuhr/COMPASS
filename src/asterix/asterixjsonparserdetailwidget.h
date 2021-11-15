#ifndef ASTERIXJSONPARSERDETAILWIDGET_H
#define ASTERIXJSONPARSERDETAILWIDGET_H

#include "asterixjsonparser.h"

#include <QWidget>

class DBOVariableSelectionWidget;

class UnitSelectionWidget;
class DataTypeFormatSelectionWidget;

class QLabel;
class QLineEdit;
class QCheckBox;
class QPushButton;
class QTextEdit;

class ASTERIXJSONParserDetailWidget : public QWidget
{
    Q_OBJECT

public slots:
    void currentIndexChangedSlot (unsigned int index);

    void mappingActiveChangedSlot();

    void mappingInArrayChangedSlot();
    void mappingAppendChangedSlot();

    void mappingDBOVariableChangedSlot();
    void dboVariableCommentChangedSlot();

    void createNewDBVariableSlot(); // create new dbovar, and mapping if required
    void editDBVariableSlot();
    void deleteDBVariableSlot();

    void deleteMappingSlot();

signals:


public:
    explicit ASTERIXJSONParserDetailWidget(ASTERIXJSONParser& parser, QWidget *parent = nullptr);

private:
    ASTERIXJSONParser& parser_;

    bool has_current_entry_ {false};
    ASTERIXJSONParser::EntryType entry_type_;
    unsigned int entry_index_;

    QLabel* info_label_ {nullptr}; // shows type of mapping, or missing details
    QCheckBox* active_check_ {nullptr};

    QLabel* json_key_label_ {nullptr};
    QLabel* asterix_desc_label_ {nullptr};
    QLabel* asterix_editions_label_ {nullptr};
    QCheckBox* in_array_check_ {nullptr};
    QCheckBox* append_check {nullptr};
    UnitSelectionWidget* unit_sel_ {nullptr};
    DataTypeFormatSelectionWidget* data_format_widget_ {nullptr};

    DBOVariableSelectionWidget* dbo_var_sel_ {nullptr};

    QTextEdit* dbo_var_comment_edit_ {nullptr};

    QPushButton* delete_mapping_button_ {nullptr}; // delete mapping
    QPushButton* new_dbovar_button_ {nullptr}; // create new dbovar, and mapping if required

    QPushButton* dbovar_edit_button_ {nullptr};
    QPushButton* dbovar_delete_button_ {nullptr};

    bool setting_new_content_ {false};

    void showJSONKey (const std::string& key);
    void showDBOVariable (const std::string& var_name, bool mapping_exists=false);

};

#endif // ASTERIXJSONPARSERDETAILWIDGET_H
