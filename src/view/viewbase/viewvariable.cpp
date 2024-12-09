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

#include "viewvariable.h"
#include "variableview.h"

#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/variable/variableset.h"
#include "dbcontent/variable/metavariable.h"
#include "dbcontent/variable/variableselectionwidget.h"

#include "global.h"
#include "compass.h"
#include "logger.h"

const std::string ViewVariable::ParamDataVar     = "data_var";
const std::string ViewVariable::ParamDataVarName = "name";
const std::string ViewVariable::ParamDataVarDBO  = "dbo";

/**
 */
ViewVariable::ViewVariable(const std::string& id_str, 
                           int idx,
                           VariableView* view)
:   id_  (id_str)
,   idx_ (idx   )
,   view_(view  )
{
}

/**
 */
ViewVariable::~ViewVariable() = default;

/**
 */
std::string ViewVariable::regParamDBO() const
{
    return (ParamDataVar + (settings_.var_name.empty() ? "" : "_" + settings_.var_name + "_") + ParamDataVarDBO);
    
}

/**
 */
std::string ViewVariable::regParamName() const
{
    return (ParamDataVar + (settings_.var_name.empty() ? "" : "_" + settings_.var_name + "_") + ParamDataVarName);
}

/**
 */
const std::string& ViewVariable::id() const
{
    return id_;
}

/**
 */
std::string ViewVariable::description() const
{
    return (variableDBContent() + ": " + variableName());
}

/**
 */
bool ViewVariable::hasVariable () const
{
    if (settings_.data_var_dbo.empty() || settings_.data_var_name.empty())
        return false;

    if (settings_.data_var_dbo == META_OBJECT_NAME)
        return COMPASS::instance().dbContentManager().existsMetaVariable(settings_.data_var_name);
    else
        return COMPASS::instance().dbContentManager().dbContent(settings_.data_var_dbo).hasVariable(settings_.data_var_name);
}

/**
 */
bool ViewVariable::isMetaVariable () const
{
    return (settings_.data_var_dbo == META_OBJECT_NAME);
}

/**
 */
bool ViewVariable::isEmpty() const
{
    return (settings_.data_var_dbo.empty() && settings_.data_var_name.empty());
}

/**
 */
boost::optional<PropertyDataType> ViewVariable::dataType() const
{
    if (!hasVariable())
        return {};

    if (isMetaVariable())
        return metaVariable().dataType();

    return variable().dataType();
}

/**
 */
dbContent::Variable& ViewVariable::variable()
{
    assert (hasVariable());
    assert (!isMetaVariable());
    assert (COMPASS::instance().dbContentManager().dbContent(settings_.data_var_dbo).hasVariable(settings_.data_var_name));

    return COMPASS::instance().dbContentManager().dbContent(settings_.data_var_dbo).variable(settings_.data_var_name);
}

/**
 */
const dbContent::Variable& ViewVariable::variable() const
{
    assert (hasVariable());
    assert (!isMetaVariable());
    assert (COMPASS::instance().dbContentManager().dbContent(settings_.data_var_dbo).hasVariable(settings_.data_var_name));

    return COMPASS::instance().dbContentManager().dbContent(settings_.data_var_dbo).variable(settings_.data_var_name);
}

/**
*/
dbContent::Variable* ViewVariable::variablePtr()
{
    if (!hasVariable() || isMetaVariable())
        return nullptr;

    return &variable();
}

/**
 */
void ViewVariable::setVariable(dbContent::Variable& var, bool notify_changes)
{
    if (settings_.data_var_dbo == var.dbContentName() && 
        settings_.data_var_name == var.name())
        return;

    if (!view_)
    {
        settings_.data_var_dbo  = var.dbContentName();
        settings_.data_var_name = var.name();
        return;
    }

    loginf << "ViewVariable: dataVar: setting var '" << id_ << "' "
        << "of view '" << view_->getName() << " to "
        << "dbo " << var.dbContentName() << " name " << var.name();

    view_->preVariableChangedEvent(idx_, var.dbContentName(), var.name());

    view_->setParameter(settings_.data_var_dbo, var.dbContentName());
    view_->setParameter(settings_.data_var_name, var.name());

    view_->postVariableChangedEvent(idx_);

    assert (hasVariable());
    assert (!isMetaVariable());

    if (notify_changes)
        view_->notifyRefreshNeeded();
}

/**
 */
dbContent::MetaVariable& ViewVariable::metaVariable()
{
    assert (hasVariable());
    assert (isMetaVariable());

    return COMPASS::instance().dbContentManager().metaVariable(settings_.data_var_name);
}

/**
 */
const dbContent::MetaVariable& ViewVariable::metaVariable() const
{
    assert (hasVariable());
    assert (isMetaVariable());

    return COMPASS::instance().dbContentManager().metaVariable(settings_.data_var_name);
}

/**
*/
dbContent::MetaVariable* ViewVariable::metaVariablePtr()
{
    if (!hasVariable() || !isMetaVariable())
        return nullptr;

    return &metaVariable();
}

