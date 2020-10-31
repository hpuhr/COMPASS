#ifndef EVALUATIONDATAWIDGET_H
#define EVALUATIONDATAWIDGET_H

#include <QWidget>

class EvaluationData;
class EvaluationManager;

class QTableView;
class QSortFilterProxyModel;

class EvaluationDataWidget : public QWidget
{
    Q_OBJECT

public slots:
    void currentRowChanged(const QModelIndex& current, const QModelIndex& previous);

public:
    EvaluationDataWidget(EvaluationData& eval_data, EvaluationManager& eval_man);

    void resizeColumnsToContents();

protected:
    EvaluationData& eval_data_;
    EvaluationManager& eval_man_;

    QTableView* table_view_{nullptr};
    QSortFilterProxyModel* proxy_model_{nullptr};
};

#endif // EVALUATIONDATAWIDGET_H
