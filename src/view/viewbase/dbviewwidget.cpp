/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "DBViewWidget.h"

#include "DBView.h"
#include "DBViewModel.h"
#include "DOGenerator.h"
#include "ViewWorkflowEditDialog.h"
#include "WorkflowEditWidget.h"

/*************************************************************************************
DBViewWidgetEventProcessor
**************************************************************************************/

/**
@brief Constructor.
@param widget View widget the event processor is part of.
  */
DBViewWidgetEventProcessor::DBViewWidgetEventProcessor(DBViewWidget* widget)
    : widget_(widget), key_ctrl_pressed_(false)
{
}

/**
 */
DBViewWidgetEventProcessor::~DBViewWidgetEventProcessor() {}

/**
 */
bool DBViewWidgetEventProcessor::keyPressEvent(QKeyEvent* e)
{
    if (EventProcessor::keyPressEvent(e))
        return true;

    bool ok = false;
    if (e->key() == Qt::Key_Control)
    {
        key_ctrl_pressed_ = true;
    }
    else if (e->key() == Qt::Key_W)
    {
        // start workflow editor
        if (key_ctrl_pressed_ && widget_ && widget_->getView())
        {
            DBViewModel* model = widget_->getView()->getModel();
            Workflow* wf = model->getWorkflow();

            ViewWorkflowEditDialog dlg(ViewWorkflowEditWidget::VIEW);
            dlg.setView(widget_->getView());

            // add workflow
            dlg.addWorkflow(wf);

            // add all generators
            unsigned int i, n = model->numberOfGenerators();
            for (i = 0; i < n; ++i)
            {
                DOGenerator* gen = model->getGenerator(i);
                if (gen->bufferGenerator())
                    dlg.addGenerator((DOGeneratorBuffer*)gen);
            }

            // update the tree view and start
            dlg.updateTree();
            dlg.exec();

            ok = true;
        }
    }

    return ok;
}

/**
 */
bool DBViewWidgetEventProcessor::keyReleaseEvent(QKeyEvent* e)
{
    if (EventProcessor::keyReleaseEvent(e))
        return true;

    bool ok = false;
    if (e->key() == Qt::Key_Control)
    {
        key_ctrl_pressed_ = false;
    }
    else if (e->key() == Qt::Key_W)
    {
        ok = true;
    }

    return ok;
}

/*************************************************************************************
DBViewWidget
**************************************************************************************/

/**
@brief Constructor.
@param class_id Configurable class id.
@param instance_id Configurable instance id.
@param config_parent Configurable parent.
@param view The view the view widget is part of.
@param parent The widgets parent.
  */
DBViewWidget::DBViewWidget(const std::string& class_id, const std::string& instance_id,
                           Configurable* config_parent, DBView* view, QWidget* parent)
    : ViewWidget(class_id, instance_id, config_parent, view, parent), do_manager_(NULL)
{
}

/**
@brief Destructor.
  */
DBViewWidget::~DBViewWidget() {}

/**
@brief Returns the view.
@return The view.
  */
DBView* DBViewWidget::getView() { return (DBView*)view_; }

/**
@brief Returns the event processor.
@return The event processor.
  */
DBViewWidgetEventProcessor* DBViewWidget::getEventProcessor()
{
    return (DBViewWidgetEventProcessor*)event_processor_;
}

/**
@brief Returns the event processor.
@return The event processor.
  */
DisplayObjectManager* DBViewWidget::getDOManager() { return do_manager_; }
