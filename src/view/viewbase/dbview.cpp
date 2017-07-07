
#include "dbview.h"
#include "dbviewmodel.h"
#include "viewwidget.h"
//#include "DBResultSetManager.h"
//#include "WorkerThreadManager.h"
#include "buffer.h"
#include "logger.h"
#include "dbovariable.h"

#include <cassert>


/**
@brief Constructor.
@param class_id Configurable class id.
@param instance_id Configurable instance id.
@param w ViewContainerWidget the view is embedded in, configurable parent.
 */
DBView::DBView( const std::string& class_id, const std::string& instance_id, ViewContainerWidget* w )
:   View( class_id, instance_id, w ),
    timer_id_( -1 ),
    load_time_( 0 )
{
    logdbg  << "DBView: constructor";
}

/**
@brief Destructor.
 */
DBView::~DBView()
{

}

/**
@brief Inits the view.
  */
bool DBView::init()
{
    return View::init();
}

/**
@brief Starts to listen for new data.

Will start the dispatcher timer event.
 */
void DBView::listen()
{
    if( !listening() )
        timer_id_ = startTimer( update_time_ );
}

/**
@brief Stops data listening.
 */
void DBView::stopListening()
{
    if( listening() )
    {
        killTimer( timer_id_ );
        timer_id_ = -1;
    }
}

/**
@brief Checks if the view is currently listening for new data.
@return True if the view is listening, otherwise false.
 */
bool DBView::listening() const
{
    return timer_id_ != -1;
}

/**
@brief Adds new buffers to the view.

The main interface to add new data.
@param data New buffers.
@return True if the data could be added, otherwise false.
 */
bool DBView::addData( BufferSet* data )
{
    boost::mutex::scoped_lock l( data_mutex_ );

    logdbg  << "DBView: addData";
    assert( data );

    //start listening
    if( timer_id_ == -1 )
        listen ();

    new_data_.addBuffers( data->getBuffers() );
    delete data;

    return true;
}

/**
@brief Clears the views data.
 */
void DBView::clearData ()
{
    boost::mutex::scoped_lock l( data_mutex_ );

    //empty buffers
    new_data_.clearAndDelete();
    data_.clearAndDelete();

    //clear model
    if( model_ )
        getModel()->clear();

    //restart loading timer
    loading_start_time_ = boost::posix_time::microsec_clock::local_time();
    load_time_ = 0.0;
    emit loadingStarted();
    emit loadingTime( load_time_ );
}

/**
@brief Dispatcher timer event.

Checks minmax information retrieval and feeds the model with new data.
@param e Event.
 */
void DBView::timerEvent( QTimerEvent* e )
{
    boost::mutex::scoped_lock l( data_mutex_ );
    logdbg  << "DBView: timerEvent";

    if (min_max_variables_.size() > 0) // any min//max of variables need to be loaded
    {
        logdbg << "DBView: timerEvent: waiting for " << min_max_variables_.size() << " min/max values";
        std::vector <DBOVariable*>::iterator it;

//        for (it = min_max_variables_.begin(); it != min_max_variables_.end(); it++)
//        {
//            assert ((*it));
//            if ((*it)->hasMinMaxInfo())
//            {
//                loginf << "DBView: timerEvent: erasing " << (*it)->id_;
//                it = min_max_variables_.erase(it);
//            }
//        }

        for ( it = min_max_variables_.begin(); it != min_max_variables_.end(); )
        {
          if ((*it)->hasMinMaxInfo())
          {
            it = min_max_variables_.erase(it);
          }
          else
          {
            ++it;
          }
        }

        if (min_max_variables_.size() > 0)
        {
            loginf << "DBView: timerEvent: still waiting for " << min_max_variables_.size() << " min/max values";

            loading_stop_time_= boost::posix_time::microsec_clock::local_time();
            boost::posix_time::time_duration diff = loading_stop_time_ - loading_start_time_;
            load_time_= diff.total_milliseconds()/1000.0;

            emit loadingTime( load_time_ );

            return;
        }
        else
            getModel()->completedMinMaxInfo();
    }

    if( new_data_.getSize() > 0 )
    {
        logdbg << "DBView: timerEvent: popping buffer";
        Buffer *buffer = new_data_.popBuffer();

        getModel()->addData( buffer->getShallowCopy() );
        data_.addBuffer( buffer );

        loading_stop_time_= boost::posix_time::microsec_clock::local_time();
        boost::posix_time::time_duration diff = loading_stop_time_ - loading_start_time_;
        load_time_= diff.total_milliseconds()/1000.0;

        emit loadingTime( load_time_ );
    }

    if (new_data_.getSize() == 0 && !DBResultSetManager::getInstance().isCurrentlyLoadingData()
            && WorkerThreadManager::getInstance().noJobs() )
    {
        loginf  << "DBView: " << getName().c_str() << ": loading done after " << load_time_ << " seconds";

        emit loadingFinished();
        stopListening ();
    }

    logdbg  << "DBView: timerEvent: done";
}

/**
@brief Starts immediate redraw or redraw from Buffer lists.

Updates the view either by redrawing through the model, which will tell the generators
to redraw themselves with their internal data, or by repushing the buffers to the model,
which will propagate them again through the whole workflow. Note that an immediate update will
most likely block the GUI, a deferred update will take more time.
@param atOnce If true an update through generators will be done, otherwise a buffer driven update.
 */
void DBView::update( bool atOnce )
{
    boost::mutex::scoped_lock l( data_mutex_ );

    if( atOnce )
      logdbg  << "DBView: update: Immediate update.";
    else
      logdbg  << "DBView: update: Interactive update.";

    if( !atOnce )
    {
        getModel()->clear();

        data_.addBuffers( new_data_.popBuffers() );
        new_data_.addBuffers( data_.popBuffers() );

        loading_start_time_ = boost::posix_time::microsec_clock::local_time();
        listen();
    }
    else
    {
        getModel()->redrawData();
    }
}

/**
@brief Adds a new variable to retrieve the minmax information for.

For some variables the minmax information has to be retrieved before beeing
able to load data in the model (the model relies on minmax information). Those
variables can be added here, so they can be retrieved before buffers are pushed to
the model.
@param variable The variable to retrieve the minmax information for.
  */
void DBView::addMinMaxVariable (DBOVariable *variable)
{
    logdbg << "DBView: addMinMaxVariable: var " << variable->id_;
    assert (!variable->hasMinMaxInfo());
    variable->buildMinMaxInfo();
    min_max_variables_.push_back (variable);
}
