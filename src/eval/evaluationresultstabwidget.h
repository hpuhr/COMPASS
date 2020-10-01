#ifndef EVALUATIONRESULTSTABWIDGET_H
#define EVALUATIONRESULTSTABWIDGET_H

#include <QWidget>
#include <QTreeView>
#include <QStackedWidget>

#include <memory>

class EvaluationManager;
class EvaluationManagerWidget;

class QPushButton;
class QSplitter;

class EvaluationResultsTabWidget : public QWidget
{
    Q_OBJECT

public slots:
    void itemClickedSlot(const QModelIndex& index);

    void stepBackSlot();

public:
    EvaluationResultsTabWidget(EvaluationManager& eval_man, EvaluationManagerWidget& man_widget);
    virtual ~EvaluationResultsTabWidget();

    void expand();

    void showResultWidget(QWidget* widget); // can be nullptr

    void selectId (const std::string& id);

protected:
    EvaluationManager& eval_man_;
    EvaluationManagerWidget& man_widget_;

    QSplitter* splitter_ {nullptr};
    std::unique_ptr<QTreeView> tree_view_;

    QStackedWidget* results_widget_{nullptr};

    QPushButton* back_button_ {nullptr};
    std::vector<std::string> id_history_;

    void expandAllParents (QModelIndex index);
    void updateBackButton ();
};

#endif // EVALUATIONRESULTSTABWIDGET_H
