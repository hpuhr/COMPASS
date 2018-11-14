#ifndef DBOEDITDATASOURCESWIDGET_H
#define DBOEDITDATASOURCESWIDGET_H

#include <QWidget>
#include "dbobject.h"

class DBObject;
class QVBoxLayout;
class QGridLayout;
class QLabel;
class QPushButton;

class DBOEditDataSourcesWidget : public QWidget
{
    Q_OBJECT
public slots:
    void syncOptionsFromDB();
    void syncOptionsFromCfg();
    void performActions();

public:
    DBOEditDataSourcesWidget(DBObject* object, QWidget* parent=0, Qt::WindowFlags f=0);
    virtual ~DBOEditDataSourcesWidget();

    void update ();

private:
    /// @brief DBObject to be managed
    DBObject* object_ {nullptr};

    QVBoxLayout* config_ds_layout_ {nullptr};
    QPushButton* sync_from_cfg_button_ {nullptr};

    QLabel* action_heading_label_ {nullptr};
    std::string action_heading_;
    QGridLayout* action_layout_ {nullptr};
    DBOEditDataSourceActionOptionsCollection action_collection_;
    QPushButton* perform_actions_button_ {nullptr};

    QGridLayout* db_ds_layout_ {nullptr};
    QPushButton* sync_from_db_button_ {nullptr};

    void clearSyncOptions();
    void displaySyncOptions ();
};

#endif // DBOEDITDATASOURCESWIDGET_H
