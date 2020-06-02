#ifndef VIEWPOINTSREPORTGENERATORDIALOG_H
#define VIEWPOINTSREPORTGENERATORDIALOG_H

#include <QDialog>

class ViewPointsReportGenerator;

class ViewPointsReportGeneratorDialog : public QDialog
{
    Q_OBJECT

public:
    ViewPointsReportGeneratorDialog(ViewPointsReportGenerator& generator,
                                    QWidget* parent = nullptr, Qt::WindowFlags f = 0);

protected:
    ViewPointsReportGenerator& generator_;
};

#endif // VIEWPOINTSREPORTGENERATORDIALOG_H
