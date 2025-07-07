#pragma once

#include "json.hpp"

#include <QDialog>

class QGridLayout;

namespace dbContent
{

class LabelGenerator;

class LabelContentDialog : public QDialog
{
    Q_OBJECT

signals:
    void doneSignal();

public slots:
    void selectedVarChangedSlot();
    void doneClickedSlot();

public:
    LabelContentDialog(const std::string& dbcontent_name, LabelGenerator& label_generator);

    nlohmann::json labelConfig() const;

protected:
    std::string dbcontent_name_;
    LabelGenerator& label_generator_;

    nlohmann::json label_config_;

    QGridLayout* var_grid_{nullptr};

    QPushButton* done_button_{nullptr};

    void createVariableGrid();
};

}

