#ifndef DBCONTENT_METAVARIABLECONFIGURATIONDIALOG_H
#define DBCONTENT_METAVARIABLECONFIGURATIONDIALOG_H

#include <QDialog>
#include <QListWidget>

class DBContentManager;

class QSplitter;
class QPushButton;

namespace dbContent
{

class MetaVariableDetailWidget;

class MetaVariableConfigurationDialog : public QDialog
{
    Q_OBJECT

signals:
    void okSignal();

public slots:
    void okClickedSlot();
    void itemSelectedSlot(const QString& text);

    void addAllMetaVariablesSlot();

public:
    MetaVariableConfigurationDialog(DBContentManager& dbo_man);
    virtual ~MetaVariableConfigurationDialog();

    void updateList();
    void selectMetaVariable (const std::string& name);
    void clearDetails();

protected:
    DBContentManager& dbo_man_;

    QSplitter* splitter_{nullptr};

    QListWidget* list_widget_ {nullptr};

    MetaVariableDetailWidget* detail_widget_{nullptr};

};

}

#endif // DBCONTENT_METAVARIABLECONFIGURATIONDIALOG_H
