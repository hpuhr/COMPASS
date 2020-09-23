#ifndef EVALUATIONSTANDARDCOMBOBOX_H
#define EVALUATIONSTANDARDCOMBOBOX_H

#include <QComboBox>

class EvaluationManager;

class EvaluationStandardComboBox : public QComboBox
{
    Q_OBJECT

public slots:
    /// @brief Emitted if type was changed
    void changedStandardSlot(const QString& standard_name); // slot for box

public:
    EvaluationStandardComboBox(EvaluationManager& eval_man, QWidget* parent=0);
    virtual ~EvaluationStandardComboBox();

    void setStandardName(const std::string& value);
    void updateStandards();

protected:
    EvaluationManager& eval_man_;
};

#endif // EVALUATIONSTANDARDCOMBOBOX_H
