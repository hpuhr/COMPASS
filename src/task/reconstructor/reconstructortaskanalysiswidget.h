#pragma once

#pragma once

#include <QWidget>

class ReconstructorTask;

class QCheckBox;
class QLineEdit;

class ReconstructorTaskAnalysisWidget : public QWidget
{
    Q_OBJECT
  signals:

  public slots:
    void utnsChangedSlot(const QString& value);
    void recNumsChangedSlot(const QString& value);

  public:
    explicit ReconstructorTaskAnalysisWidget(ReconstructorTask& task, bool probimm_reconst, QWidget *parent = nullptr);
    virtual ~ReconstructorTaskAnalysisWidget();

    void updateValues();

  protected:
    void timestampsChanged();

    ReconstructorTask& task_;
    bool probimm_reconst_ {false};

    QCheckBox* debug_check_{nullptr};

    QLineEdit* utns_edit_{nullptr};
    QLineEdit* rec_nums_edit_{nullptr};

    QLineEdit* timestamp_min_edit_{nullptr};
    QLineEdit* timestamp_max_edit_{nullptr};

    QCheckBox* debug_association_check_{nullptr};
    QCheckBox* debug_outliers_check_{nullptr};
    QCheckBox* debug_accuracy_est_check_{nullptr};
    QCheckBox* debug_bias_correction_check_{nullptr};
    QCheckBox* debug_geo_altitude_correction_check_{nullptr};

    std::map<std::string, QCheckBox*> deep_debug_accuracy_estimation_checks_;
    std::map<std::string, QCheckBox*> deep_debug_accuracy_estimation_write_vp_checks_;

    QCheckBox* debug_reference_calculation_check_{nullptr};
    QCheckBox* debug_kalman_chains_check_{nullptr};
    QCheckBox* debug_write_reconstruction_viewpoints_check_{nullptr};
};
