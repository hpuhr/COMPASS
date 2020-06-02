#ifndef VIEWPOINTSREPORTGENERATORDIALOG_H
#define VIEWPOINTSREPORTGENERATORDIALOG_H

#include <QDialog>

class ViewPointsReportGenerator;

class QPushButton;
class QLabel;
class QProgressBar;

class ViewPointsReportGeneratorDialog : public QDialog
{
    Q_OBJECT

public slots:
  void runSlot();

public:
    ViewPointsReportGeneratorDialog(ViewPointsReportGenerator& generator,
                                    QWidget* parent = nullptr, Qt::WindowFlags f = 0);

    void setElapsedTime (const std::string& time_str);
    void setProgress (unsigned int min, unsigned int max, unsigned int value);
    void setStatus (const std::string& status);
    void setRemainingTime (const std::string& time_str);

protected:
    ViewPointsReportGenerator& generator_;

    QPushButton* run_button_{nullptr};

    QLabel* elapsed_time_label_{nullptr};
    QProgressBar* progress_bar_{nullptr};
    QLabel* status_label_{nullptr};
    QLabel* remaining_time_label_{nullptr};
};

#endif // VIEWPOINTSREPORTGENERATORDIALOG_H
