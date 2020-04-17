#ifndef VIEWPOINTSWIDGET_H
#define VIEWPOINTSWIDGET_H

#include <QWidget>

class ViewManager;
class ViewPointsTableModel;

class QTableView;
class QPushButton;

class ViewPointsWidget : public QWidget
{
    Q_OBJECT

  public slots:
    void exportSlot();
    void importSlot();

public:
    ViewPointsWidget(ViewManager& view_manager);
    virtual ~ViewPointsWidget();

    void update();

private:
    ViewManager& view_manager_;

    QTableView* table_view_{nullptr};
    ViewPointsTableModel* table_model_{nullptr};

    QPushButton* export_button_{nullptr};
    QPushButton* import_button_{nullptr};

    void importFile (const std::string& filename);
};

#endif // VIEWPOINTSWIDGET_H
