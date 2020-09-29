#include "evaluationstandardtabwidget.h"
#include "evaluationmanagerwidget.h"
#include "evaluationmanager.h"
#include "evaluationstandardcombobox.h"
#include "evaluationstandard.h"
#include "evaluationstandardwidget.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QStackedWidget>
#include <QFrame>
#include <QTreeView>

using namespace std;

EvaluationStandardTabWidget::EvaluationStandardTabWidget(EvaluationManager& eval_man,
                                                         EvaluationManagerWidget& man_widget)
    : QWidget(nullptr), eval_man_(eval_man), man_widget_(man_widget)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    QFont font_bold;
    font_bold.setBold(true);

    // standard
    {
        QHBoxLayout* std_layout = new QHBoxLayout();

        QLabel* standard_label = new QLabel("Standard");
        standard_label->setFont(font_bold);
        std_layout->addWidget(standard_label);

        standard_box_.reset(new EvaluationStandardComboBox(eval_man_));

        std_layout->addWidget(standard_box_.get());

        main_layout->addLayout(std_layout);
    }

    // buttons
    {
        QHBoxLayout* button_layout = new QHBoxLayout();

        add_button_ = new QPushButton("Add");
        connect (add_button_, &QPushButton::clicked,
                 this, &EvaluationStandardTabWidget::addStandardSlot);
        button_layout->addWidget(add_button_);

        rename_button_ = new QPushButton("Rename");
        button_layout->addWidget(rename_button_);

        copy_button_ = new QPushButton("Copy");
        button_layout->addWidget(copy_button_);

        remove_button_ = new QPushButton("Remove");
        connect (remove_button_, &QPushButton::clicked,
                 this, &EvaluationStandardTabWidget::removeStandardSlot);
        button_layout->addWidget(remove_button_);

        updateButtons();

        main_layout->addLayout(button_layout);
    }

    // standards stack
    {
//        QFrame* line = new QFrame();
//        line->setFrameShape(QFrame::HLine);
//        line->setFrameShadow(QFrame::Sunken);
//        main_layout->addWidget(line);

        standards_widget_ = new QStackedWidget();
        main_layout->addWidget(standards_widget_);
    }

    if (eval_man_.hasCurrentStandard())
        updateStandardStack();

    // connections
    connect (&eval_man_, &EvaluationManager::standardsChangedSignal,
             this, &EvaluationStandardTabWidget::changedStandardsSlot);
    connect (&eval_man_, &EvaluationManager::currentStandardChangedSignal,
             this, &EvaluationStandardTabWidget::changedCurrentStandardSlot);

    setContentsMargins(0, 0, 0, 0);
    setLayout(main_layout);
}

void EvaluationStandardTabWidget::changedStandardsSlot()
{
    loginf << "EvaluationStandardTabWidget: changedStandardsSlot";

    assert (standard_box_);
    standard_box_->updateStandards();
}

void EvaluationStandardTabWidget::changedCurrentStandardSlot()
{
    loginf << "EvaluationStandardTabWidget: changedCurrentStandardSlot";

    assert (standard_box_);
    standard_box_->setStandardName(eval_man_.currentStandardName());

    updateButtons();
    updateStandardStack();
}

void EvaluationStandardTabWidget::addStandardSlot ()
{
    loginf << "EvaluationStandardTabWidget: addStandardSlot";

    bool ok;
    QString text =
        QInputDialog::getText(this, tr("Standard Name"),
                              tr("Specify a (unique) standard name:"), QLineEdit::Normal, "", &ok);

    if (ok && !text.isEmpty())
    {
        std::string name = text.toStdString();

        if (!name.size())
        {
            QMessageBox m_warning(QMessageBox::Warning, "Adding Standard Failed",
            "Standard has to have a non-empty name.", QMessageBox::Ok);
            m_warning.exec();
            return;
        }

        if (eval_man_.hasStandard(name))
        {
            QMessageBox m_warning(QMessageBox::Warning, "Adding Standard Failed",
            "Standard with this name already exists.", QMessageBox::Ok);
            m_warning.exec();
            return;
        }

        eval_man_.addStandard(name);
    }
}

void EvaluationStandardTabWidget::removeStandardSlot ()
{
    loginf << "EvaluationStandardTabWidget: removeStandardSlot";

    assert (eval_man_.hasCurrentStandard());
    eval_man_.deleteCurrentStandard();
}


void EvaluationStandardTabWidget::updateButtons()
{
    assert (add_button_);
    add_button_->setDisabled(false);
    assert (rename_button_);
    rename_button_->setDisabled(true);
    assert (copy_button_);
    copy_button_->setDisabled(true);
    assert (remove_button_);
    remove_button_->setDisabled(!eval_man_.hasCurrentStandard());
}

void EvaluationStandardTabWidget::updateStandardStack()
{
    assert(standards_widget_);

    string standard_name = eval_man_.currentStandardName();

    if (!standard_name.size())
    {
        while (standards_widget_->count() > 0)  // remove all widgets
            standards_widget_->removeWidget(standards_widget_->widget(0));
        return;
    }

    EvaluationStandard& standard = eval_man_.currentStandard();

    if (standards_widget_->indexOf(standard.widget()) < 0)
        standards_widget_->addWidget(standard.widget());

    standards_widget_->setCurrentWidget(standard.widget());
}
