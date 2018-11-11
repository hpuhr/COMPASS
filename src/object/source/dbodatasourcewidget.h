#ifndef DBODATASOURCEWIDGET_H
#define DBODATASOURCEWIDGET_H

#include "dbodatasource.h"

#include <QWidget>

class QLineEdit;

class DBODataSourceWidget : public QWidget
{
    Q_OBJECT

public slots:
    // slots for setting by QLineEdit
    void changedIdSlot ();
    void changedShortNameColumnSlot ();
    void changedNameColumnSlot ();
    void changedSacColumnSlot ();
    void changedSicColumnSlot ();
    void changedLatitudeColumnSlot ();
    void changedLongitudeColumnSlot ();
    void changedAltitudeColumnSlot ();

    // slots for updating from ds
    void updateIdSlot();
    void updateShortNameColumnSlot ();
    void updateNameColumnSlot ();
    void updateSacColumnSlot ();
    void updateSicColumnSlot ();
    void updateLatitudeColumnSlot ();
    void updateLongitudeColumnSlot ();
    void updateAltitudeColumnSlot ();

public:
    DBODataSourceWidget(DBODataSource& data_source, QWidget* parent=0, Qt::WindowFlags f=0);
    virtual ~DBODataSourceWidget() {}

    void setDataSource (DBODataSource& data_source);

    void update ();

private:
    DBODataSource* data_source_;

    QLineEdit* id_edit_{nullptr};
    QLineEdit* short_name_edit_{nullptr};
    QLineEdit* name_edit_{nullptr};
    QLineEdit* sac_edit_{nullptr};
    QLineEdit* sic_edit_{nullptr};
    QLineEdit* latitude_edit_{nullptr};
    QLineEdit* longitude_edit_{nullptr};
    QLineEdit* altitude_edit_{nullptr};
};

#endif // DBODATASOURCEWIDGET_H
