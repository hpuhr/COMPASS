#ifndef VIEWPOINTSREPORTGENERATORDIALOG_H
#define VIEWPOINTSREPORTGENERATORDIALOG_H

#include <QDialog>

class ViewPointsReportGenerator;

class QPushButton;
class QLabel;
class QProgressBar;
class QLineEdit;

class ViewPointsReportGeneratorDialog : public QDialog
{
    Q_OBJECT

public slots:
    void pathEditedSlot (const QString& text);
    void filenameEditedSlot(const QString& text);

    void runSlot();
    void cancelSlot();

public:
    ViewPointsReportGeneratorDialog(ViewPointsReportGenerator& generator,
                                    QWidget* parent = nullptr, Qt::WindowFlags f = 0);

    void updateFileInfo ();

    void setElapsedTime (const std::string& time_str);
    void setProgress (unsigned int min, unsigned int max, unsigned int value);
    void setStatus (const std::string& status);
    void setRemainingTime (const std::string& time_str);

protected:
    ViewPointsReportGenerator& generator_;

    QLineEdit* directory_edit_ {nullptr};
    QLineEdit* filename_edit_ {nullptr};

    QPushButton* run_button_{nullptr};

    QLabel* elapsed_time_label_{nullptr};
    QProgressBar* progress_bar_{nullptr};
    QLabel* status_label_{nullptr};
    QLabel* remaining_time_label_{nullptr};

    QPushButton* quit_button_{nullptr};
};

#endif // VIEWPOINTSREPORTGENERATORDIALOG_H
