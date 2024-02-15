#include "ffttablemodel.h"
#include "fftmanager.h"
#include "logger.h"
#include "files.h"
#include "fft/dbfft.h"
#include "fft/configurationfft.h"

using namespace Utils;
using namespace std;

FFTTableModel::FFTTableModel(FFTManager& ds_man, FFTsConfigurationDialog& dialog)
    : fft_man_(ds_man), dialog_(dialog)
{
    db_icon_ = QIcon(Files::getIconFilepath("db.png").c_str());
    config_icon_ = QIcon(Files::getIconFilepath("configuration.png").c_str());
}

int FFTTableModel::rowCount(const QModelIndex& parent) const
{
    return fft_man_.getAllFFTNames().size();
}

int FFTTableModel::columnCount(const QModelIndex& parent) const
{
    return table_columns_.size();
}

QVariant FFTTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
    {
        logdbg << "FFTTableModel: data: display role: row " << index.row() << " col " << index.column();

        assert (index.row() >= 0);
        assert (index.row() < fft_man_.getAllFFTNames().size());

        string fft_name = fft_man_.getAllFFTNames().at(index.row());

        logdbg << "FFTTableModel: data: got fft_name " << fft_name;

        assert (index.column() < table_columns_.size());
        std::string col_name = table_columns_.at(index.column()).toStdString();

        if (fft_man_.hasDBFFT(fft_name))
        {
            DBFFT& fft = fft_man_.dbFFT(fft_name);

            if (col_name == "Name")
                return fft.name().c_str();
            else
                return QVariant();
        }
        else // cfg only
        {
            assert (fft_man_.hasConfigFFT(fft_name));

            ConfigurationFFT& fft = fft_man_.configFFT(fft_name);

            if (col_name == "Name")
                return fft.name().c_str();
            else
                return QVariant();
        }
    }
    case Qt::DecorationRole:
    {
        assert (index.row() >= 0);
        assert (index.row() < fft_man_.getAllFFTNames().size());

        string fft_name = fft_man_.getAllFFTNames().at(index.row());

        logdbg << "FFTTableModel: data: got fft_name " << fft_name;

        assert (index.column() < table_columns_.size());
        std::string col_name = table_columns_.at(index.column()).toStdString();

        if (col_name != "In DB" && col_name != "In Cfg")
            return QVariant();

        if (col_name == "In DB" && fft_man_.hasDBFFT(fft_name))
            return db_icon_;
        else if (col_name == "In Cfg" && fft_man_.hasConfigFFT(fft_name))
            return config_icon_;
        else
            return QVariant();

    }
    default:
    {
        return QVariant();
    }
    }
}

QVariant FFTTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        assert (section < table_columns_.size());
        return table_columns_.at(section);
    }

    return QVariant();
}

QModelIndex FFTTableModel::index(int row, int column, const QModelIndex& parent) const
{
    return createIndex(row, column);
}

QModelIndex FFTTableModel::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

Qt::ItemFlags FFTTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    assert (index.column() < table_columns_.size());

    return QAbstractItemModel::flags(index);
}

std::string FFTTableModel::getNameOf (const QModelIndex& index)
{
    assert (index.isValid());

    assert (index.row() >= 0);
    assert (index.row() < fft_man_.getAllFFTNames().size());

    return fft_man_.getAllFFTNames().at(index.row());
}

QModelIndex FFTTableModel::fftIndex(const std::string& fft_name)
{
    loginf << "FFTTableModel: selectFFT: fft_name " << fft_name;

    auto fft_names = fft_man_.getAllFFTNames();

    auto itr = std::find(fft_names.begin(), fft_names.end(), fft_name);
    assert (itr != fft_names.end());

    unsigned int row = std::distance(fft_names.begin(), itr);

    return index(row, 0);
}

void FFTTableModel::updateFFT(const std::string& fft_name)
{
    loginf << "FFTTableModel: updateFFT: fft_name " << fft_name;

    auto fft_names = fft_man_.getAllFFTNames();

    auto itr = std::find(fft_names.begin(), fft_names.end(), fft_name);
    assert (itr != fft_names.end());

    unsigned int row = std::distance(fft_names.begin(), itr);

    emit dataChanged(index(row, 0), index(row, table_columns_.size()-1), QVector<int>(Qt::DisplayRole));
}

void FFTTableModel::beginModelReset()
{
    beginResetModel();
}
void FFTTableModel::endModelReset()
{
    endResetModel();
}
