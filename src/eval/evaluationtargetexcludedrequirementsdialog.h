#pragma once

#include <QDialog>

#include <set>
#include <string>

class QScrollArea;
class QGridLayout;
class QCheckBox;
class QLineEdit;

class EvaluationTargetExcludedRequirementsDialog : public QDialog
{
public:
    EvaluationTargetExcludedRequirementsDialog(
                const std::string utn_str, std::set<std::string> selected_requirements,
        std::set<std::string> available_requirements, std::string comment="", QWidget* parent=nullptr);
    virtual ~EvaluationTargetExcludedRequirementsDialog() = default;

    std::set<std::string> selectedRequirements() const;
    std::string comment() const;

protected:
    std::set<std::string> selected_requirements_;
    std::set<std::string> available_requirements_;

private:
    QScrollArea* scroll_area_{nullptr};
    QWidget* scroll_widget_{nullptr};
    QGridLayout* grid_layout_{nullptr};

    std::vector<QCheckBox*> requirement_checkboxes_;
    QLineEdit* comment_edit_{nullptr};
};

