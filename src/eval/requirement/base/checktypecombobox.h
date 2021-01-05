#ifndef CHECKTYPECOMBOBOX_H
#define CHECKTYPECOMBOBOX_H

#include "eval/requirement/base/checktype.h"

#include <QComboBox>

namespace EvaluationRequirement
{


class CheckTypeComboBox : public QComboBox
{
    Q_OBJECT

signals:
    /// @brief Emitted if type was changed
    void changedTypeSignal();

public:
    /// @brief Constructor
    CheckTypeComboBox(QWidget* parent = 0)
        : QComboBox(parent)
    {
        addItem("Minimum");
        addItem("Maximum");

        connect(this, SIGNAL(activated(const QString&)), this, SIGNAL(changedTypeSignal()));
    }
    /// @brief Destructor
    virtual ~CheckTypeComboBox() {}

    /// @brief Returns the currently selected framing
    CHECK_TYPE getType()
    {
        if (currentText() == "Minimum")
            return CHECK_TYPE::MIN;
        else
            return CHECK_TYPE::MAX;
    }

    /// @brief Sets the currently selected data type
    void setType(CHECK_TYPE type)
    {
        if (type == CHECK_TYPE::MIN)
            setCurrentIndex(0);
        else
            setCurrentIndex(1);
    }
};

}

#endif // CHECKTYPECOMBOBOX_H
