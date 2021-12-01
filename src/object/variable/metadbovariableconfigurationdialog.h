#ifndef METADBOVARIABLECONFIGURATIONDIALOG_H
#define METADBOVARIABLECONFIGURATIONDIALOG_H

#include <QDialog>

class DBObjectManager;

class QPushButton;

class MetaDBOVariableConfigurationDialog : public QDialog
{
    Q_OBJECT

signals:
    void okSignal();

public slots:
    void okClickedSlot();

public:
    MetaDBOVariableConfigurationDialog(DBObjectManager& dbo_man);

protected:
    DBObjectManager& dbo_man_;
};

#endif // METADBOVARIABLECONFIGURATIONDIALOG_H
