#ifndef STOREDDBODATASOURCEWIDGET_H
#define STOREDDBODATASOURCEWIDGET_H

#include "storeddbodatasource.h"

#include <QWidget>

class QLineEdit;

class StoredDBODataSourceWidget : public QWidget
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
    StoredDBODataSourceWidget(StoredDBODataSource& data_source, QWidget* parent=0, Qt::WindowFlags f=0);
    virtual ~StoredDBODataSourceWidget() {}

    void update ();

private:
    StoredDBODataSource& data_source_;

    QLineEdit* id_edit_{nullptr};
    QLineEdit* short_name_edit_{nullptr};
    QLineEdit* name_edit_{nullptr};
    QLineEdit* sac_edit_{nullptr};
    QLineEdit* sic_edit_{nullptr};
    QLineEdit* latitude_edit_{nullptr};
    QLineEdit* longitude_edit_{nullptr};
    QLineEdit* altitude_edit_{nullptr};
};

#endif // STOREDDBODATASOURCEWIDGET_H
