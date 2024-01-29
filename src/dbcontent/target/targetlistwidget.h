#ifndef DBCONTENT_TARGETLISTWIDGET_H
#define DBCONTENT_TARGETLISTWIDGET_H

#include <QWidget>
#include <QItemSelection>

class DBContentManager;

class QToolBar;
class QTableView;
class QSortFilterProxyModel;

namespace dbContent {

class TargetModel;

class TargetListWidget : public QWidget
{
    Q_OBJECT

public slots:
    void actionTriggeredSlot(QAction* action);
    void useAllSlot();
    void useNoneSlot();
    void clearCommentsSlot();
    void filterSlot();

    void customContextMenuSlot(const QPoint& p);
    void showFullUTNSlot ();
    void showSurroundingDataSlot ();
    void currentRowChanged(const QModelIndex& current, const QModelIndex& previous);
    void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

public:
    TargetListWidget(TargetModel& model, DBContentManager& dbcont_manager);
    virtual ~TargetListWidget() {};

    void resizeColumnsToContents();

protected:
    TargetModel& model_;
    DBContentManager& dbcont_manager_;

    QToolBar* toolbar_ {nullptr};

    QTableView* table_view_{nullptr};
    QSortFilterProxyModel* proxy_model_{nullptr};
};

};

#endif // DBCONTENT_TARGETLISTWIDGET_H
