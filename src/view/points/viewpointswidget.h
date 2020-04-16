#ifndef VIEWPOINTSWIDGET_H
#define VIEWPOINTSWIDGET_H

#include <QWidget>

class ViewManager;
class ViewPointsTableModel;

class QTableView;

class ViewPointsWidget : public QWidget
{
    Q_OBJECT

  public slots:

public:
    ViewPointsWidget(ViewManager& view_manager);
    virtual ~ViewPointsWidget();

    void update();

private:
    ViewManager& view_manager_;

    QTableView* table_view_{nullptr};
    ViewPointsTableModel* table_model_{nullptr};
};

#endif // VIEWPOINTSWIDGET_H
