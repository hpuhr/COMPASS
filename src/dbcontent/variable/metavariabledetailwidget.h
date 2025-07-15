#pragma once

#include <QWidget>

class DBContentManager;


class QLineEdit;
class QTextEdit;
class QPushButton;

namespace dbContent
{

class MetaVariable;
class VariableSelectionWidget;

class MetaVariableDetailWidget : public QWidget
{
    Q_OBJECT

public slots:
    void nameEditedSlot();
    void variableChangedSlot();

    void deleteVariableSlot();

public:
    MetaVariableDetailWidget(DBContentManager& dbo_man, QWidget* parent = nullptr);

    void show (MetaVariable& meta_var);
    void clear();

private:
    DBContentManager& dbo_man_;

    bool has_current_entry_ {false};
    MetaVariable* meta_var_ {nullptr};

    QLineEdit* name_edit_{nullptr};
    QTextEdit* description_edit_{nullptr};

    std::map<std::string, VariableSelectionWidget*> selection_widgets_; // db content -> var select

    QPushButton* delete_button_ {nullptr};
};

}

