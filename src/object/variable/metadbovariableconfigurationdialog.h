#ifndef METADBOVARIABLECONFIGURATIONDIALOG_H
#define METADBOVARIABLECONFIGURATIONDIALOG_H

#include <QDialog>
#include <QListWidget>

class DBObjectManager;
class MetaDBOVariableDetailWidget;

class QSplitter;
class QPushButton;

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
    MetaDBOVariableConfigurationDialog(DBObjectManager& dbo_man);
    virtual ~MetaDBOVariableConfigurationDialog();

    void updateList();

protected:
    DBObjectManager& dbo_man_;

    QSplitter* splitter_{nullptr};

    QListWidget* list_widget_ {nullptr};

    MetaDBOVariableDetailWidget* detail_widget_{nullptr};

};

#endif // METADBOVARIABLECONFIGURATIONDIALOG_H
