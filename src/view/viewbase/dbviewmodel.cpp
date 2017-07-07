
#include "DBViewModel.h"
#include "DBView.h"
#include "DBViewWidget.h"
#include "ViewWidget.h"
#include "Buffer.h"
#include "Workflow.h"
#include "DOGenerator.h"
#include "DOPoints.h"
#include "DOTiledPoints.h"
#include "DOBins.h"
#include "DOBins3D.h"
#include "DOLines.h"
#include "DOShape.h"
#include "DOAeronauticals.h"
#include "ComputationElement.h"
#include "DisplayObjectManager.h"

#include <stdexcept>

#include <boost/assign/list_of.hpp>

//Add strings for your generator here, to make it available in GUI elements.
DBViewModel::GeneratorTypeStringMap DBViewModel::generator_type_strings_ = boost::assign::map_list_of
        ( GENERATOR_POINTS_BUFFER, "GENERATOR_POINTS_BUFFER" )
        ( GENERATOR_POINTS_RAW   , "GENERATOR_POINTS_RAW"    )
        ( GENERATOR_POINTS_TILED , "GENERATOR_POINTS_TILED"  )
        ( GENERATOR_LINES_BUFFER , "GENERATOR_LINES_BUFFER"  )
        ( GENERATOR_LINES_RAW    , "GENERATOR_LINES_RAW"     )
        ( GENERATOR_BINS         , "GENERATOR_BINS"          )
        ( GENERATOR_BINS2D       , "GENERATOR_BINS2D"        )
        ( GENERATOR_SHAPE        , "GENERATOR_SHAPE"         )
        ( GENERATOR_AERONAUTICAL , "GENERATOR_AERONAUTICAL"  );


/**
@brief Constructor.
@param class_id Configurable class id.
@param instance_id Configurable instance id.
@param view The view the model is part of, configurable parent.
*/
DBViewModel::DBViewModel( std::string class_id, std::string instance_id, DBView* view )
:   ViewModel( class_id, instance_id, view ),
    workflow_( NULL )
{
    connect( view, SIGNAL(loadingFinished()), this, SLOT(loadingFinished()) );

    //get a valid display object manager from the widget (MUST be provided!)
    do_manager_ = view->getWidget()->getDOManager();
    assert( do_manager_ );

    //will restore generators and their display objects, needs the ViewWidget to be already constructed
    createSubConfigurables();

    //will reconnect the layers with the display objects restored in the step before
    do_manager_->initConfigurableLayers();
}

/**
@brief Destructor.
*/
DBViewModel::~DBViewModel()
{
    if( workflow_)
        delete workflow_;

    clearGenerators();
}


/**
@brief Redraws the data via the generators.
  */
void DBViewModel::redrawData()
{
    unsigned int i, n = generators_.size();
    for( i=0; i<n; ++i )
        generators_[ i ]->redraw();

    //don't forget to reapply the selection after redraw
    enableSelection( true );
}

/**
@brief Clears the data and updates the display if wished.
@param update If true updates the display, if false not. Not used, I know....
  */
void DBViewModel::clear( bool update )
{
    //clear workflow
    workflow_->clear();

    //clear generators, just to be sure (>attached< generators should be cleared automatically with the workflow)
    unsigned int i, n = generators_.size();
    for( i=0; i<n; ++i )
        generators_[ i ]->clearSlot();
}

/**
@brief Adds buffer data to the model.

Main interface to push data to the model. Can be used to do some
prechecks on the data.
@param buffer Buffer to be worked by the model.
@return True if the buffer has been added, false otherwise.
*/
bool DBViewModel::addData( Buffer* buffer )
{
    assert( buffer );
    return processBuffer( buffer );
}

/**
@brief Processes the given buffer.

Reimplement for specific behaviour. At default the buffer is just pushed to
the workflow.
@param buffer Buffer to be processed by the model.
@return True if the buffer has been processed, false otherwise.
*/
bool DBViewModel::processBuffer( Buffer* buffer )
{
    workflow_->process( buffer );
    return true;
}

