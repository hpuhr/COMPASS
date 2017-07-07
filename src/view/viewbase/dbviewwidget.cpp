
#include "DBViewWidget.h"
#include "DBView.h"
#include "DBViewModel.h"
#include "ViewWorkflowEditDialog.h"
#include "WorkflowEditWidget.h"
#include "DOGenerator.h"


/*************************************************************************************
DBViewWidgetEventProcessor
**************************************************************************************/

/**
@brief Constructor.
@param widget View widget the event processor is part of.
  */
DBViewWidgetEventProcessor::DBViewWidgetEventProcessor( DBViewWidget* widget )
:   widget_( widget ),
    key_ctrl_pressed_( false )
{
}

/**
  */
DBViewWidgetEventProcessor::~DBViewWidgetEventProcessor()
{
}

/**
  */
bool DBViewWidgetEventProcessor::keyPressEvent( QKeyEvent* e )
{
    if( EventProcessor::keyPressEvent( e ) )
        return true;

    bool ok = false;
    if( e->key() == Qt::Key_Control )
    {
        key_ctrl_pressed_ = true;
    }
    else if( e->key() == Qt::Key_W )
    {
        //start workflow editor
        if( key_ctrl_pressed_ && widget_ && widget_->getView() )
        {
            DBViewModel* model = widget_->getView()->getModel();
            Workflow* wf = model->getWorkflow();

            ViewWorkflowEditDialog dlg( ViewWorkflowEditWidget::VIEW );
            dlg.setView( widget_->getView() );

            //add workflow
            dlg.addWorkflow( wf );

            //add all generators
            unsigned int i, n = model->numberOfGenerators();
            for( i=0; i<n; ++i )
            {
                DOGenerator* gen = model->getGenerator( i );
                if( gen->bufferGenerator() )
                    dlg.addGenerator( (DOGeneratorBuffer*)gen );
            }

            //update the tree view and start
            dlg.updateTree();
            dlg.exec();

            ok = true;
        }
    }

    return ok;
}

/**
  */
bool DBViewWidgetEventProcessor::keyReleaseEvent( QKeyEvent* e )
{
    if( EventProcessor::keyReleaseEvent( e ) )
        return true;

    bool ok = false;
    if( e->key() == Qt::Key_Control )
    {
        key_ctrl_pressed_ = false;
    }
    else if( e->key() == Qt::Key_W )
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
DBViewWidget::DBViewWidget( const std::string& class_id,
                            const std::string& instance_id,
                            Configurable* config_parent,
                            DBView* view,
                            QWidget* parent )
:   ViewWidget( class_id, instance_id, config_parent, view, parent ),
    do_manager_( NULL )
{
}

/**
@brief Destructor.
  */
DBViewWidget::~DBViewWidget()
{
}

/**
@brief Returns the view.
@return The view.
  */
DBView* DBViewWidget::getView()
{
    return (DBView*)view_;
}

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
DisplayObjectManager* DBViewWidget::getDOManager()
{
    return do_manager_;
}
