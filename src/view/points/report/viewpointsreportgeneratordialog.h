#ifndef VIEWPOINTSREPORTGENERATORDIALOG_H
#define VIEWPOINTSREPORTGENERATORDIALOG_H

#include <QDialog>

class ViewPointsReportGenerator;

class QPushButton;

class ViewPointsReportGeneratorDialog : public QDialog
{
    Q_OBJECT

public slots:
  void runSlot();

public:
    ViewPointsReportGeneratorDialog(ViewPointsReportGenerator& generator,
                                    QWidget* parent = nullptr, Qt::WindowFlags f = 0);

protected:
    ViewPointsReportGenerator& generator_;

    QPushButton* run_button_{nullptr};
};

#endif // VIEWPOINTSREPORTGENERATORDIALOG_H
