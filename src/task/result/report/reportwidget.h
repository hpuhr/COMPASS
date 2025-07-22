#pragma once

#include "report/treemodel.h"
#include "json_fwd.hpp"

#include <boost/optional.hpp>

#include <QWidget>
#include <QTreeView>
#include <QStackedWidget>

class TaskResultsWidget;
class PopupMenu;

class QPushButton;
class QLabel;

namespace ResultReport
{

class Section;

class ReportWidget : public QWidget
{
    Q_OBJECT

public slots:
    void itemClickedSlot(const QModelIndex& index);
    void itemDblClickedSlot(const QModelIndex& index);
    void contextMenuSlot(const QPoint& pos);

    void stepBackSlot();

public:
    ReportWidget(TaskResultsWidget& task_result_widget);
    virtual ~ReportWidget();

    void setReport(const std::shared_ptr<Report>& report);
    void clear();

    void expand();

    void showResultWidget(Section* section, // can be nullptr
                          bool preload_ondemand_contents); 

    void selectId (const std::string& id, 
                   bool show_figure,
                   const nlohmann::json& config);
    void reshowLastId ();

    std::string currentSectionID() const;
    nlohmann::json currentSectionConfig() const;

    boost::optional<nlohmann::json> getTableData(const std::string& result_id,
                                                 const std::string& table_id,
                                                 bool rowwise = true,
                                                 const std::vector<int>& cols = std::vector<int>()) const;

    void showFigure(const QModelIndex& index);

    static const std::string FieldConfigScrollPosV;
    static const std::string FieldConfigScrollPosH;

protected:
    void expandAllParents (QModelIndex index);
    void updateBackButton ();
    void updateCurrentSectionLabel();
    void triggerItem (const QModelIndex& index,
                      bool preload_ondemand_contents);
    
    bool configureSection(const nlohmann::json& config);

    TaskResultsWidget& task_result_widget_;

    ResultReport::TreeModel tree_model_;
    QTreeView* tree_view_;

    QStackedWidget* results_widget_{nullptr};

    QPushButton* sections_button_{nullptr};
    std::unique_ptr<PopupMenu> sections_menu_;

    QPushButton* back_button_ {nullptr};
    std::vector<std::string> id_history_;

    QLabel*  current_section_label_ = nullptr;
    Section* current_section_       = nullptr;
};

}
