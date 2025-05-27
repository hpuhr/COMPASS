#pragma once

#include <QDialog>

#include "timewindowcollectionwidget.h"

class QLineEdit;

class EvaluationTargetExcludedTimeWindowsDialog : public QDialog
{
    Q_OBJECT

public:
    EvaluationTargetExcludedTimeWindowsDialog(
        const std::string utn_str,
        Utils::TimeWindowCollection& collection, std::string comment="", QWidget* parent=nullptr);
    virtual ~EvaluationTargetExcludedTimeWindowsDialog() = default;

    std::string comment() const;

protected:
    Utils::TimeWindowCollection& collection_;

    TimeWindowCollectionWidget* tw_widget_{nullptr};
    QLineEdit* comment_edit_{nullptr};
};

