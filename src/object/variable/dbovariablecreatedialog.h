#ifndef DBOVARIABLE_CREATEDIALOG_H
#define DBOVARIABLE_CREATEDIALOG_H

#include "dbovariable.h"

#include <QDialog>

class DBOVariable;
class DBOVariableDataTypeComboBox;
class UnitSelectionWidget;
class StringRepresentationComboBox;

class QLineEdit;
class QCheckBox;
class QPushButton;
class QTextEdit;

class DBOVariableCreateDialog : public QDialog
{
    Q_OBJECT

public slots:
    void nameChangedSlot(const QString& name);
    void shortNameChangedSlot(const QString& name);
    void commentChangedSlot();
    void dbColumnChangedSlot(const QString& name);

    void doneSlot();

public:
    DBOVariableCreateDialog(DBObject& object, QWidget* parent = nullptr, Qt::WindowFlags f = 0);

protected:
    DBObject& object_;

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

    std::string name_;
    std::string short_name_;

    PropertyDataType data_type_;
    std::string dimension_;
    std::string unit_;

    DBOVariable::Representation representation_;
    std::string representation_str_;

    std::string description_;
    std::string db_column_name_;

};


#endif // DBOVARIABLE_CREATEDIALOG_H