/**
@brief Triggered if a buffer has finished in a DOGeneratorBuffer.
  */
void DBViewModel::bufferFinished()
{
}

/**
@brief Triggered if the last buffer of a certain DBO type has finished in a DOGeneratorBuffer.
  */
void DBViewModel::lastBufferFinished()
{
    //apply selection after redraw
    enableSelection( true );
}

/**
@brief Triggered if loading from the database has finished.

This does only involve the database side, it does not mean that drawing has finished!
  */
void DBViewModel::loadingFinished()
{
}

/**
@brief Applies the selection through the generators.
@param types Involved DBO types to be checked with the generators.
@param sel If true selects the current selection items, otherwise deselects them.
  */
void DBViewModel::updateSelectionGenerators( const std::set<DB_OBJECT_TYPE>& types, bool sel )
{
    //check if the generators obtain one of the involved DBO types
    unsigned int i, n = generators_.size();
    for( i=0; i<n; ++i )
    {
        if( generators_[ i ]->bufferGenerator() )
            ((DOGeneratorBuffer*)generators_[ i ])->markForSelection( types );
    }

    //apply the selection (only applied if previously marked)
    for( i=0; i<n; ++i )
    {
        if( generators_[ i ]->bufferGenerator() )
            ((DOGeneratorBuffer*)generators_[ i ])->select( sel );
    }
}

/**
@brief Applies the current selection.
@param enable If true selects the current selection items, otherwise deselects them.
  */
void DBViewModel::enableSelection( bool enable )
{
    //collect involved DBO types from selection
    std::set<DB_OBJECT_TYPE> types;
    ViewSelectionEntries& entries = ViewSelection::getInstance().getEntries();
    ViewSelectionEntries::const_iterator it, itend = entries.end();
    for( it=entries.begin(); it!=itend; ++it )
        types.insert( (DB_OBJECT_TYPE)( it->id_.first ) );

    //update selectio in involved generators
    updateSelectionGenerators( types, enable );
}

/**
@brief Adds a new generator to the model.
@param type Generator id.
@param elem Computation element to attach the generator to.
@return The created generator.
  */
DOGenerator* DBViewModel::addDOGenerator( GeneratorType type, ComputationElement* elem )
{
    if( !do_manager_ )
        throw std::runtime_error( "DBViewModel: addDOGenerator: No display object manager set." );

    std::string elem_name = "";
    if( elem )
        elem_name = elem->name();

    //attach the type to the class name for later identification
    Configuration& config = addNewSubConfiguration( "DOGenerator_" + QString::number( (int)type ).toStdString() );

    config.addParameterString( "computation", elem_name );
    generateSubConfigurable( config.getClassId(), config.getInstanceId() );

    return generators_.back();
}

/**
@brief Returns the number of generators in the model.
@return Number of generators in the model.
  */
unsigned int DBViewModel::numberOfGenerators() const
{
    return generators_.size();
}

/**
@brief Returns the idx'th generator of the model.
@param idx Index of the desired generator.
@return The desired generator.
  */
DOGenerator* DBViewModel::getGenerator( int idx )
{
    if( idx < 0 || idx >= (signed)generators_.size() )
        throw std::runtime_error( "DBViewModel: getGenerator: Index out of bounds." );
    return generators_[ idx ];
}

/**
@brief Clears all generators.
  */
void DBViewModel::clearGenerators()
{
    unsigned int i, n = generators_.size();
    for( i=0; i<n; ++i )
        delete generators_[ i ];
    generators_.clear();
}

/**
@brief Removes AND DELETES the given generator.
@param generator Generator to remove and delete.
  */
void DBViewModel::removeGenerator( DOGenerator* generator )
{
    unsigned int i, n = generators_.size();
    for( i=0; i<n; ++i )
    {
        if( generators_[ i ] == generator )
        {
            generators_.erase( generators_.begin()+i );
            delete generator;
            return;
        }
    }

    throw std::runtime_error( "DBViewModel: removeGenerator: Given generator not present." );
}

