#include "scatterseriestreeitem.h"
#include "scatterseriesmodel.h"
#include "logger.h"

#include <QApplication>

#include <cassert>

#include <QApplication>
#include <QDialog>
#include <QMenu>
#include <QThread>
#include <QtGui>

const unsigned int space = 2;

ScatterSeriesTreeItemDelegate::ScatterSeriesTreeItemDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

void ScatterSeriesTreeItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                const QModelIndex& index) const
{
    logdbg << "ScatterSeriesTreeItemDelegate: paint: r " << index.row() << " c " << index.column();

    if (index.column() == 1)  // only do custom painting in column 0
    {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    QFont font = QApplication::font();
    font.setStyleHint(QFont::TypeWriter);
    QFontMetrics fm(font);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    ScatterSeriesTreeItem* item = static_cast<ScatterSeriesTreeItem*>(index.internalPointer());
    assert(item);

    QRect r = option.rect;  // getting the rect of the cell
    int x, y, w, h;
    x = r.left();     // the X coordinate
    y = r.top();      // the Y coordinate
    w = fm.height();  // button width
    h = fm.height();  // button height

    if (item->canHide())
    {
        QStyleOptionButton cbOpt;
        cbOpt.rect = QRect(x, y, w, h);

        if (item->hidden())
        {
            cbOpt.state |= QStyle::State_Off;
        }
        else
        {
            cbOpt.state |= QStyle::State_On;
        }

        QApplication::style()->drawControl(QStyle::CE_CheckBox, &cbOpt, painter);
        x += w + space;
    }

    logdbg << "ScatterSeriesTreeItemDelegate: paint: 1";

    QStyleOptionButton button;
    button.rect = QRect(x, y, w, h);
    button.state = QStyle::State_Enabled;
    button.features = QStyleOptionButton::None;

    logdbg << "ScatterSeriesTreeItemDelegate: paint: 2";

    QIcon icon = qvariant_cast<QIcon>(index.data(ScatterSeriesModel::DataRole::IconRole));
    logdbg << "ScatterSeriesTreeItemDelegate: paint: 2b";
    button.icon = icon;
    button.iconSize = QSize(w - space, h - space);

    QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter);

    logdbg << "ScatterSeriesTreeItemDelegate: paint: 3";

    QString headerText = qvariant_cast<QString>(index.data(0));

    QSize iconsize(w, h);

    logdbg << "ScatterSeriesTreeItemDelegate: paint: 4";

    QRect headerRect = option.rect;
    QRect iconRect(0, 0, w, h);

    iconRect.setRight(iconsize.width() + w);
    iconRect.setTop(iconRect.top());

    headerRect.setLeft(x + w + space);
    headerRect.setTop(headerRect.top());

    painter->setFont(font);
    QTextOption text_option;
    text_option.setWrapMode(QTextOption::WordWrap);
    painter->drawText(headerRect, headerText, text_option);

    logdbg << "ScatterSeriesTreeItemDelegate: paint: 5";

    painter->restore();

    logdbg << "ScatterSeriesTreeItemDelegate: paint: done";
}

bool ScatterSeriesTreeItemDelegate::editorEvent(QEvent* event, QAbstractItemModel* model,
                                      const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (index.column() == 1)  // only process events for col 0
        return true;

    if (event->type() == QEvent::MouseButtonRelease)
    {
        logdbg << "ScatterSeriesTreeItemDelegate: editorEvent: release";

        QMouseEvent* e = (QMouseEvent*)event;
        int clickX = e->x();
        int clickY = e->y();

        QFont font = QApplication::font();
        QFontMetrics fm(font);

        QRect r = option.rect;  // getting the rect of the cell
        int x, y, w, h;
        x = r.left();     // the X coordinate
        y = r.top();      // the Y coordinate
        w = fm.height();  // button width
        h = fm.height();  // button height

        ScatterSeriesTreeItem* item = static_cast<ScatterSeriesTreeItem*>(index.internalPointer());
        assert(item);

        if (item->canHide())
        {
            if (clickX > x && clickX < x + w && clickY > y && clickY < y + h)
            {
                logdbg << "ScatterSeriesTreeItemDelegate: editorEvent: checkbox";
                item->hide(!item->hidden()); // , true

                return true;
            }
            else  // not in checkbox, shift x for button detection
                x += w + space;
        }

        if (clickX > x && clickX < x + w && clickY > y && clickY < y + h)
        {
            logdbg << "ScatterSeriesTreeItemDelegate: editorEvent: button";
            if (item->hasMenu())
            {
                logdbg << "ScatterSeriesTreeItemDelegate: editorEvent:menu at x " << e->globalX() << " y "
                       << e->globalY();

                item->execMenu(QPoint(e->globalX(), e->globalY()));
            }
        }
    }

    return true;
}


ScatterSeriesTreeItem::ScatterSeriesTreeItem(
    const std::string& name, ScatterSeriesModel& model,
    ScatterSeriesCollection::DataSeries* data_series,
    ScatterSeriesTreeItem* parent_item)
    : name_(name), model_(model), data_series_(data_series), parent_item_(parent_item)
{
    if (data_series_)
    {
        QPixmap pixmap(100,100);
        pixmap.fill(data_series_->color);
        color_icon_ = QIcon(pixmap);
    }

}

ScatterSeriesTreeItem::~ScatterSeriesTreeItem()
{
    child_items_.clear();
}

void ScatterSeriesTreeItem::appendChild(ScatterSeriesTreeItem* item)
{
    logdbg << "ScatterSeriesTreeItem " << name_ << ": appendChild: " << item->name();

    assert (!child_items_.count(item->name()));

    child_items_[item->name()].reset(item);
}

