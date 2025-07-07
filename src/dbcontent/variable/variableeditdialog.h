#ifndef DBCONTENT_VARIABLEEDITDIALOG_H
#define DBCONTENT_VARIABLEEDITDIALOG_H

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

class VariableEditDialog : public QDialog
{
    Q_OBJECT

public slots:
    void nameChangedSlot(const QString& name);
    void shortNameChangedSlot(const QString& name);
    void commentChangedSlot();
    void dbColumnChangedSlot(const QString& name);

    void doneSlot();

public:
    VariableEditDialog(Variable& variable, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    bool variableEdited() const;

    Variable &variable() const;

protected:
    Variable& variable_;

    bool expert_mode_ {false}; // COMPASS expert mode

    QLineEdit* name_edit_ {nullptr};
    QLineEdit* short_name_edit_ {nullptr};
    QTextEdit* description_edit_ {nullptr};
    VariableDataTypeComboBox* type_combo_ {nullptr};
    UnitSelectionWidget* unit_sel_ {nullptr};
    StringRepresentationComboBox* representation_box_ {nullptr};
    QLineEdit* db_column_edit_ {nullptr};

    QPushButton* done_button_ {nullptr};

    bool variable_edited_ {false};
};

}

#endif // DBCONTENT_VARIABLEEDITDIALOG_H
