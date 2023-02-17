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

#pragma once

/**
 * Declares a new runtime command.
 * Add preferably at end of command class declaration.
 * 
 * struct RTCommandExample : public RTCommand
 * {
 *     ...
 *     DECLARE_RTCOMMAND(example, "just an example command")
 * };
 */
#define DECLARE_RTCOMMAND(Name, Description)                                          \
public:                                                                               \
    static QString staticName() { return #Name; }                                     \
    static QString staticDescription() { return Description; }                        \
    static void init() { is_registered_ = true; }                                     \
protected:                                                                            \
    virtual QString name_impl() const override { return staticName(); }               \
    virtual QString description_impl() const override { return staticDescription(); } \
private:                                                                              \
    static bool is_registered_;

/**
 * Add on top of cpp file (if possible outside of any namespace).
 * 
 * #include "rtcommandexample.h"
 * 
 * REGISTER_RTCOMMAND({any_needed_namespace::}RTCommandExample)
 * 
 * RTCommandExample::RTCommandExample()
 * {
 *     ...
 */
#define REGISTER_RTCOMMAND(Class) \
    bool Class::is_registered_ = rtcommand::RTCommandRegistry::instance().registerCommand(Class::staticName(), Class::staticDescription(), [] () { return new Class; });

/**
 * Overrides the abstract option related methods with empty behaviour. 
 * Add preferably at end of command class declaration.
 * 
 * struct RTCommandExample : public RTCommand
 * {
 *     ...
 *     DECLARE_RTCOMMAND(example, "just an example command")
 *     DECLARE_RTCOMMAND_NOOPTIONS
 * };
 */
#define DECLARE_RTCOMMAND_NOOPTIONS                                                 \
protected:                                                                          \
    virtual void collectOptions_impl(OptionsDescription& options,                   \
                                     PosOptionsDescription& positional) override {} \
    virtual void assignVariables_impl(const VariablesMap& variables) override {}

/**
 * Overrides the abstract option related methods, to be implemented in source.
 * Add preferably at end of command class declaration.
 * 
 * struct RTCommandExample : public RTCommand
 * {
 *     ...
 *     DECLARE_RTCOMMAND(example, "just an example command")
 *     DECLARE_RTCOMMAND_OPTIONS
 * };
 */
#define DECLARE_RTCOMMAND_OPTIONS                                                 \
protected:                                                                        \
    virtual void collectOptions_impl(OptionsDescription& options,                 \
                                     PosOptionsDescription& positional) override; \
    virtual void assignVariables_impl(const VariablesMap& variables) override;

/**
 * Can be used when deriving RTCommand::assignVariables_impl() to either retrieve a value 
 * from the given set of variables or return with false.
 */
#define RTCOMMAND_CHECK_VAR(Variables, Name, BoolVar) \
    BoolVar = Variables.count(Name) > 0;

#define RTCOMMAND_GET_VAR_OR_THROW(Variables, Name, Type, Var)                                         \
    if (!Variables.count(Name))                                                                        \
        throw std::runtime_error(std::string("Could not retrieve rtcommand variable '") + Name + "'"); \
    Var = Variables[Name].as<Type>();

#define RTCOMMAND_GET_QSTRING_OR_THROW(Variables, Name, Var)             \
    {                                                                    \
        std::string _##Var;                                              \
        RTCOMMAND_GET_VAR_OR_THROW(Variables, Name, std::string, _##Var) \
        Var = QString::fromStdString(_##Var);                            \
    }

/**
 * Can be used when deriving RTCommand::collectOptions_impl() to add options and positional option mappings.
 */
#define ADD_RTCOMMAND_OPTIONS(Options)     \
    namespace po = boost::program_options; \
    Options.add_options()

#define ADD_RTCOMMAND_POS_OPTION(PosOptions, Name, PosInt) \
    PosOptions.add(Name, PosInt);

/**
 * Can be used when overriding RTCommand::valid() to check for certain conditions.
 */
#define CHECK_RTCOMMAND_INVALID_CONDITION(ErrCondition, ErrMsg) \
    if (ErrCondition)                                           \
    {                                                           \
        return rtcommand::IsValid(false, ErrMsg);               \
    }
