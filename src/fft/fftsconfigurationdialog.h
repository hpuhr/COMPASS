#pragma once

#include <QDialog>

#include <memory>

class FFTManager;
class FFTTableModel;
class FFTEditWidget;

class QTableView;
class QSortFilterProxyModel;

class FFTsConfigurationDialog : public QDialog
{
    Q_OBJECT

signals:
    void doneSignal();

public slots:

    void currentRowChanged(const QModelIndex& current, const QModelIndex& previous);

    void newFFTClickedSlot();

    void importClickedSlot();
    void deleteAllClickedSlot();
    void exportClickedSlot();
    void doneClickedSlot();

public:
    FFTsConfigurationDialog(FFTManager& ds_man);

    void updateFFT(const std::string& name);
    void beginResetModel();
    void endResetModel();

protected:
    FFTManager& fft_man_;

    QTableView* table_view_{nullptr};
    QSortFilterProxyModel* proxy_model_{nullptr};
    FFTTableModel* table_model_;

    FFTEditWidget* edit_widget_ {nullptr};
};

