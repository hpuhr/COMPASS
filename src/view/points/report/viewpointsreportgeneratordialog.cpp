#include "viewpointsreportgeneratordialog.h"
#include "viewpointsreportgenerator.h"
#include "logger.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QTextEdit>
#include <QLineEdit>


ViewPointsReportGeneratorDialog::ViewPointsReportGeneratorDialog(ViewPointsReportGenerator& generator,
                                                                 QWidget* parent, Qt::WindowFlags f)
    : QDialog(parent, f), generator_(generator)
{
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    setWindowTitle("Export View Points as PDF");

    setModal(true);

    setMinimumSize(QSize(600, 800));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(16);

    QVBoxLayout* main_layout = new QVBoxLayout();

    int row = 0;
    {
        QGridLayout* config_grid = new QGridLayout();

        config_grid->addWidget(new QLabel("Report Path"), row, 0);

        directory_edit_ = new QLineEdit ();
        directory_edit_->setText(generator_.reportPath().c_str());
        connect(directory_edit_, &QLineEdit::textEdited, this, &ViewPointsReportGeneratorDialog::pathEditedSlot);
        config_grid->addWidget(directory_edit_, row, 1);

        ++row;
        config_grid->addWidget(new QLabel("Report Filename"), row, 0);

        filename_edit_ = new QLineEdit();
        filename_edit_->setText(generator_.reportFilename().c_str());
        connect(filename_edit_, &QLineEdit::textEdited, this, &ViewPointsReportGeneratorDialog::filenameEditedSlot);
        config_grid->addWidget(filename_edit_, row, 1);

        main_layout->addLayout(config_grid);
    }

    run_button_ = new QPushButton("Run");
    connect(run_button_, &QPushButton::clicked, this, &ViewPointsReportGeneratorDialog::runSlot);
    main_layout->addWidget(run_button_);

    main_layout->addStretch();

    row = 0;
    {
        QGridLayout* status_grid = new QGridLayout();

        status_grid->addWidget(new QLabel("Elapsed Time"), row, 0);
        elapsed_time_label_ = new QLabel();
        elapsed_time_label_->setAlignment(Qt::AlignRight);
        status_grid->addWidget(elapsed_time_label_, row, 1);

        ++row;
        status_grid->addWidget(new QLabel("Progress"), row, 0);
        progress_bar_ = new QProgressBar();
        status_grid->addWidget(progress_bar_, row, 1);

        ++row;
        status_grid->addWidget(new QLabel("Status"), row, 0);
        status_label_ = new QLabel();
        status_label_->setAlignment(Qt::AlignRight);
        status_grid->addWidget(status_label_, row, 1);

        ++row;
        status_grid->addWidget(new QLabel("Remaining Time"), row, 0);
        remaining_time_label_ = new QLabel();
        remaining_time_label_->setAlignment(Qt::AlignRight);
        status_grid->addWidget(remaining_time_label_, row, 1);

        main_layout->addLayout(status_grid);
    }

    main_layout->addStretch();

    quit_button_ = new QPushButton("Cancel");
    connect(quit_button_, &QPushButton::clicked, this, &ViewPointsReportGeneratorDialog::cancelSlot);
    main_layout->addWidget(quit_button_);

    setLayout(main_layout);
}

void ViewPointsReportGeneratorDialog::pathEditedSlot (const QString& text)
{
    generator_.reportPath(text.toStdString());
}

void ViewPointsReportGeneratorDialog::filenameEditedSlot(const QString& text)
{
    generator_.reportFilename(text.toStdString());
}


void ViewPointsReportGeneratorDialog::runSlot()
{
    loginf << "ViewPointsReportGeneratorDialog: runSlot";
    generator_.run();
}

void ViewPointsReportGeneratorDialog::cancelSlot()
{
    loginf << "ViewPointsReportGeneratorDialog: cancelSlot";
    generator_.cancel();
}

void ViewPointsReportGeneratorDialog::setElapsedTime (const std::string& time_str)
{
    assert (elapsed_time_label_);
    elapsed_time_label_->setText(time_str.c_str());
}

void ViewPointsReportGeneratorDialog::setProgress (unsigned int min, unsigned int max, unsigned int value)
{
    assert (progress_bar_);
    assert (max >= min);

    progress_bar_->setRange(min, max);
    progress_bar_->setValue(value);
}

void ViewPointsReportGeneratorDialog::setStatus (const std::string& status)
{
    assert (status_label_);
    status_label_->setText(status.c_str());
}

void ViewPointsReportGeneratorDialog::setRemainingTime (const std::string& time_str)
{
    assert (remaining_time_label_);
    remaining_time_label_->setText(time_str.c_str());
}
