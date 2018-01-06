#ifndef DBOLABELDEFINITIONWIDGET_H
#define DBOLABELDEFINITIONWIDGET_H


#include <QWidget>
#include <QStringList>

#include "dbolabeldefinition.h"

class QTableWidget;
class QTableWidgetItem;

class DBOLabelDefinitionWidget : public QWidget
{
  Q_OBJECT

public slots:
  void cellChangedSlot (int row, int column);

public:
  DBOLabelDefinitionWidget(DBOLabelDefinition* definition);
  virtual ~DBOLabelDefinitionWidget();

private:
  bool seperate_window_;
  //QWidget *target_;
  DBOLabelDefinition *definition_;
  QTableWidget *table_;
  //std::map <std::string, DBOLabelEntry *> &entries_;
  QStringList list_;

  //std::vector<std::vector <QTableWidgetItem*>> table_items_;

  void setTable ();
};

#endif // DBOLABELDEFINITIONWIDGET_H
