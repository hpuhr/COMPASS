#pragma once

#include <QList>
#include <QVariant>

class QMenu;
class QWidget;

#include <QItemDelegate>
#include <QStyledItemDelegate>

class ScatterSeriesTreeItemDelegate : public QStyledItemDelegate  // public QItemDelegate
{
    Q_OBJECT

public:
    ScatterSeriesTreeItemDelegate(QObject* parent = 0);
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option,
                     const QModelIndex& index);
    // QSize sizeHint(const QStyleOptionViewItem &  option , const QModelIndex & index) const;
};

class ScatterSeriesTreeItem : public QObject
{
    Q_OBJECT

public:
    explicit ScatterSeriesTreeItem(const std::string& name, ScatterSeriesTreeItem* parent_item = nullptr);
    virtual ~ScatterSeriesTreeItem();

    virtual ScatterSeriesTreeItem* child(int row);
    int childCount() const;
    int columnCount() const;
    virtual QVariant data(int column) const;
    virtual QVariant icon() const;
    int row() const;

    bool hasParentItem() const { return parent_item_ != nullptr; }
    ScatterSeriesTreeItem* parentItem();

    void deleteChildren();

    virtual bool hasMenu() const { return false; }
    virtual void execMenu(const QPoint& pos) {};

    virtual bool canHide() const { return true; }
    virtual bool hidden() const { return hidden_; }
    virtual void hide(bool value) { hidden_ = value; update_hide_impl();}

    bool itemVisible() const;

    const std::string& name() const { return name_; }

    // void moveChildUp(OSGLayerTreeItem* child);
    // void moveChildDown(OSGLayerTreeItem* child);
    // void moveChildToBegin(OSGLayerTreeItem* child);
    // void moveChildToEnd(OSGLayerTreeItem* child);

    // void moveUp();
    // void moveDown();
    // void moveToBegin();
    // void moveToEnd();

    //unsigned int getIndexOf(OSGLayerTreeItem* child);

protected:
    bool hidden_{false};

    std::vector<ScatterSeriesTreeItem*> child_items_;
    std::string name_;
    ScatterSeriesTreeItem* parent_item_{nullptr};

    void appendChild(ScatterSeriesTreeItem* child);
    void removeChild(ScatterSeriesTreeItem* child);

    virtual void update_hide_impl() = 0;

};

