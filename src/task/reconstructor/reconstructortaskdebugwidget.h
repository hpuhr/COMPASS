#pragma once

#pragma once

#include <QWidget>

class ReconstructorTask;

class QCheckBox;
class QLineEdit;

class ReconstructorTaskDebugWidget : public QWidget
{
    Q_OBJECT
  signals:

  public slots:
    void toggleDebugSlot();

    void utnsChangedSlot(const QString& value);
    void recNumsChangedSlot(const QString& value);

  public:
    explicit ReconstructorTaskDebugWidget(ReconstructorTask& task, QWidget *parent = nullptr);
    virtual ~ReconstructorTaskDebugWidget();

    void updateValues();

  protected:
    void timestampsChanged();

    ReconstructorTask& task_;

    QCheckBox* debug_check_{nullptr};

    QLineEdit* utns_edit_{nullptr};
    QLineEdit* rec_nums_edit_{nullptr};
    QLineEdit* timestamp_min_edit_{nullptr};
    QLineEdit* timestamp_max_edit_{nullptr};
};
