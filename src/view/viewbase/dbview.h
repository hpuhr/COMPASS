
#ifndef DBVIEW_H
#define DBVIEW_H

#include <QObject>

#include "bufferset.h"
#include "dbovariableset.h"
#include "view.h"

#include "boost/date_time/posix_time/posix_time.hpp"

class DBViewModel;
class DBViewWidget;
class DBOVariable;


/**
@brief Serves as base class for all DB driven views.

This class is intendent as a base class for all views that rely on data coming from the database,
mostly beeing provided in form of a Buffer. There are associated DBViewWidget and DBViewModel classes,
one should derive from too when creating a new DBView based view.

@todo The readlist should now be created automatically from the workflow.
 */
class DBView : public View
{
    Q_OBJECT
public:
    DBView( const std::string& class_id, const std::string& instance_id, ViewContainerWidget* w );
    virtual ~DBView();

    virtual void update( bool atOnce=false );
    virtual void clearData();
    virtual bool init();
    /// @brief Returns the view type as a string.
    virtual std::string viewType() const { return "DBView"; }
    virtual bool addData( BufferSet* data );
    virtual void timerEvent( QTimerEvent* e );

    void listen();
    void stopListening();
    bool listening() const;

    DBOVariableSet& getReadList () { return read_set_; }
    /// @brief Creates the needed readlist for this view.
    virtual void updateReadList ()=0;

    DBViewModel* getModel() { return (DBViewModel*)model_; }
    DBViewWidget* getWidget() { return (DBViewWidget*)widget_; }

    void addMinMaxVariable (DBOVariable *variable);

protected:
    /// The current readset, determines what should be loaded for this view in the database
    DBOVariableSet read_set_;
    /// Buffers to be read
    BufferSet new_data_;
    /// Buffers already read
    BufferSet data_;

    /// Dispatch interval time
    static const int update_time_=50;
    /// ID of the dispatch timer
    int timer_id_;

    /// Loading start time
    boost::posix_time::ptime loading_start_time_;
    /// Loading end time
    boost::posix_time::ptime loading_stop_time_;
    /// Current loading time
    double load_time_;

    /// Mutex guarding the data reading process
    boost::mutex data_mutex_;

    /// Variables to retrieve the minmax information for
    std::vector <DBOVariable*> min_max_variables_;
};

#endif //DBVIEW_H