/**
 */
void ViewVariable::setMetaVariable(dbContent::MetaVariable& var, bool notify_changes)
{
    if (settings_.data_var_dbo == META_OBJECT_NAME && 
        settings_.data_var_name == var.name())
        return;

    if (!view_)
    {
        settings_.data_var_dbo  = META_OBJECT_NAME;
        settings_.data_var_name = var.name();
        return;
    }
    
    loginf << "ViewVariable: metaDataVar: setting metavar '" << id_ << "' "
           << "of view '" << view_->getName() << " to "
           << "name " << var.name();

    view_->preVariableChangedEvent(idx_, META_OBJECT_NAME, var.name());

    view_->setParameter(settings_.data_var_dbo, META_OBJECT_NAME);
    view_->setParameter(settings_.data_var_name, var.name());

    view_->postVariableChangedEvent(idx_);

    assert (hasVariable());
    assert (isMetaVariable());

    if (notify_changes)
        view_->notifyRefreshNeeded();
}

/**
 */
void ViewVariable::setEmpty(bool notify_changes)
{
    assert(settings_.show_empty_vars);

    if (settings_.data_var_dbo == "" && 
        settings_.data_var_name == "")
        return;

    if (!view_)
    {
        settings_.data_var_dbo  = "";
        settings_.data_var_name = "";
        return;
    }

    loginf << "ViewVariable: setEmpty: setting to empty";

    view_->preVariableChangedEvent(idx_, "", "");

    view_->setParameter(settings_.data_var_dbo , std::string());
    view_->setParameter(settings_.data_var_name, std::string());

    view_->postVariableChangedEvent(idx_);

    assert(!hasVariable());
    assert(isEmpty());

    if (notify_changes)
        view_->notifyRefreshNeeded();
}

/**
 */
dbContent::Variable* ViewVariable::getFor(const std::string& dbcontent_name)
{
    if (!hasVariable())
        return nullptr;

    dbContent::Variable* var = nullptr;

    if (isMetaVariable())
    {
        auto& meta_var = metaVariable();

        if (!meta_var.existsIn(dbcontent_name))
            return nullptr;

        var = &meta_var.getFor(dbcontent_name);
    }
    else
    {
        if (settings_.data_var_dbo != dbcontent_name)
            return nullptr;

        var = &variable();
    }

    assert(var);
    
    return var;
}

/**
*/
const dbContent::Variable* ViewVariable::getFor(const std::string& dbcontent_name) const
{
    if (!hasVariable())
        return nullptr;

    const dbContent::Variable* var = nullptr;

    if (isMetaVariable())
    {
        const auto& meta_var = metaVariable();
        auto& meta_var_unconst = const_cast<dbContent::MetaVariable&>(meta_var);

        if (!meta_var_unconst.existsIn(dbcontent_name))
            return nullptr;

        var = &meta_var_unconst.getFor(dbcontent_name);
    }
    else
    {
        if (settings_.data_var_dbo != dbcontent_name)
            return nullptr;

        var = &variable();
    }

    assert(var);
    
    return var;
}

/**
 */
const std::string& ViewVariable::variableDBContent() const
{
    return settings_.data_var_dbo;
}

/**
 */
const std::string& ViewVariable::variableName() const
{
    return settings_.data_var_name;
}

/**
 */
void ViewVariable::addToSet(dbContent::VariableSet& set, 
                            const std::string& dbcontent_name)
{
    if (hasVariable())
    {
        if (isMetaVariable())
        {
            dbContent::MetaVariable& meta_var = metaVariable();

            if (meta_var.existsIn(dbcontent_name) && !set.hasVariable(meta_var.getFor(dbcontent_name)))
                set.add(meta_var.getFor(dbcontent_name));
        }
        else
        {
            dbContent::Variable& var = variable();

            if (var.dbContentName() == dbcontent_name && !set.hasVariable(var))
                set.add(var);
        }
    }
}

/**
 */
void ViewVariable::setVariable(const dbContent::VariableSelectionWidget& selection,
                               bool notify_changes)
{
    if (selection.hasVariable())
        setVariable(selection.selectedVariable(), notify_changes);
    else if (selection.hasMetaVariable())
        setMetaVariable(selection.selectedMetaVariable(), notify_changes);
    else
        setEmpty(notify_changes);
}

/**
 */
void ViewVariable::configureWidget(dbContent::VariableSelectionWidget& selection)
{
    selection.showMetaVariables(settings_.show_meta_vars);
    selection.showEmptyVariable(settings_.show_empty_vars);

    if (!settings_.valid_data_types.empty())
        selection.showDataTypesOnly(std::vector<PropertyDataType>(settings_.valid_data_types.begin(),
                                                                  settings_.valid_data_types.end()));
}

/**
 */
void ViewVariable::updateWidget(dbContent::VariableSelectionWidget& selection)
{
    if (!hasVariable())
        return;

    if (isMetaVariable())
        selection.selectedMetaVariable(metaVariable());
    else
        selection.selectedVariable(variable());
}
