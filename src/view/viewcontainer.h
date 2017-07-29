#ifndef VIEWCONTAINER_H
#define VIEWCONTAINER_H

#include <QObject>
#include <QMenu>

#include "configurable.h"

class ManagementWidget;
class View;
class ViewManager;
class QHBoxLayout;
class QPushButton;
class QTabWidget;

class ViewManager;
class ViewContainerConfigWidget;

class ViewContainer : public QObject, public Configurable
{
    Q_OBJECT
  public slots:
    void showMenuSlot ();
    //void saveViewTemplate ();
    void deleteView ();

public:
    ViewContainer(const std::string &class_id, const std::string &instance_id, Configurable *parent, ViewManager *view_manager, QTabWidget *tab_widget);
    virtual ~ViewContainer();

    void addView (View *view);
    void removeView (View *view);
    const std::vector<View*>& getViews() const;

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    virtual std::string getName ();
    static unsigned int getViewCount () { return view_count_; }

    //  void addGeographicView();
    //  void addHistogramView();
    void addListBoxView();
    void addOSGView();
    //  void addMosaicView();
    //  void addScatterPlotView();
    //  void addTemplateView (std::string template_name);

    ViewContainerConfigWidget *configWidget ();

protected:
    ViewManager &view_manager_;

    std::vector<View*> views_;

    QTabWidget *tab_widget_;

    QMenu menu_;
    QPushButton *last_active_manage_button_;

    ViewContainerConfigWidget *config_widget_;

    std::map <QPushButton*, View*> view_manage_buttons_;

    static unsigned int view_count_;

    virtual void checkSubConfigurables ();
};

#endif // VIEWCONTAINER_H