// void ScatterSeriesTreeItem::removeChild(ScatterSeriesTreeItem* item)
// {
//     logdbg << "ScatterSeriesTreeItem " << name_ << ": removeChild: " << item->name();
//     auto it = std::find(child_items_.begin(), child_items_.end(), item);
//     assert(it != child_items_.end());

//     if (it != child_items_.end())
//         child_items_.erase(it);
// }

// void ScatterSeriesTreeItem::updateHiddenImpl()
// {
//     for (auto& child_it : child_items_)
//         child_it.second->updateHidden();
// }

// void ScatterSeriesTreeItem::moveChildUp(ScatterSeriesTreeItem* child)
// {
//     auto it = std::find(child_items_.begin(), child_items_.end(), child);
//     assert(it != child_items_.end());

//     if (it != child_items_.begin())
//         std::iter_swap(it, it - 1);
// }
// void ScatterSeriesTreeItem::moveChildDown(ScatterSeriesTreeItem* child)
// {
//     auto it = std::find(child_items_.begin(), child_items_.end(), child);
//     assert(it != child_items_.end());

//     if (it + 1 != child_items_.end())
//         std::iter_swap(it, it + 1);
// }
// void ScatterSeriesTreeItem::moveChildToBegin(ScatterSeriesTreeItem* child)
// {
//     auto it = std::find(child_items_.begin(), child_items_.end(), child);
//     assert(it != child_items_.end());

//     std::rotate(child_items_.begin(), it, it + 1);
// }
// void ScatterSeriesTreeItem::moveChildToEnd(ScatterSeriesTreeItem* child)
// {
//     auto it = std::find(child_items_.begin(), child_items_.end(), child);
//     assert(it != child_items_.end());

//     if (it == child_items_.end() - 1)
//         return;

//     std::rotate(it, it + 1, child_items_.end());
// }

// void ScatterSeriesTreeItem::moveUp()
// {
//     assert(parent_item_);
//     parent_item_->moveChildUp(this);
// }
// void ScatterSeriesTreeItem::moveDown()
// {
//     assert(parent_item_);
//     parent_item_->moveChildDown(this);
// }
// void ScatterSeriesTreeItem::moveToBegin()
// {
//     assert(parent_item_);

//     parent_item_->moveChildToBegin(this);
// }
// void ScatterSeriesTreeItem::moveToEnd()
// {
//     assert(parent_item_);
//     parent_item_->moveChildToEnd(this);
// }

unsigned int ScatterSeriesTreeItem::getIndexOf(ScatterSeriesTreeItem* child)
{
    // Use std::find_if with a lambda to find the item
    // auto it = std::find_if(
    //     itemMap.begin(),
    //     itemMap.end(),
    //     [targetItem](const std::pair<const std::string, std::unique_ptr<ScatterSeriesTreeItem>>& pair) {
    //         return pair.second.get() == targetItem;
    //     }
    //     );

    auto it = std::find_if(child_items_.begin(), child_items_.end(),
                           [child](const std::pair<const std::string, std::unique_ptr<ScatterSeriesTreeItem>>& x)
                           { return x.second.get() == child;});

    assert(it != child_items_.end());

    return distance(child_items_.begin(), it);
}

void ScatterSeriesTreeItem::clear()
{
    child_items_.clear();
}

ScatterSeriesTreeItem* ScatterSeriesTreeItem::child(int row)
{
    assert(row >= 0);
    assert((unsigned int)row < child_items_.size());

    auto it = std::next(child_items_.begin(), row);
    assert (it != child_items_.end());

    logdbg << "ScatterSeriesTreeItem " << name_ << ": child: " << it->second->name();
    return it->second.get();
}

int ScatterSeriesTreeItem::childCount() const
{
    logdbg << "ScatterSeriesTreeItem " << name_ << ": childCount: " << child_items_.size();
    return child_items_.size();
}

int ScatterSeriesTreeItem::columnCount() const
{
    return 2;
}

QVariant ScatterSeriesTreeItem::data(int column) const
{
    if (column == 0)
        return name_.c_str();
    else if (column == 1)
    {
        if (data_series_)
            return (unsigned int) data_series_->scatter_series.points.size();
        else
            return QVariant();
    }
    else
        assert(false);
}

QVariant ScatterSeriesTreeItem::icon() const
{
    if (data_series_)
        return color_icon_;
    else
        return QVariant();
}

ScatterSeriesTreeItem* ScatterSeriesTreeItem::parentItem() { return parent_item_; }

int ScatterSeriesTreeItem::row() const
{
    if (parent_item_)
        return parent_item_->getIndexOf(const_cast<ScatterSeriesTreeItem*>(this));

    return 0;
}

void ScatterSeriesTreeItem::hide(bool value)
{
    loginf << "ScatterSeriesTreeItem: hide: " << name_ << " hidden " << value;

    hidden_ = value;

    updateHidden();

    emit model_.visibilityChangedSignal();
}

void ScatterSeriesTreeItem::hideAll(bool emit_signal)
{
    hidden_ = true;

    for (auto& child_it : child_items_)
        child_it.second->hideAll(false);

    updateHidden();

    if (emit_signal)
        emit model_.visibilityChangedSignal();
}

void ScatterSeriesTreeItem::updateHidden()
{
    if (data_series_)
        data_series_->visible = !itemHidden();

    for (auto& child_it : child_items_)
        child_it.second->updateHidden();
}


/**
 * Checks if the item is hidden, either itself or by parent
 */
bool ScatterSeriesTreeItem::itemHidden() const
{
    return hidden() || (parent_item_ && parent_item_->itemHidden());
}
