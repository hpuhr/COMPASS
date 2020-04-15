#ifndef VIEWPOINTSWIDGET_H
#define VIEWPOINTSWIDGET_H

#include <QWidget>

class ViewManager;

class ViewPointsWidget : public QWidget
{
    Q_OBJECT

  public slots:

public:
    ViewPointsWidget(ViewManager& view_manager);
    virtual ~ViewPointsWidget();

private:
    ViewManager& view_manager_;
};

#endif // VIEWPOINTSWIDGET_H
