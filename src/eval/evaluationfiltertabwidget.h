
#pragma once

#include <QWidget>

#include <memory>

class EvaluationCalculator;
class EvaluationManagerWidget;
class TimeWindowCollectionWidget;

class QLineEdit;
class QCheckBox;
class QDateTimeEdit;

/**
 */
class EvaluationFilterTabWidget : public QWidget
{
    Q_OBJECT

private slots:
    void toggleUseFiltersSlot();

    void toggleUseTimeSlot();
    void timeBeginEditedSlot (const QDateTime& datetime);
    void timeEndEditedSlot (const QDateTime& datetime);

    void toggleUseRefTrajAccuracySlot();
    void minRefTrajAccuracyEditedSlot (const QString& text);

    void toggleUseADSBSlot();
    void toggleUseV0Slot();
    void toggleUseV1Slot();
    void toggleUseV2Slot();

    void toggleUseMinNUCPSlot();
    void minNUCPEditedSlot (const QString& text);
    void toggleUseMaxNUCPSlot();
    void maxNUCPEditedSlot (const QString& text);

    void toggleUseMinNICSlot();
    void minNICEditedSlot (const QString& text);
    void toggleUseMaxNICSlot();
    void maxNICEditedSlot (const QString& text);

    void toggleUseMinNACpSlot();
    void minNACPEditedSlot (const QString& text);
    void toggleUseMaxNACpSlot();
    void maxNACPEditedSlot (const QString& text);

    void toggleUseMinSILv1Slot();
    void minSILv1PEditedSlot (const QString& text);
    void toggleUseMaxSILv1Slot();
    void maxSILv1PEditedSlot (const QString& text);

    void toggleUseMinSILv2Slot();
    void minSILv2PEditedSlot (const QString& text);
    void toggleUseMaxSILv2Slot();
    void maxSILv2PEditedSlot (const QString& text);

public:
    EvaluationFilterTabWidget(EvaluationCalculator& calculator);

    void update();

protected:
    EvaluationCalculator& calculator_;

    QCheckBox* use_filter_check_{nullptr};

    QCheckBox* use_time_check_{nullptr};
    QDateTimeEdit* time_begin_edit_{nullptr};
    QDateTimeEdit* time_end_edit_{nullptr};

    // reftraj

    QCheckBox* use_reftraj_acc_check_{nullptr};
    QLineEdit* min_reftraj_acc_edit_{nullptr};

    // adsb

    QCheckBox* use_adsb_check_{nullptr};

    QCheckBox* use_v0_check_{nullptr};
    QCheckBox* use_v1_check_{nullptr};
    QCheckBox* use_v2_check_{nullptr};

    // nucp
    QCheckBox* use_min_nucp_check_{nullptr};
    QLineEdit* min_nucp_edit_{nullptr};

    QCheckBox* use_max_nucp_check_{nullptr};
    QLineEdit* max_nucp_edit_{nullptr};

    // nic
    QCheckBox* use_min_nic_check_{nullptr};
    QLineEdit* min_nic_edit_{nullptr};

    QCheckBox* use_max_nic_check_{nullptr};
    QLineEdit* max_nic_edit_{nullptr};

    // nacp
    QCheckBox* use_min_nacp_check_{nullptr};
    QLineEdit* min_nacp_edit_{nullptr};

    QCheckBox* use_max_nacp_check_{nullptr};
    QLineEdit* max_nacp_edit_{nullptr};

    // sil v1
    QCheckBox* use_min_sil_v1_check_{nullptr};
    QLineEdit* min_sil_v1_edit_{nullptr};

    QCheckBox* use_max_sil_v1_check_{nullptr};
    QLineEdit* max_sil_v1_edit_{nullptr};

    // sil v2
    QCheckBox* use_min_sil_v2_check_{nullptr};
    QLineEdit* min_sil_v2_edit_{nullptr};

    QCheckBox* use_max_sil_v2_check_{nullptr};
    QLineEdit* max_sil_v2_edit_{nullptr};

    bool update_active_ {false};
};

