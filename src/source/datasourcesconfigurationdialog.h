#ifndef DATASOURCESCONFIGURATIONDIALOG_H
#define DATASOURCESCONFIGURATIONDIALOG_H

#include <QDialog>

class DataSourceManager;
class DataSourceTableModel;
class DataSourceEditWidget;

class QTableView;
class QSortFilterProxyModel;

class DataSourcesConfigurationDialog : public QDialog
{
    Q_OBJECT

signals:
    void doneSignal();

public slots:

    void currentRowChanged(const QModelIndex& current, const QModelIndex& previous);

    void importClickedSlot();
    void deleteAllClickedSlot();
    void exportClickedSlot();
    void doneClickedSlot();

public:
    DataSourcesConfigurationDialog(DataSourceManager& ds_man);

    void updateDataSource(unsigned int ds_id);
    void beginResetModel();
    void endResetModel();

protected:
    DataSourceManager& ds_man_;

    QTableView* table_view_{nullptr};
    QSortFilterProxyModel* proxy_model_{nullptr};
    DataSourceTableModel* table_model_;

    DataSourceEditWidget* edit_widget_ {nullptr};
};

#endif // DATASOURCESCONFIGURATIONDIALOG_H
