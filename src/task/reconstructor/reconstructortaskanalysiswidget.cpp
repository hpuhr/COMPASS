#include "reconstructortaskanalysiswidget.h"
#include "reconstructortask.h"
#include "stringconv.h"
#include "timeconv.h"
#include "logger.h"
#include "compass.h"
#include "datasourcemanager.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QCheckBox>

using namespace std;
using namespace Utils;


ReconstructorTaskAnalysisWidget::ReconstructorTaskAnalysisWidget(
    ReconstructorTask& task, bool probimm_reconst, QWidget *parent)
    : QWidget{parent}, task_(task), probimm_reconst_(probimm_reconst)
{
    bool add_debug_stuff = !COMPASS::isAppImage() || COMPASS::instance().expertMode();

    QFormLayout* combo_layout = new QFormLayout;
    //combo_layout->setMargin(0);
    combo_layout->setFormAlignment(Qt::AlignRight | Qt::AlignTop);

    debug_check_ = new QCheckBox ();
    connect(debug_check_, &QCheckBox::clicked,
            this, [ = ] (bool ok) { task_.debugSettings().debug_ = ok; });
    combo_layout->addRow("Analyse Reconstruction", debug_check_);

    utns_edit_ = new QLineEdit();
    connect(utns_edit_, &QLineEdit::textEdited, this, &ReconstructorTaskAnalysisWidget::utnsChangedSlot);

    if (add_debug_stuff)
        combo_layout->addRow("Debug UTNs", utns_edit_);

    rec_nums_edit_ = new QLineEdit();
    connect(rec_nums_edit_, &QLineEdit::textEdited, this, &ReconstructorTaskAnalysisWidget::recNumsChangedSlot);

    if (add_debug_stuff)
        combo_layout->addRow("Debug Record Numbers", rec_nums_edit_);

    timestamp_min_edit_ = new QLineEdit();
    connect(timestamp_min_edit_, &QLineEdit::textEdited, this, &ReconstructorTaskAnalysisWidget::timestampsChanged);

    if (add_debug_stuff)
        combo_layout->addRow("Debug Timestamp Min.", timestamp_min_edit_);

    timestamp_max_edit_ = new QLineEdit();
    connect(timestamp_max_edit_, &QLineEdit::textEdited, this, &ReconstructorTaskAnalysisWidget::timestampsChanged);

    if (add_debug_stuff)
        combo_layout->addRow("Debug Timestamp Max.", timestamp_max_edit_);

    debug_association_check_= new QCheckBox();
    connect(debug_association_check_, &QCheckBox::clicked,
            this, [ = ] (bool ok) { task_.debugSettings().debug_association_ = ok; });

    if (add_debug_stuff)
        combo_layout->addRow("Debug Association", debug_association_check_);

    debug_outliers_check_= new QCheckBox();
    connect(debug_outliers_check_, &QCheckBox::clicked,
            this, [ = ] (bool ok) { task_.debugSettings().debug_outlier_detection_ = ok; });

    if (probimm_reconst_ && add_debug_stuff)
        combo_layout->addRow("Debug Outlier Detection", debug_outliers_check_);

    // acc est

    debug_accuracy_est_check_ = new QCheckBox();
    connect(debug_accuracy_est_check_, &QCheckBox::clicked,
            this, [ = ] (bool ok) { task_.debugSettings().debug_accuracy_estimation_ = ok; });

    if (probimm_reconst_)
        combo_layout->addRow("Analyse Accuracy Estimation", debug_accuracy_est_check_);

    debug_bias_correction_check_ = new QCheckBox();
    connect(debug_bias_correction_check_, &QCheckBox::clicked,
            this, [ = ] (bool ok) { task_.debugSettings().debug_bias_correction_ = ok; });

    if (probimm_reconst_)
        combo_layout->addRow("Analyse Bias Correction", debug_bias_correction_check_);

    debug_geo_altitude_correction_check_ = new QCheckBox();
    connect(debug_geo_altitude_correction_check_, &QCheckBox::clicked,
            this, [ = ] (bool ok) { task_.debugSettings().debug_geo_altitude_correction_ = ok; });

    if (probimm_reconst_)
        combo_layout->addRow("Analyse Geo.Altitude Correction", debug_geo_altitude_correction_check_);

    // deep acc est

    for (auto& ds_type : COMPASS::instance().dataSourceManager().data_source_types_)
    {
        QCheckBox* check = new QCheckBox(("Deep Analyse "+ds_type+" Accuracy Estimation").c_str());
        connect(check, &QCheckBox::clicked,
                this, [ = ] (bool ok) { task_.debugSettings().deepDebugAccuracyEstimation(ds_type,ok); });

        QCheckBox* write_vp_check = new QCheckBox("Write View Points");
        connect(write_vp_check, &QCheckBox::clicked,
                this, [ = ] (bool ok) { task_.debugSettings().deepDebugAccuracyEstimationWriteVP(ds_type,ok); });

        deep_debug_accuracy_estimation_checks_[ds_type] = check;
        deep_debug_accuracy_estimation_write_vp_checks_[ds_type] = write_vp_check;

        if (probimm_reconst_)
            combo_layout->addRow(check, write_vp_check);
    }

    // reference stuff

    debug_reference_calculation_check_ = new QCheckBox();
    connect(debug_reference_calculation_check_, &QCheckBox::clicked,
            this, [ = ] (bool ok) { task_.debugSettings().debug_reference_calculation_ = ok; });

    if (add_debug_stuff)
        combo_layout->addRow("Debug Reference Calculation", debug_reference_calculation_check_);

    debug_kalman_chains_check_ = new QCheckBox();
    connect(debug_kalman_chains_check_, &QCheckBox::clicked,
            this, [ = ] (bool ok) { task_.debugSettings().debug_kalman_chains_= ok; });

    if (add_debug_stuff)
        combo_layout->addRow("Debug Kalman Chains", debug_kalman_chains_check_);

    debug_write_reconstruction_viewpoints_check_ = new QCheckBox();
    connect(debug_write_reconstruction_viewpoints_check_, &QCheckBox::clicked,
            this, [ = ] (bool ok) { task_.debugSettings().debug_write_reconstruction_viewpoints_ = ok; });

    if (add_debug_stuff)
        combo_layout->addRow("Write Debug Reconstruction View Points", debug_write_reconstruction_viewpoints_check_);

    setLayout(combo_layout);

    updateValues();
}

