#ifndef EVALUATIONSTANDARDTABWIDGET_H
#define EVALUATIONSTANDARDTABWIDGET_H

#include <QWidget>

#include <memory>

class EvaluationManager;
class EvaluationManagerWidget;
class EvaluationStandardComboBox;

class EvaluationStandardTabWidget : public QWidget
{
    Q_OBJECT

private slots:
    void changedStandardSlot(const QString& standard_name); // from std box
    void changedStandardsSlot(); // eval man
    void changedCurrentStandardSlot(); // eval man

public:
    EvaluationStandardTabWidget(EvaluationManager& eval_man, EvaluationManagerWidget& man_widget);

protected:
    EvaluationManager& eval_man_;
    EvaluationManagerWidget& man_widget_;

    std::unique_ptr<EvaluationStandardComboBox> standard_box_ {nullptr};
};

#endif // EVALUATIONSTANDARDTABWIDGET_H
