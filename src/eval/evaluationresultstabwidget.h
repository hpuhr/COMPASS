#ifndef EVALUATIONRESULTSTABWIDGET_H
#define EVALUATIONRESULTSTABWIDGET_H

#include <QWidget>
#include <QTreeView>
#include <QStackedWidget>

#include <memory>

class EvaluationManager;
class EvaluationManagerWidget;

class EvaluationResultsTabWidget : public QWidget
{
    Q_OBJECT

public slots:
    void itemClickedSlot(const QModelIndex& index);

public:
    EvaluationResultsTabWidget(EvaluationManager& eval_man, EvaluationManagerWidget& man_widget);

    void expand();

    void showResultWidget(QWidget* widget); // can be nullptr

    void selectId (const std::string& id);

protected:
    EvaluationManager& eval_man_;
    EvaluationManagerWidget& man_widget_;

    std::unique_ptr<QTreeView> tree_view_;

    QStackedWidget* results_widget_{nullptr};

    void expandAllParents (QModelIndex index);
};

#endif // EVALUATIONRESULTSTABWIDGET_H
