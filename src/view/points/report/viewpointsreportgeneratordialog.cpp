#include "viewpointsreportgeneratordialog.h"
#include "viewpointsreportgenerator.h"

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

    setWindowTitle("Export View Points Latex Report");

    setModal(true);

    setMinimumSize(QSize(600, 800));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(16);

    QVBoxLayout* main_layout = new QVBoxLayout();

    setLayout(main_layout);
}
