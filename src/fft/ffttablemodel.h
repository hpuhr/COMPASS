#pragma once

#include <QAbstractItemModel>
#include <QIcon>

class FFTManager;
class FFTsConfigurationDialog;

class FFTTableModel : public QAbstractItemModel
{
public:
    FFTTableModel(FFTManager& ds_man, FFTsConfigurationDialog& dialog);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    std::string getNameOf (const QModelIndex& index);

    QModelIndex fftIndex(const std::string& fft_name); // returns row
    void updateFFT(const std::string& fft_name);
    void beginModelReset();
    void endModelReset();

protected:
    FFTManager& fft_man_;
    FFTsConfigurationDialog& dialog_;

    QStringList table_columns_ {"Name", "In DB", "In Cfg"};

    QIcon db_icon_;
    QIcon config_icon_;
};

