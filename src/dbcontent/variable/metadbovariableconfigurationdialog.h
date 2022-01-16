#ifndef METADBOVARIABLECONFIGURATIONDIALOG_H
#define METADBOVARIABLECONFIGURATIONDIALOG_H

#include <QDialog>
#include <QListWidget>

class DBContentManager;

class QSplitter;
class QPushButton;

namespace dbContent
{

class MetaDBOVariableDetailWidget;

class MetaDBOVariableConfigurationDialog : public QDialog
{
    Q_OBJECT

signals:
    void okSignal();

public slots:
    void okClickedSlot();
    void itemSelectedSlot(const QString& text);

    void addAllMetaVariablesSlot();

public:
    MetaDBOVariableConfigurationDialog(DBContentManager& dbo_man);
    virtual ~MetaDBOVariableConfigurationDialog();

    void updateList();
    void selectMetaVariable (const std::string& name);
    void clearDetails();

protected:
    DBContentManager& dbo_man_;

    QSplitter* splitter_{nullptr};

    QListWidget* list_widget_ {nullptr};

    MetaDBOVariableDetailWidget* detail_widget_{nullptr};

};

}

#endif // METADBOVARIABLECONFIGURATIONDIALOG_H
