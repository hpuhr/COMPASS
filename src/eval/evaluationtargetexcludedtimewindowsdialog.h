#pragma once

#include <QDialog>

#include "timewindowcollectionwidget.h"

class EvaluationTargetExcludedTimeWindowsDialog : public QDialog
{
    Q_OBJECT

public:
    EvaluationTargetExcludedTimeWindowsDialog(
        const std::string utn_str,
        Utils::TimeWindowCollection& collection, QWidget* parent=nullptr);
    virtual ~EvaluationTargetExcludedTimeWindowsDialog() = default;

protected:
    Utils::TimeWindowCollection& collection_;

    TimeWindowCollectionWidget* tw_widget_{nullptr};
};

