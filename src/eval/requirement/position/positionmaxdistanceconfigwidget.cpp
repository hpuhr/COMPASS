#include "eval/requirement/position/positionmaxdistanceconfigwidget.h"
#include "eval/requirement/position/positionmaxdistanceconfig.h"
#include "textfielddoublevalidator.h"
#include "logger.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>

namespace EvaluationRequirement
{

    PositionMaxDistanceConfigWidget::PositionMaxDistanceConfigWidget(PositionMaxDistanceConfig& config)
        : QWidget(), config_(config)
    {
        form_layout_ = new QFormLayout();

        config_.addGUIElements(form_layout_);

        // max ref time diff
        max_ref_time_diff_edit_ = new QLineEdit(QString::number(config_.maxRefTimeDiff()));
        max_ref_time_diff_edit_->setValidator(new QDoubleValidator(0.0, 30.0, 2, this));
        connect(max_ref_time_diff_edit_, &QLineEdit::textEdited,
                this, &PositionMaxDistanceConfigWidget::maxRefTimeDiffEditSlot);

        form_layout_->addRow("Maximum Reference Time Difference [s]", max_ref_time_diff_edit_);


        // max dist
        max_distance_edit_ = new QLineEdit(QString::number(config_.maxDistance()));
        max_distance_edit_->setValidator(new QDoubleValidator(0.0, 10000.0, 2, this));
        connect(max_distance_edit_, &QLineEdit::textEdited,
                this, &PositionMaxDistanceConfigWidget::maxDistanceEditSlot);

        form_layout_->addRow("Maximum Distance [m]", max_distance_edit_);

        // prob
        maximum_prob_edit_ = new QLineEdit(QString::number(config_.maximumProbability()));
        maximum_prob_edit_->setValidator(new QDoubleValidator(0.01, 1.0, 2, this));
        connect(maximum_prob_edit_, &QLineEdit::textEdited,
                this, &PositionMaxDistanceConfigWidget::maximumProbEditSlot);

        form_layout_->addRow("Maximum Probability [1]", maximum_prob_edit_);

        setLayout(form_layout_);
    }

    void PositionMaxDistanceConfigWidget::maxRefTimeDiffEditSlot(QString value)
    {
        loginf << "PositionMaxDistanceConfigWidget: maxRefTimeDiffEditSlot: value " << value.toStdString();

        bool ok;
        float val = value.toFloat(&ok);

        if (ok)
            config_.maxRefTimeDiff(val);
        else
            loginf << "PositionMaxDistanceConfigWidget: maxRefTimeDiffEditSlot: invalid value";
    }


    void PositionMaxDistanceConfigWidget::maxDistanceEditSlot(QString value)
    {
        loginf << "PositionMaxDistanceConfigWidget: maxDistanceEditSlot: value " << value.toStdString();

        bool ok;
        float val = value.toFloat(&ok);

        if (ok)
            config_.maxDistance(val);
        else
            loginf << "PositionMaxDistanceConfigWidget: maxDistanceEditSlot: invalid value";
    }

    void PositionMaxDistanceConfigWidget::PositionMaxDistanceConfigWidget::maximumProbEditSlot(QString value)
    {
        loginf << "PositionMaxDistanceConfigWidget: maximumProbEditSlot: value " << value.toStdString();

        bool ok;
        float val = value.toFloat(&ok);

        if (ok)
            config_.maximumProbability(val);
        else
            loginf << "PositionMaxDistanceConfigWidget: maximumProbEditSlot: invalid value";
    }

}
