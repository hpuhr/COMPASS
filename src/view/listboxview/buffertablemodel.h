#ifndef BUFFERTABLEMODEL_H
#define BUFFERTABLEMODEL_H

#include <memory>

#include <QAbstractTableModel>

class Buffer;
class DBObject;

class BufferTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    BufferTableModel(QObject *parent, DBObject &object);
    virtual ~BufferTableModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    void clearData ();
    void setData (std::shared_ptr <Buffer> buffer);

protected:
    DBObject &object_;
    std::shared_ptr <Buffer> buffer_;
};

#endif // BUFFERTABLEMODEL_H
