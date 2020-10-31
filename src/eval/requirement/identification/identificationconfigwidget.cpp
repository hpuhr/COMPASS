#include "eval/requirement/identification/identificationconfigwidget.h"
#include "eval/requirement/identification/identificationconfig.h"
#include "textfielddoublevalidator.h"
#include "logger.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>

using namespace std;

namespace EvaluationRequirement
{

    IdentificationConfigWidget::IdentificationConfigWidget(IdentificationConfig& config)
        : QWidget(), config_(config)
    {
        form_layout_ = new QFormLayout();

        config_.addGUIElements(form_layout_);

        // max ref time diff
        max_ref_time_diff_edit_ = new QLineEdit(QString::number(config_.maxRefTimeDiff()));
        max_ref_time_diff_edit_->setValidator(new QDoubleValidator(0.0, 30.0, 2, this));
        connect(max_ref_time_diff_edit_, &QLineEdit::textEdited,
                this, &IdentificationConfigWidget::maxRefTimeDiffEditSlot);

        form_layout_->addRow("Maximum Reference Time Difference [s]", max_ref_time_diff_edit_);

        // prob
        minimum_prob_edit_ = new QLineEdit(QString::number(config_.minimumProbability()));
        minimum_prob_edit_->setValidator(new QDoubleValidator(0.0001, 1.0, 4, this));
        connect(minimum_prob_edit_, &QLineEdit::textEdited,
                this, &IdentificationConfigWidget::minimumProbEditSlot);

        form_layout_->addRow("Minimum Probability [1]", minimum_prob_edit_);

        setLayout(form_layout_);
    }

    void IdentificationConfigWidget::maxRefTimeDiffEditSlot(QString value)
    {
        loginf << "IdentificationConfigWidget: maxRefTimeDiffEditSlot: value " << value.toStdString();

        bool ok;
        float val = value.toFloat(&ok);

        if (ok)
            config_.maxRefTimeDiff(val);
        else
            loginf << "IdentificationConfigWidget: maxRefTimeDiffEditSlot: invalid value";
    }


    void IdentificationConfigWidget::minimumProbEditSlot(QString value)
    {
        loginf << "EvaluationRequirementIdentificationConfigWidget: minimumProbEditSlot: value " << value.toStdString();

        bool ok;
        float val = value.toFloat(&ok);

        if (ok)
            config_.minimumProbability(val);
        else
            loginf << "EvaluationRequirementIdentificationConfigWidget: minimumProbEditSlot: invalid value";
    }

}
