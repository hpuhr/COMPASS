#ifndef EVALUATIONDATAWIDGET_H
#define EVALUATIONDATAWIDGET_H

#include <QWidget>

class EvaluationData;

class QTableView;
class QSortFilterProxyModel;

class EvaluationDataWidget : public QWidget
{
public:
    EvaluationDataWidget(EvaluationData& eval_data);

protected:
    EvaluationData& eval_data_;

    QTableView* table_view_{nullptr};
    QSortFilterProxyModel* proxy_model_{nullptr};
};

#endif // EVALUATIONDATAWIDGET_H