ReconstructorTaskAnalysisWidget::~ReconstructorTaskAnalysisWidget()
{
}

void ReconstructorTaskAnalysisWidget::updateValues()
{
    loginf << "ReconstructorTaskDebugWidget: updateValues";

    bool add_debug_stuff = !COMPASS::isAppImage() || COMPASS::instance().expertMode();

    if (!add_debug_stuff) // disable everything not shown in the settings, to be on the safe side
    {
        task_.debugSettings().debug_utns_.clear();
        task_.debugSettings().debug_rec_nums_.clear();

        // task_.debugSettings().debug_timestamp_min_ = {}; // should be ok not to reset
        // task_.debugSettings().debug_timestamp_max_ = {};

        task_.debugSettings().debug_association_ = false;

        task_.debugSettings().debug_reference_calculation_ = false;
        task_.debugSettings().debug_kalman_chains_ = false;
        task_.debugSettings().debug_write_reconstruction_viewpoints_ = false;
    }

    assert (debug_check_);
    debug_check_->setChecked(task_.debugSettings().debug_);

    assert (utns_edit_);
    utns_edit_->setText(String::compress(task_.debugSettings().debug_utns_, ',').c_str());

    assert (rec_nums_edit_);
    rec_nums_edit_->setText(String::compress(task_.debugSettings().debug_rec_nums_, ',').c_str());

    assert (timestamp_min_edit_);
    if (!task_.debugSettings().debug_timestamp_min_.is_not_a_date_time())
        timestamp_min_edit_->setText(QString::fromStdString(
            Utils::Time::toString(task_.debugSettings().debug_timestamp_min_)));
    else
        timestamp_min_edit_->setText("");

    assert (timestamp_max_edit_);
    if (!task_.debugSettings().debug_timestamp_max_.is_not_a_date_time())
        timestamp_max_edit_->setText(QString::fromStdString(
            Utils::Time::toString(task_.debugSettings().debug_timestamp_max_)));
    else
        timestamp_max_edit_->setText("");

    assert (debug_association_check_);
    debug_association_check_->setChecked(task_.debugSettings().debug_association_);

    assert (debug_outliers_check_);
    debug_outliers_check_->setChecked(task_.debugSettings().debug_outlier_detection_);

    // acc est

    assert (debug_accuracy_est_check_);
    debug_accuracy_est_check_->setChecked(task_.debugSettings().debug_accuracy_estimation_);

    assert (debug_bias_correction_check_);
    debug_bias_correction_check_->setChecked(task_.debugSettings().debug_bias_correction_);

    assert (debug_geo_altitude_correction_check_);
    debug_geo_altitude_correction_check_->setChecked(task_.debugSettings().debug_geo_altitude_correction_);

    // deep acc est

    for (auto& ds_type : COMPASS::instance().dataSourceManager().data_source_types_)
    {
        assert (deep_debug_accuracy_estimation_checks_.count(ds_type));
        assert (deep_debug_accuracy_estimation_write_vp_checks_.count(ds_type));

        deep_debug_accuracy_estimation_checks_[ds_type]->setChecked(
            task_.debugSettings().deepDebugAccuracyEstimation(ds_type));

        deep_debug_accuracy_estimation_write_vp_checks_[ds_type]->setChecked(
            task_.debugSettings().deepDebugAccuracyEstimationWriteVP(ds_type));
    }

    // reference stuff

    assert (debug_reference_calculation_check_);
    debug_reference_calculation_check_->setChecked(task_.debugSettings().debug_reference_calculation_);

    assert (debug_kalman_chains_check_);
    debug_kalman_chains_check_->setChecked(task_.debugSettings().debug_kalman_chains_);

    assert (debug_write_reconstruction_viewpoints_check_);
    debug_write_reconstruction_viewpoints_check_->setChecked(
        task_.debugSettings().debug_write_reconstruction_viewpoints_);
}

