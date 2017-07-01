/*
 * ViewsWidget.h
 *
 *  Created on: Apr 12, 2012
 *      Author: sk
 */

#ifndef VIEWSWIDGET_H_
#define VIEWSWIDGET_H_

#include <QFrame>
#include <map>

class ViewManager;
class ViewContainerConfigWidget;
class QVBoxLayout;
class QPushButton;

class ViewManagerWidget : public QFrame
{
    Q_OBJECT
private slots:
    void databaseBusy ();
    void databaseIdle ();

    void addViewSlot();
    //  void addGeographicViewSlot();
    //  void addHistogramViewSlot();
    //  void addListBoxViewSlot();
    //  void addMosaicViewSlot();
    //  void addScatterPlotViewSlot();
    //  void addGeographicViewNewWindowSlot();
    //  void addHistogramViewNewWindowSlot();
    //  void addListBoxViewNewWindowSlot();
    //  void addMosaicViewNewWindowSlot();
    //  void addScatterPlotViewNewWindowSlot();
    //  void addTemplateSlot ();
    //  void addTemplateNewWindowSlot ();

public:
    ViewManagerWidget(ViewManager &view_manager);
    virtual ~ViewManagerWidget();

    void update ();

private:
    ViewManager &view_manager_;
    QVBoxLayout *layout_;
    QVBoxLayout *cont_layout_;

    QPushButton *add_button_;

    std::vector<ViewContainerConfigWidget*> cont_widgets_;
    std::map <QAction*, std::pair<std::string, int> > add_template_actions_;
};

#endif /* VIEWSWIDGET_H_ */
