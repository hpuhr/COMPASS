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


#ifndef DBVIEWMODEL_H
#define DBVIEWMODEL_H

#include <QObject>

#include "Global.h"
#include "ViewModel.h"

class DBView;
class Buffer;
class Workflow;
class DOGenerator;
class ComputationElement;
class DisplayObjectManager;

/**
@brief ViewModel for database driven views.

A DBViewModel contains a Workflow that transforms the incoming data for the view.
The transformed data can be viewed by one or more DOGenerators that are attached
to the workflow's ComputationElements. The view specific workflow and the generators
should be created and added in the reimplemented createWorkflow() method. They may also
be added dynamically at runtime e.g. through the ViewWorkflowEditWidget, but they should
be initialized to some default behaviour anyway.

New generators have to be registered via addDOGenerator(). This method will need a generator
ID to construct the right generator. Generator ID's are defined in the GeneratorType enum.
Newly implemented generators must be assigned a new ID in this class, in order to be able to
add them to the model.

Selection updates are handled via the generators at default, by marking them for the selection
first, depending on their stored buffer DBO types, and then updating the selection in the marked
generators.

Add a new generator:
- Add a new generator id in .h (GeneratorType)
- Add a new string type at the beginning of .cpp
- Add handling for the specific generator type in generateSubConfigurable()

@todo Not consistent: Generators are not always database driven, but they are introduced in
DBViewModel. Maybe it would be more consistent to introduce a GeneratorModel... and derive from this
one for DBViewModel.
  */
class DBViewModel : public ViewModel
{
    Q_OBJECT
  public:
    /// ID's for generators, add new generator ID's here
    enum GeneratorType
    {
        GENERATOR_POINTS_BUFFER = 0,
        GENERATOR_POINTS_RAW,
        GENERATOR_POINTS_TILED,
        GENERATOR_LINES_BUFFER,
        GENERATOR_LINES_RAW,
        GENERATOR_BINS,
        GENERATOR_BINS2D,
        GENERATOR_SHAPE,
        GENERATOR_AERONAUTICAL
    };

    typedef std::map<GeneratorType, std::string> GeneratorTypeStringMap;
    typedef std::vector<DOGenerator*> Generators;

    DBViewModel(std::string class_id, std::string instance_id, DBView* view);
    virtual ~DBViewModel();

    virtual void redrawData();
    virtual void clear(bool update = true);
    bool addData(Buffer* buffer);

    /// @brief creates the workflow for the view, reimplement and construct your workflow here
    virtual void createWorkflow() {}

    DBView* getView() { return (DBView*)view_; }

    virtual void completedMinMaxInfo();

    virtual void generateSubConfigurable(std::string class_id, std::string instance_id);

    /// @brief Checks if the model obtains a Workflow, reimplemented for convenience
    bool hasWorkflow() const { return true; }
    /// @brief Returns the models Workflow
    Workflow* getWorkflow() { return workflow_; }

    DOGenerator* addDOGenerator(GeneratorType type, ComputationElement* elem = NULL);
    unsigned int numberOfGenerators() const;
    DOGenerator* getGenerator(int idx);
    void removeGenerator(DOGenerator* generator);

    /// @brief Returns the generator ID's as strings
    static const GeneratorTypeStringMap& getPossibleGenerators() { return generator_type_strings_; }

  protected slots:
    virtual void bufferFinished();
    virtual void lastBufferFinished();
    virtual void loadingFinished();
    virtual void enableSelection(bool enable);

  protected:
    virtual void checkSubConfigurables();
    virtual bool processBuffer(Buffer* buffer);
    void clearGenerators();
    void updateSelectionGenerators(const std::set<DB_OBJECT_TYPE>& types, bool sel = true);

    /// The display object manager in use
    DisplayObjectManager* do_manager_;
    /// The added generators
    Generators generators_;
    /// The models Workflow
    Workflow* workflow_;

    /// The generator ID's as strings
    static GeneratorTypeStringMap generator_type_strings_;
};

#endif  // DBVIEWMODEL_H