/**
@brief Triggered when the minmax retrieval has finished.

Reimplement for specific behaviour.
  */
void DBViewModel::completedMinMaxInfo ()
{
    logerr << "DBViewModel: completedMinMaxInfo: forgot to override this function";
}

/**
@brief Generates the subconfigurables.

The workflow NEEDS to be created FIRST, THEN the generators. On restore the generators
need the computations they are attached to.
Add your new generators here.
  */
void DBViewModel::generateSubConfigurable( std::string class_id, std::string instance_id )
{
    //alphabetical, create workflow first
    if( class_id == "ANiceWorkflow" )
    {
        if( workflow_ )
            delete workflow_;
        workflow_ = new Workflow( class_id, instance_id, this );
    }

    QString str( class_id.c_str() );
    if( str.startsWith( "DOGenerator" ) )
    {
        //get id from classname
        bool ok = false;
        int type = str.split( "_" ).at( 1 ).toInt( &ok );
        if( !ok )
            throw std::runtime_error( "DBViewModel: generateSubConfigurable: Could not decode generator id from " + class_id );

        //create generators from type, add here
        DOGenerator* generator;
        switch( type )
        {
            case GENERATOR_POINTS_BUFFER:
                generator = new DOPointsGeneratorBuffer( class_id, instance_id, this, do_manager_ );
                break;
            case GENERATOR_POINTS_RAW:
                generator = new DOPointsGeneratorRaw( class_id, instance_id, this, do_manager_ );
                break;
            case GENERATOR_POINTS_TILED:
                generator = new DOTiledPointsGeneratorBuffer( class_id, instance_id, this, do_manager_ );
                break;
            case GENERATOR_LINES_BUFFER:
                generator = new DOLinesGeneratorBuffer( class_id, instance_id, this, do_manager_ );
                break;
            case GENERATOR_LINES_RAW:
                generator = new DOLinesGeneratorRaw( class_id, instance_id, this, do_manager_ );
                break;
            case GENERATOR_BINS:
                generator = new DOBinsGeneratorBuffer( class_id, instance_id, this, do_manager_ );
                break;
            case GENERATOR_BINS2D:
                generator = new DOBins3DGeneratorBuffer( class_id, instance_id, this, do_manager_ );
                break;
            case GENERATOR_SHAPE:
                generator = new DOShapeGenerator( class_id, instance_id, this, do_manager_ );
                break;
            case GENERATOR_AERONAUTICAL:
                generator = new DOAeronauticalGenerator( class_id, instance_id, this, do_manager_ );
                break;
            default:
                throw std::runtime_error( "DBViewModel: addDOGenerator: Unknown generator type." );
                return;
        }

        //store the model in the generator
        generator->setModel( this );

        generators_.push_back( generator );

        //connect buffer generators and attach them to computations if needed
        if( generator->bufferGenerator() )
        {
            DOGeneratorBuffer* gen_buffer = (DOGeneratorBuffer*)generator;
            std::string computation = gen_buffer->getComputationName();
            if( workflow_->hasComputation( computation ) )
                gen_buffer->attachToComputation( workflow_->getComputation( computation ) );
            connect( gen_buffer, SIGNAL(lastBufferFinishedSignal()), this, SLOT(lastBufferFinished()) );
            connect( gen_buffer, SIGNAL(bufferFinishedSignal()), this, SLOT(bufferFinished()) );
        }

        //every generator is a potential label source
        if( do_manager_ && do_manager_->getLabels() )
            do_manager_->getLabels()->addLabelSource( generator );
    }
}

/**
  */
void DBViewModel::checkSubConfigurables()
{
    if( !workflow_ )
    {
        Configuration& config = addNewSubConfiguration( "ANiceWorkflow", "ANiceWorkflow0" );
        config.addParameterString( "name", "ViewWorkflow" );
        generateSubConfigurable( config.getClassId(), config.getInstanceId() );
    }
}
