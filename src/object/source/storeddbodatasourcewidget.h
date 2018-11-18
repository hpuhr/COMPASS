#ifndef STOREDDBODATASOURCEWIDGET_H
#define STOREDDBODATASOURCEWIDGET_H

#include "storeddbodatasource.h"

#include <QWidget>
#include <QLineEdit>

class InvalidQLineEdit : public QLineEdit
{
public:
    explicit InvalidQLineEdit(QWidget *parent = Q_NULLPTR) : QLineEdit(parent) {}
    explicit InvalidQLineEdit(const QString &text, QWidget *parent = Q_NULLPTR) : QLineEdit (text, parent) {}

    void setValid (bool value) {
        if (value)
            setStyleSheet("QLineEdit { background: rgb(255, 255, 255); selection-background-color:"
                                         " rgb(200, 200, 200); }");
        else
            setStyleSheet("QLineEdit { background: rgb(255, 100, 100); selection-background-color:"
                                         " rgb(255, 200, 200); }");
    }
};

class StoredDBODataSourceWidget : public QWidget
{
    Q_OBJECT

public slots:
    // slots for setting by QLineEdit
    //void changedIdSlot ();
    void changedShortNameColumnSlot (const QString& value_str);
    void changedNameColumnSlot (const QString& value_str);
    void changedSacColumnSlot (const QString& value_str);
    void changedSicColumnSlot (const QString& value_str);
    void changedLatitudeColumnSlot (const QString& value_str);
    void changedLongitudeColumnSlot (const QString& value_str);
    void changedAltitudeColumnSlot (const QString& value_str);

    // slots for updating from ds
    void updateIdSlot();
    void updateShortNameColumnSlot ();
    void updateNameColumnSlot ();
    void updateSacColumnSlot ();
    void updateSicColumnSlot ();
    void updateLatitudeColumnSlot ();
    void updateLongitudeColumnSlot ();
    void updateAltitudeColumnSlot ();

    void deleteSlot ();

public:
    StoredDBODataSourceWidget(StoredDBODataSource& data_source, bool add_headers=false,
                              QWidget* parent=0, Qt::WindowFlags f=0);
    virtual ~StoredDBODataSourceWidget() {}

    void setDataSource (StoredDBODataSource& data_source);

    void update ();

private:
    StoredDBODataSource* data_source_;

    QLineEdit* id_edit_{nullptr};
    QLineEdit* name_edit_{nullptr};
    InvalidQLineEdit* short_name_edit_{nullptr};
    InvalidQLineEdit* sac_edit_{nullptr};
    InvalidQLineEdit* sic_edit_{nullptr};
    InvalidQLineEdit* latitude_edit_{nullptr};
    InvalidQLineEdit* longitude_edit_{nullptr};
    InvalidQLineEdit* altitude_edit_{nullptr};
};

#endif // STOREDDBODATASOURCEWIDGET_H
