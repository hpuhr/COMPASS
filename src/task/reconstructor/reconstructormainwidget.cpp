#include "reconstructormainwidget.h"
#include "reconstructorbase.h"

#include "logger.h"

#include <QLabel>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>

/**
*/
ReconstructorMainWidget::ReconstructorMainWidget(ReconstructorBase& reconstructor, 
                                                 QWidget* parent)
:   QWidget       (parent       )
,   reconstructor_(reconstructor)
{
    QFormLayout* layout = new QFormLayout;
    layout->setFormAlignment(Qt::AlignRight | Qt::AlignTop);

    setLayout(layout);

    auto addSection = [ & ] (const std::string& name)
    {
        QLabel* label = new QLabel(QString::fromStdString(name));
        auto f = label->font();
        f.setBold(true);
        label->setFont(f);

        layout->addRow(label);
    };

    addSection("Data Source");

    ds_name_edit_ = new QLineEdit;
    ds_name_edit_->setReadOnly(true);
    ds_name_edit_->setFrame(false);

    layout->addRow("Name", ds_name_edit_);

    ds_sac_box_ = new QSpinBox;
    ds_sac_box_->setReadOnly(true);
    ds_sac_box_->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
    ds_sac_box_->setFrame(false);
    ds_sac_box_->setMinimum(0);
    ds_sac_box_->setMaximum(std::numeric_limits<int>::max());

    layout->addRow("SAC", ds_sac_box_);

    ds_sic_box_ = new QSpinBox;
    ds_sic_box_->setReadOnly(true);
    ds_sic_box_->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
    ds_sic_box_->setFrame(false);
    ds_sic_box_->setMinimum(0);
    ds_sic_box_->setMaximum(std::numeric_limits<int>::max());

    layout->addRow("SIC", ds_sic_box_);

    ds_line_combo_ = new QComboBox;
    ds_line_combo_->addItem("1", QVariant(0));
    ds_line_combo_->addItem("2", QVariant(1));
    ds_line_combo_->addItem("3", QVariant(2));
    ds_line_combo_->addItem("4", QVariant(3));

    connect(ds_line_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
        [ = ] (int idx) { this->reconstructor_.baseSettings().ds_line = ds_line_combo_->itemData(idx).toInt(); });

    layout->addRow("Line ID", ds_line_combo_);
    
    addSection("Data Slicing");

    slice_length_box_ = new QSpinBox;
    slice_length_box_->setSuffix(" min");
    slice_length_box_->setMinimum(10);
    slice_length_box_->setMaximum(120);

    connect(slice_length_box_, QOverload<int>::of(&QSpinBox::valueChanged), 
        [ & ] (int v) { this->reconstructor_.baseSettings().slice_duration_in_minutes = v; });

    layout->addRow("Slice Length", slice_length_box_);

    slice_overlap_box_ = new QSpinBox;
    slice_overlap_box_->setSuffix(" min");
    slice_overlap_box_->setMinimum(2);
    slice_overlap_box_->setMaximum(20);

    connect(slice_overlap_box_, QOverload<int>::of(&QSpinBox::valueChanged), 
        [ & ] (int v) { this->reconstructor_.baseSettings().outdated_duration_in_minutes = v; });

    layout->addRow("Slice Overlap Length", slice_overlap_box_);

    updateValues();
}

/**
*/
ReconstructorMainWidget::~ReconstructorMainWidget() = default;

/**
*/
void ReconstructorMainWidget::updateValues()
{
    loginf << "ReconstructorMainWidget: updateValues";

    const auto& settings = reconstructor_.baseSettings();

    assert(ds_name_edit_);
    ds_name_edit_->setText(QString::fromStdString(settings.ds_name));

    assert(ds_sac_box_);
    ds_sac_box_->setValue(settings.ds_sac);

    assert(ds_sic_box_);
    ds_sic_box_->setValue(settings.ds_sic);

    assert (ds_line_combo_);
    int line_idx = ds_line_combo_->findData(QVariant(settings.ds_line));
    assert(line_idx >= 0);
    ds_line_combo_->setCurrentIndex(line_idx);

    assert (slice_length_box_);
    slice_length_box_->setValue(settings.slice_duration_in_minutes);

    assert (slice_overlap_box_);
    slice_overlap_box_->setValue(settings.outdated_duration_in_minutes);
}
