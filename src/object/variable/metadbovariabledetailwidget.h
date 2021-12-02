#ifndef METADBOVARIABLEDETAILWIDGET_H
#define METADBOVARIABLEDETAILWIDGET_H

#include <QWidget>

class DBObjectManager;
class MetaDBOVariable;
class DBOVariableSelectionWidget;

class QLineEdit;
class QTextEdit;
class QPushButton;

class MetaDBOVariableDetailWidget : public QWidget
{
    Q_OBJECT

public slots:
    void nameEditedSlot();
    void variableChangedSlot();

    void deleteVariableSlot();

public:
    MetaDBOVariableDetailWidget(DBObjectManager& dbo_man, QWidget* parent = nullptr);

    void show (MetaDBOVariable& meta_var);
    void clear();

private:
    DBObjectManager& dbo_man_;

    bool has_current_entry_ {false};
    MetaDBOVariable* meta_var_ {nullptr};

    QLineEdit* name_edit_{nullptr};
    QTextEdit* description_edit_{nullptr};

    std::map<std::string, DBOVariableSelectionWidget*> selection_widgets_; // db content -> var select

    QPushButton* delete_button_ {nullptr};
};

#endif // METADBOVARIABLEDETAILWIDGET_H
