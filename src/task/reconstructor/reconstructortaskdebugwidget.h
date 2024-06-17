#pragma once

#pragma once

#include <QWidget>

class ReconstructorTask;

class QLineEdit;

class ReconstructorTaskDebugWidget : public QWidget
{
    Q_OBJECT
  signals:

  public slots:
    void utnsChangedSlot(const QString& value);
    void recNumsChangedSlot(const QString& value);

  public:
    explicit ReconstructorTaskDebugWidget(ReconstructorTask& task, QWidget *parent = nullptr);
    virtual ~ReconstructorTaskDebugWidget();

    void updateValues();

  protected:
    void timestampsChanged();

    ReconstructorTask& task_;

    QLineEdit* utns_edit_{nullptr};
    QLineEdit* rec_nums_edit_{nullptr};
    QLineEdit* timestamp_min_edit_{nullptr};
    QLineEdit* timestamp_max_edit_{nullptr};
};