void ReconstructorTaskAnalysisWidget::utnsChangedSlot(const QString& value)
{
    loginf << "ReconstructorTaskDebugWidget: utnsChangedSlot: value '" << value.toStdString() << "'";

    set<unsigned int> values_tmp;
    vector<string> split_str = String::split(value.toStdString(), ',');

    bool ok;

    for (auto& tmp_str : split_str)
    {
        unsigned int utn_tmp = QString(tmp_str.c_str()).toInt(&ok, 10);

        if (!ok)
        {
            logerr << "ReconstructorTaskDebugWidget: utnsChangedSlot: utn '" << tmp_str << "' not valid";
            break;
        }

        values_tmp.insert(utn_tmp);
    }

    task_.debugSettings().debug_utns_ = values_tmp;
}

void ReconstructorTaskAnalysisWidget::recNumsChangedSlot(const QString& value)
{
    loginf << "ReconstructorTaskDebugWidget: recNumsChangedSlot: value '" << value.toStdString() << "'";

    set<unsigned long> values_tmp;
    vector<string> split_str = String::split(value.toStdString(), ',');

    bool ok;

    for (auto& tmp_str : split_str)
    {
        unsigned long utn_tmp = QString(tmp_str.c_str()).toInt(&ok, 10);

        if (!ok)
        {
            logerr << "ReconstructorTaskDebugWidget: utnsChangedSlot: utn '" << tmp_str << "' not valid";
            break;
        }

        values_tmp.insert(utn_tmp);
    }

    task_.debugSettings().debug_rec_nums_ = values_tmp;
}

void ReconstructorTaskAnalysisWidget::timestampsChanged()
{
    auto checkTimestamp = [ & ] (QLineEdit* line_edit)
    {
        auto txt = line_edit->text().toStdString();
        auto ts  = Utils::Time::fromString(txt);

        bool ts_ok = !ts.is_not_a_date_time();

        boost::optional<boost::posix_time::ptime> ret;

        if (ts_ok)
            ret = ts;

        line_edit->setStyleSheet(ts_ok ? "" : "color: red");

        return ret;
    };

    auto ts_min = checkTimestamp(timestamp_min_edit_);
    task_.debugSettings().debug_timestamp_min_ = ts_min.has_value() ? ts_min.value() : boost::posix_time::ptime();

    if (ts_min.has_value())
        loginf << "ReconstructorTaskDebugWidget: timestampsChanged: set ts min to "
               << Utils::Time::toString(ts_min.value());

    auto ts_max = checkTimestamp(timestamp_max_edit_);
    task_.debugSettings().debug_timestamp_max_ = ts_max.has_value() ? ts_max.value() : boost::posix_time::ptime();

    if (ts_max.has_value())
        loginf << "ReconstructorTaskDebugWidget: timestampsChanged: set ts max to "
               << Utils::Time::toString(ts_max.value());
}
