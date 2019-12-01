#ifndef CREATEARTASASSOCIATIONSTASKWIDGET_H
#define CREATEARTASASSOCIATIONSTASKWIDGET_H


#include <QWidget>

class CreateARTASAssociationsTask;
class QPushButton;
class DBODataSourceSelectionComboBox;
class DBOVariableSelectionWidget;
class QLineEdit;
class QCheckBox;

class CreateARTASAssociationsTaskWidget : public QWidget
{
    Q_OBJECT

public slots:
    void currentDataSourceChangedSlot ();
    void anyVariableChangedSlot();

    void endTrackTimeEditSlot(QString value);

    void associationTimePastEditSlot(QString value);
    void associationTimeFutureEditSlot(QString value);
    void missesAcceptableTimeEditSlot(QString value);
    void associationsDubiousDistantTimeEditSlot(QString value);
    void associationDubiousCloseTimePastEditSlot(QString value);
    void associationDubiousCloseTimeFutureEditSlot(QString value);

    void anyTrackFlagChangedSlot();

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
    DBOVariableSelectionWidget* track_coasting_box_ {nullptr};

    DBOVariableSelectionWidget* key_box_ {nullptr};
    DBOVariableSelectionWidget* hash_box_ {nullptr};
    DBOVariableSelectionWidget* tod_box_ {nullptr};

    QLineEdit* end_track_time_edit_ {nullptr};

    QLineEdit* association_time_past_edit_ {nullptr};
    QLineEdit* association_time_future_edit_ {nullptr};

    QLineEdit* misses_acceptable_time_edit_ {nullptr};

    QLineEdit* associations_dubious_distant_time_edit_ {nullptr};
    QLineEdit* association_dubious_close_time_past_edit_ {nullptr};
    QLineEdit* association_dubious_close_time_future_edit_ {nullptr};

    QCheckBox* ignore_track_end_associations_check_ {nullptr};
    QCheckBox* mark_track_end_associations_dubious_check_ {nullptr};
    QCheckBox* ignore_track_coasting_associations_check_ {nullptr};
    QCheckBox* mark_track_coasting_associations_dubious_check_ {nullptr};

    QPushButton* calc_button_ {nullptr};
};

#endif // CREATEARTASASSOCIATIONSTASKWIDGET_H
