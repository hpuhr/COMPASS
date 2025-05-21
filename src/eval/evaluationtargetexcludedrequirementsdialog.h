#pragma once

#include <QDialog>

#include <set>
#include <string>

class QScrollArea;
class QGridLayout;
class QCheckBox;

class EvaluationTargetExcludedRequirementsDialog : public QDialog
{
public:
    EvaluationTargetExcludedRequirementsDialog(
                const std::string utn_str, std::set<std::string> selected_requirements,
        std::set<std::string> available_requirements, QWidget* parent=nullptr);
    virtual ~EvaluationTargetExcludedRequirementsDialog() = default;

    std::set<std::string> selectedRequirements() const;

protected:
    std::set<std::string> selected_requirements_;
    std::set<std::string> available_requirements_;

private:
    QScrollArea* scroll_area_;
    QWidget* scroll_widget_;
    QGridLayout* grid_layout_;
    std::vector<QCheckBox*> requirement_checkboxes_;
};

