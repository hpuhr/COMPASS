/*
 * ViewContainerWidget.h
 *
 *  Created on: Jan 15, 2012
 *      Author: sk
 */

#ifndef VIEWCONTAINERWIDGET_H_
#define VIEWCONTAINERWIDGET_H_

#include <QWidget>
#include <QTabWidget>
#include <QMenu>

#include "configurable.h"

class ManagementWidget;
class View;
class ViewManager;
class ViewContainerTabWidget;
class QHBoxLayout;
class QPushButton;

/**
 * Serves as container for different views. May be embedded in a parent widget, or in new window, which is decided by constructor.
 *
 * Includes DBConfigTab, therefore DB HAS to be loaded before a constructor is called.
 */
class ViewContainerWidget : public QWidget, public Configurable
{
  Q_OBJECT
public slots:
  void showMenuSlot ();
  //void saveViewTemplate ();
  void deleteView ();

public:
  ViewContainerWidget(const std::string &class_id, const std::string &instance_id, ViewManager *parent);
  virtual ~ViewContainerWidget();

//  void addGeographicView();
//  void addHistogramView();
//  void addListBoxView();
//  void addMosaicView();
//  void addScatterPlotView();
//  void addTemplateView (std::string template_name);

  ViewContainerTabWidget *getTabWidget ();

  void addView (View *view);
  void removeView (View *view);
  const std::vector<View*>& getViews() const;

  virtual void generateSubConfigurable (std::string class_id, std::string instance_id);

  std::string getName ();
  static unsigned int getViewCount () { return view_count_; }

protected:
  ViewManager &view_manager_;

  bool seperate_window_;

  std::vector<View*> views_;
  QWidget *target_;
  QHBoxLayout *target_layout_;
  ViewContainerTabWidget *tab_widget_;
  ManagementWidget *manager_;

  bool deleted_;

  QMenu menu_;
  QPushButton *last_active_manage_button_;

  std::map <QPushButton*, View*> view_manage_buttons_;

  unsigned int pos_x_;
  unsigned int pos_y_;
  unsigned int width_;
  unsigned int height_;
  unsigned int min_width_;
  unsigned int min_height_;

 static unsigned int view_count_;

 void closeEvent ( QCloseEvent * event );
 virtual void moveEvent (QMoveEvent *event);
 virtual void resizeEvent (QResizeEvent *event);

 virtual void checkSubConfigurables ();

 void createGUIElements ();

};

#endif /* VIEWCONTAINERWIDGET_H_ */
