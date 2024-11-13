#include "reconstructortaskdialog.h"
#include "reconstructortask.h"
#include "simplereconstructor.h"
#include "simplereconstructorwidget.h"
#include "global.h"
#include "compass.h"
#include "licensemanager.h"

#if USE_EXPERIMENTAL_SOURCE == true
#include "probimmreconstructor.h"
#include "probimmreconstructorwidget.h"
#endif

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QFormLayout>
#include <QComboBox>
#include <QStandardItemModel>

ReconstructorTaskDialog::ReconstructorTaskDialog(ReconstructorTask& task)
    : QDialog(), task_(task)
{
    setWindowTitle("Reconstruct Reference Trajectories");
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    setModal(true);

    setMinimumSize(QSize(1000, 800));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(16);

    QVBoxLayout* main_layout = new QVBoxLayout();

    QFormLayout* combo_layout = new QFormLayout;
    combo_layout->setMargin(0);
    combo_layout->setFormAlignment(Qt::AlignRight | Qt::AlignTop);

    reconstructor_box_ = new QComboBox();
    reconstructor_box_->addItem(QString::fromStdString(ReconstructorTask::ScoringUMReconstructorName));

    const auto& license_manager = COMPASS::instance().licenseManager();

#if USE_EXPERIMENTAL_SOURCE == true
    if (license_manager.componentEnabled(license::License::Component::ComponentProbIMMReconstructor))
        reconstructor_box_->addItem(QString::fromStdString(ReconstructorTask::ProbImmReconstructorName));
#endif

    int idx = reconstructor_box_->findText(QString::fromStdString(task_.currentReconstructorStr()));
    reconstructor_box_->setCurrentIndex(idx);

    connect(reconstructor_box_, &QComboBox::currentTextChanged,
            this, &ReconstructorTaskDialog::reconstructorMethodChangedSlot);

    combo_layout->addRow(tr("Reconstructor Method"), reconstructor_box_);

    reconstructor_info_ = new QLabel;
    reconstructor_info_->setWordWrap(true);
    reconstructor_info_->setVisible(false);
    reconstructor_info_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto reconstructor_info2 = new QLabel;
    reconstructor_info2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QHBoxLayout* info_layout = new QHBoxLayout;
    info_layout->addWidget(reconstructor_info_);
    info_layout->addWidget(reconstructor_info2);

    combo_layout->addRow(tr(""), info_layout);

    main_layout->addLayout(combo_layout);

    reconstructor_widget_stack_ = new QStackedWidget();

    reconstructor_widget_stack_->addWidget(task_.simpleReconstructor()->widget());

#if USE_EXPERIMENTAL_SOURCE == true
    if (license_manager.componentEnabled(license::License::Component::ComponentProbIMMReconstructor))
        reconstructor_widget_stack_->addWidget(task_.probIMMReconstructor()->widget());
#endif

    showCurrentReconstructorWidget();

    main_layout->addWidget(reconstructor_widget_stack_);

    QHBoxLayout* button_layout = new QHBoxLayout();

    cancel_button_ = new QPushButton("Cancel");
    connect(cancel_button_, &QPushButton::clicked, this, &QDialog::reject);
    button_layout->addWidget(cancel_button_);

    button_layout->addStretch();

    run_button_ = new QPushButton("Run");
    connect(run_button_, &QPushButton::clicked, this, &QDialog::accept);
    button_layout->addWidget(run_button_);

    main_layout->addLayout(button_layout);

    setLayout(main_layout);

    updateButtons();
    updateReconstructorInfo();
}

ReconstructorTaskDialog::~ReconstructorTaskDialog()
{
}

void ReconstructorTaskDialog::updateReconstructorInfo()
{
    reconstructor_info_->setText("");
    reconstructor_info_->setVisible(false);

    auto rec = task_.currentReconstructor();
    if (!rec)
        return;

    auto info_str = rec->reconstructorInfoString();
    if (!info_str.empty())
    {
        reconstructor_info_->setText(QString::fromStdString(info_str));
        reconstructor_info_->setVisible(true);
    }
}

void ReconstructorTaskDialog::showCurrentReconstructorWidget()
{
    const auto& reconst_str = task_.currentReconstructorStr();
    int idx = reconstructor_box_->findText(QString::fromStdString(reconst_str));

    loginf << "ReconstructorTaskDialog: showCurrentReconstructorWidget: value " << idx;

    assert(idx >= 0);

    reconstructor_widget_stack_->setCurrentIndex(idx);

    task_.currentReconstructor()->updateWidgets();
}

void ReconstructorTaskDialog::updateButtons()
{
    assert (run_button_);

    run_button_->setDisabled(!task_.canRun());
}

void ReconstructorTaskDialog::reconstructorMethodChangedSlot(const QString& value)
{
    loginf << "ReconstructorTaskDialog: reconstructorMethodChangedSlot: value " << value.toStdString();

    task_.currentReconstructorStr(value.toStdString());

    showCurrentReconstructorWidget();
    updateReconstructorInfo();
}
