#include "evaluationsectorwidget.h"
#include "evaluationmanager.h"
#include "sectorlayer.h"
#include "sector.h"
#include "evaluationstandard.h"
#include "eval/requirement/group.h"
#include "eval/requirement/config.h"

#include <QLabel>
#include <QCheckBox>
#include <QGridLayout>

using namespace std;

EvaluationSectorWidget::EvaluationSectorWidget(EvaluationManager& eval_man)
    : eval_man_(eval_man)
{
    grid_layout_ = new QGridLayout();

    update();

    setLayout(grid_layout_);
}

void EvaluationSectorWidget::update()
{
    assert (grid_layout_);

    QLayoutItem* child;
    while ((child = grid_layout_->takeAt(0)) != 0)
    {
        grid_layout_->removeItem(child);
        delete child;
    }

    unsigned int row=0;
    unsigned int col=0;

    QFont font_italic;
    font_italic.setItalic(true);

    if (eval_man_.hasCurrentStandard())
    {
        EvaluationStandard& std = eval_man_.currentStandard();
        //const string& std_name = std.name();

        for (auto& sec_lay_it : eval_man_.sectorsLayers())
        {
            col = 0;
            const string& sector_layer_name = sec_lay_it->name();

            QLabel* sec_label = new QLabel(sector_layer_name.c_str());
            sec_label->setFont(font_italic);
            grid_layout_->addWidget(sec_label, row, col);
            ++col;

            for (auto& req_grp_it : std)
            {
                const string& requirement_group_name = req_grp_it.first;

                QCheckBox* check = new QCheckBox(requirement_group_name.c_str());
                check->setChecked(eval_man_.useGroupInSectorLayer(sector_layer_name, requirement_group_name));
                check->setProperty("sector_layer_name", sector_layer_name.c_str());
                check->setProperty("requirement_group_name", requirement_group_name.c_str());
                connect(check, &QCheckBox::clicked, this, &EvaluationSectorWidget::toggleUseGroupSlot);

                grid_layout_->addWidget(check, row, col);

                ++col;
            }
            ++row;
        }
    }
}

void EvaluationSectorWidget::toggleUseGroupSlot()
{
    QCheckBox* check = static_cast<QCheckBox*> (QObject::sender());
    assert (check);

    string sector_layer_name = check->property("sector_layer_name").toString().toStdString();
    string requirement_group_name = check->property("requirement_group_name").toString().toStdString();

    assert (eval_man_.hasCurrentStandard());

    eval_man_.useGroupInSectorLayer(sector_layer_name, requirement_group_name, check->checkState() == Qt::Checked);
}
