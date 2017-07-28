#include "buffertablemodel.h"

#include "buffer.h"
#include "dbobject.h"

BufferTableModel::BufferTableModel(QObject *parent, DBObject &object)
    :QAbstractTableModel(parent), object_(object)
{
}

BufferTableModel::~BufferTableModel()
{
    buffer_ = nullptr;
}

int BufferTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    if (buffer_)
    {
        logdbg << "BufferTableModel: rowCount: " << buffer_->size();
        return buffer_->size();
    }
    else
    {
        logdbg << "BufferTableModel: rowCount: 0";
        return 0;
    }
}

int BufferTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    if (buffer_)
    {
        logdbg << "BufferTableModel: columnCount: " << buffer_->properties().size();
        return buffer_->properties().size();
    }
    else
    {
        logdbg << "BufferTableModel: columnCount: 0";
        return 0;
    }
}

QVariant BufferTableModel::data(const QModelIndex &index, int role) const
{
    logdbg << "BufferTableModel: data: row " << index.row()-1 << " col " << index.column()-1;
    if (role == Qt::DisplayRole)
    {
        assert (buffer_);

        bool null=false;
        std::string value_str;

        unsigned int row = index.row(); // indexes start at 0 in this family
        unsigned int col = index.column();

        const PropertyList &properties = buffer_->properties();

        assert (row < buffer_->size());
        assert (col < properties.size());

        PropertyDataType data_type = properties.at(col).dataType();
        value_str = "NULL";

        if (data_type == PropertyDataType::BOOL)
        {
            null = buffer_->getBool(properties.at(col).name()).isNone(row);
            if (!null)
                value_str = buffer_->getBool(properties.at(col).name()).getAsRepresentationString(row);
        }
        else if (data_type == PropertyDataType::CHAR)
        {
            null = buffer_->getChar(properties.at(col).name()).isNone(row);
            if (!null)
                value_str = buffer_->getChar(properties.at(col).name()).getAsRepresentationString(row);
        }
        else if (data_type == PropertyDataType::UCHAR)
        {
            null = buffer_->getUChar(properties.at(col).name()).isNone(row);
            if (!null)
                value_str = buffer_->getUChar(properties.at(col).name()).getAsRepresentationString(row);
        }
        else if (data_type == PropertyDataType::INT)
        {
            null = buffer_->getInt(properties.at(col).name()).isNone(row);
            if (!null)
                value_str = buffer_->getInt(properties.at(col).name()).getAsRepresentationString(row);
        }
        else if (data_type == PropertyDataType::UINT)
        {
            null = buffer_->getUInt(properties.at(col).name()).isNone(row);
            if (!null)
                value_str = buffer_->getUInt(properties.at(col).name()).getAsRepresentationString(row);
        }
        else if (data_type == PropertyDataType::LONGINT)
        {
            null = buffer_->getLongInt(properties.at(col).name()).isNone(row);
            if (!null)
                value_str = buffer_->getLongInt(properties.at(col).name()).getAsRepresentationString(row);
        }
        else if (data_type == PropertyDataType::ULONGINT)
        {
            null = buffer_->getULongInt(properties.at(col).name()).isNone(row);
            if (!null)
                value_str = buffer_->getULongInt(properties.at(col).name()).getAsRepresentationString(row);
        }
        else if (data_type == PropertyDataType::FLOAT)
        {
            null = buffer_->getFloat(properties.at(col).name()).isNone(row);
            if (!null)
                value_str = buffer_->getFloat(properties.at(col).name()).getAsRepresentationString(row);
        }
        else if (data_type == PropertyDataType::DOUBLE)
        {
            null = buffer_->getDouble(properties.at(col).name()).isNone(row);
            if (!null)
                value_str = buffer_->getDouble(properties.at(col).name()).getAsRepresentationString(row);
        }
        else if (data_type == PropertyDataType::STRING)
        {
            null = buffer_->getString(properties.at(col).name()).isNone(row);
            if (!null)
                value_str = buffer_->getString(properties.at(col).name()).getAsString(row);
        }
        else
            throw std::domain_error ("BufferTableWidget: show: unknown property data type");

        if (null)
            return QVariant();
        else
            return QString (value_str.c_str());
    }
    return QVariant();
}

QVariant BufferTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        logdbg << "BufferTableModel: headerData: section " << section;
        const PropertyList &properties = buffer_->properties();
        unsigned int col = section;

        assert (col < properties.size());
        return QString (properties.at(col).name().c_str());
    }
    else if(orientation == Qt::Vertical)
        return section;

    return QVariant();
}

void BufferTableModel::clearData ()
{
    beginResetModel();

    buffer_=nullptr;

    endResetModel();

}

void BufferTableModel::setData (std::shared_ptr <Buffer> buffer)
{
    assert (buffer);
    beginResetModel();

    buffer_=buffer;

    endResetModel();
}

void BufferTableModel::saveAsCSV (const std::string &file_name, bool overwrite)
{
    loginf << "BufferTableModel: saveAsCSV: into filename " << file_name << " overwrite " << overwrite;

}
