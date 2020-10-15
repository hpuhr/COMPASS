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

    if (eval_man_.hasCurrentStandard())
    {
        EvaluationStandard& std = eval_man_.currentStandard();
        const string& std_name = std.name();

        for (auto& sec_lay_it : eval_man_.sectorsLayers())
        {
            const string& sec_lay_name = sec_lay_it->name();

            for (auto& sec_it : sec_lay_it->sectors())
            {
                col = 0;
                const string& sec_name = sec_it->name();

                QLabel* sec_label = new QLabel((sec_lay_name+":"+sec_name).c_str());
                grid_layout_->addWidget(sec_label, row, col);
                ++col;

                for (auto& req_grp_it : std)
                {
                    const string& req_grp_name = req_grp_it.first;

                    QCheckBox* check = new QCheckBox(req_grp_name.c_str());
                    check->setChecked(eval_man_.useGroupInSector(sec_lay_name, sec_name, req_grp_name));
                    check->setProperty("sec_lay_name", sec_lay_name.c_str());
                    check->setProperty("sec_name", sec_name.c_str());
                    check->setProperty("req_grp_name", req_grp_name.c_str());
                    connect(check, &QCheckBox::clicked, this, &EvaluationSectorWidget::toggleUseGroupSlot);

                    grid_layout_->addWidget(check, row, col);

                    ++col;
                }
                ++row;
            }
        }
    }
}

void EvaluationSectorWidget::toggleUseGroupSlot()
{
    QCheckBox* check = static_cast<QCheckBox*> (QObject::sender());
    assert (check);

    string sec_lay_name = check->property("sec_lay_name").toString().toStdString();
    string sec_name = check->property("sec_name").toString().toStdString();
    string req_grp_name = check->property("req_grp_name").toString().toStdString();

    assert (eval_man_.hasCurrentStandard());

    eval_man_.useGroupInSector(sec_lay_name, sec_name, req_grp_name, check->checkState() == Qt::Checked);
}
