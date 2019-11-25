#ifndef CREATEARTASASSOCIATIONSTASKWIDGET_H
#define CREATEARTASASSOCIATIONSTASKWIDGET_H


#include <QWidget>

class CreateARTASAssociationsTask;
class QPushButton;
class DBODataSourceSelectionComboBox;
class DBOVariableSelectionWidget;
class QLineEdit;

class CreateARTASAssociationsTaskWidget : public QWidget
{
    Q_OBJECT

public slots:
    void currentDataSourceChangedSlot ();
    void anyVariableChangedSlot();

    void endTrackTimeEditSlot(QString value);
    void beginningTimeEditSlot(QString value);
    void dubiousTimeEditSlot(QString value);
    void futureTimeEditSlot(QString value);

    void runSlot ();
    void runDoneSlot ();

public:

    CreateARTASAssociationsTaskWidget(CreateARTASAssociationsTask& task, QWidget* parent=0,
                                      Qt::WindowFlags f=0);

    virtual ~CreateARTASAssociationsTaskWidget();

    void update ();

protected:
    CreateARTASAssociationsTask& task_;

    DBODataSourceSelectionComboBox* ds_combo_ {nullptr};
    DBOVariableSelectionWidget* ds_id_box_ {nullptr};
    DBOVariableSelectionWidget* track_num_box_ {nullptr};
    DBOVariableSelectionWidget* track_begin_box_ {nullptr};
    DBOVariableSelectionWidget* track_end_box_ {nullptr};

    DBOVariableSelectionWidget* key_box_ {nullptr};
    DBOVariableSelectionWidget* hash_box_ {nullptr};
    DBOVariableSelectionWidget* tod_box_ {nullptr};

    QLineEdit* end_track_time_edit_ {nullptr};
    QLineEdit* beginning_time_edit_ {nullptr};
    QLineEdit* dubious_time_edit_ {nullptr};
    QLineEdit* future_time_edit_ {nullptr};

    QPushButton* calc_button_ {nullptr};
};

#endif // CREATEARTASASSOCIATIONSTASKWIDGET_H
