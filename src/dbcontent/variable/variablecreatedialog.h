#ifndef DBCONTENT_VARIABLECREATEDIALOG_H
#define DBCONTENT_VARIABLECREATEDIALOG_H

#include "dbcontent/variable/variable.h"

#include <QDialog>

class UnitSelectionWidget;
class StringRepresentationComboBox;

class QLineEdit;
class QCheckBox;
class QPushButton;
class QTextEdit;

namespace dbContent
{

class Variable;
class VariableDataTypeComboBox;

class VariableCreateDialog : public QDialog
{
    Q_OBJECT

public slots:
    void nameChangedSlot(const QString& name);
    void shortNameChangedSlot(const QString& name);
    void commentChangedSlot();
    void dbColumnChangedSlot(const QString& name);

public:
    VariableCreateDialog(DBContent& object, const std::string name="",
                            const std::string description="",
                            QWidget* parent = nullptr, Qt::WindowFlags f = 0);

    std::string name() const;

    std::string shortName() const;

    std::string dataTypeStr() const;

    std::string dimension() const;

    std::string unit() const;

    std::string  representationStr() const;

    std::string description() const;

    std::string dbColumnName() const;

protected:
    DBContent& object_;

    QLineEdit* name_edit_ {nullptr};
    QLineEdit* short_name_edit_ {nullptr};
    QTextEdit* description_edit_ {nullptr};
    dbContent::VariableDataTypeComboBox* type_combo_ {nullptr};
    UnitSelectionWidget* unit_sel_ {nullptr};
    StringRepresentationComboBox* representation_box_ {nullptr};
    QLineEdit* db_column_edit_ {nullptr};

    QPushButton* cancel_button_ {nullptr};
    QPushButton* ok_button_ {nullptr};

    std::string name_;
    bool name_ok_ {false};
    std::string short_name_;

    PropertyDataType data_type_ {PropertyDataType::BOOL};
    std::string data_type_str_ {"BOOL"};
    std::string dimension_;
    std::string unit_;

    Variable::Representation representation_ {Variable::Representation::STANDARD};
    std::string representation_str_ {"STANDARD"};

    std::string description_;
    std::string db_column_name_;
    bool db_column_name_ok_ {false};

    void checkSettings();
};

}

#endif // DBCONTENT_VARIABLECREATEDIALOG_H
