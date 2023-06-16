#ifndef EVALUATIONREQUIREMENTCOMPARISONTYPECOMBOBOX_H
#define EVALUATIONREQUIREMENTCOMPARISONTYPECOMBOBOX_H

#include "eval/requirement/base/comparisontype.h"

#include <QComboBox>

namespace EvaluationRequirement
{


class ComparisonTypeComboBox : public QComboBox
{
    Q_OBJECT

signals:
    /// @brief Emitted if type was changed
    void changedTypeSignal();

public:
    /// @brief Constructor
    ComparisonTypeComboBox(QWidget* parent = 0)
        : QComboBox(parent)
    {
        addItem(comparisonTypeLongString(COMPARISON_TYPE::LESS_THAN).c_str());
        addItem(comparisonTypeLongString(COMPARISON_TYPE::LESS_THAN_OR_EQUAL).c_str());
        addItem(comparisonTypeLongString(COMPARISON_TYPE::GREATER_THAN).c_str());
        addItem(comparisonTypeLongString(COMPARISON_TYPE::GREATER_THAN_OR_EQUAL).c_str());

        connect(this, SIGNAL(activated(const QString&)), this, SIGNAL(changedTypeSignal()));
    }
    /// @brief Destructor
    virtual ~ComparisonTypeComboBox() {}

    /// @brief Returns the currently selected framing
    COMPARISON_TYPE getType()
    {
        if (currentText() == "Less Than (<)")
            return COMPARISON_TYPE::LESS_THAN;
        else if (currentText() == "Less Than or Equal (<=)")
            return COMPARISON_TYPE::LESS_THAN_OR_EQUAL;
        else if (currentText() == "Greater Than (>)")
            return COMPARISON_TYPE::GREATER_THAN;
        else if (currentText() == "Greater Than or Equal (>=)")
            return COMPARISON_TYPE::GREATER_THAN_OR_EQUAL;
        else
            throw std::runtime_error("ComparisonTypeComboBox: getType: unknown type '"+currentText().toStdString()+"'");
    }

    /// @brief Sets the currently selected data type
    void setType(COMPARISON_TYPE type)
    {
        setCurrentIndex((unsigned int) type);
    }
};

}

#endif // EVALUATIONREQUIREMENTCOMPARISONTYPECOMBOBOX_H
