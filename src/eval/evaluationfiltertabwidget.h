#ifndef EVALUATIONFILTERTABWIDGET_H
#define EVALUATIONFILTERTABWIDGET_H

#include <QWidget>

#include <memory>

class EvaluationManager;
class EvaluationManagerWidget;

class QLineEdit;
class QCheckBox;

class EvaluationFilterTabWidget : public QWidget
{
    Q_OBJECT

private slots:
    void toggleUseFiltersSlot();

    void toggleUseTimeSlot();
    void timeBeginEditedSlot (const QString& text);
    void timeEndEditedSlot (const QString& text);

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
    EvaluationFilterTabWidget(EvaluationManager& eval_man, EvaluationManagerWidget& man_widget);

    void update();

protected:
    EvaluationManager& eval_man_;
    EvaluationManagerWidget& man_widget_;

    QCheckBox* use_filter_check_{nullptr};

    QCheckBox* use_time_check_{nullptr};
    QLineEdit* time_begin_edit_{nullptr};
    QLineEdit* time_end_edit_{nullptr};

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
};

#endif // EVALUATIONFILTERTABWIDGET_H
