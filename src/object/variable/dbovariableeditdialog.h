#ifndef DBOVARIABLE_EDITDIALOG_H
#define DBOVARIABLE_EDITDIALOG_H

#include <QDialog>

class DBOVariable;
class DBOVariableDataTypeComboBox;
class UnitSelectionWidget;
class StringRepresentationComboBox;

class QLineEdit;
class QCheckBox;
class QPushButton;
class QTextEdit;

class DBOVariableEditDialog : public QDialog
{
    Q_OBJECT

public slots:
    void nameChangedSlot(const QString& name);
    void shortNameChangedSlot(const QString& name);
    void commentChangedSlot();
    void dbColumnChangedSlot(const QString& name);

    void doneSlot();

public:
    DBOVariableEditDialog(DBOVariable& variable, QWidget* parent = nullptr, Qt::WindowFlags f = 0);

    bool variableEdited() const;

    DBOVariable &variable() const;

protected:
    DBOVariable& variable_;

    QLineEdit* name_edit_ {nullptr};
    QLineEdit* short_name_edit_ {nullptr};
    QTextEdit* description_edit_ {nullptr};
    DBOVariableDataTypeComboBox* type_combo_ {nullptr};
    UnitSelectionWidget* unit_sel_ {nullptr};
    StringRepresentationComboBox* representation_box_ {nullptr};
    QLineEdit* db_column_edit_ {nullptr};

    QPushButton* done_button_ {nullptr};

    std::string invalid_bg_str_;
    std::string valid_bg_str_;

    bool variable_edited_ {false};
};


#endif // DBOVARIABLE_EDITDIALOG_H
