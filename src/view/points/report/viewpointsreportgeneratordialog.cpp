#include "viewpointsreportgeneratordialog.h"
#include "viewpointsreportgenerator.h"
#include "logger.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

ViewPointsReportGeneratorDialog::ViewPointsReportGeneratorDialog(ViewPointsReportGenerator& generator,
                                                                 QWidget* parent, Qt::WindowFlags f)
    : QDialog(parent, f), generator_(generator)
{
    //setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    setWindowTitle("Export View Points as PDF");

    setModal(true);

    setMinimumSize(QSize(600, 800));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(16);

    QVBoxLayout* main_layout = new QVBoxLayout();

    run_button_ = new QPushButton("Run");
    connect(run_button_, &QPushButton::clicked, this, &ViewPointsReportGeneratorDialog::runSlot);
    main_layout->addWidget(run_button_);

    setLayout(main_layout);
}


void ViewPointsReportGeneratorDialog::runSlot()
{
    loginf << "ViewPointsReportGeneratorDialog: runSlot";
    generator_.run();
}
