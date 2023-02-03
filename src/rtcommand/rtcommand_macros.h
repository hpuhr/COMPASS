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
#define DECLARE_RTCOMMAND(Name, Description)                                     \
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
#define DECLARE_RTCOMMAND_NOOPTIONS                                                            \
protected:                                                                                     \
    virtual bool collectOptions_impl(OptionsDescription& options) override { return true; }    \
    virtual bool assignVariables_impl(const VariablesMap& variables) override { return true; }

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
#define DECLARE_RTCOMMAND_OPTIONS                                              \
protected:                                                                     \
    virtual bool collectOptions_impl(OptionsDescription& options) override;    \
    virtual bool assignVariables_impl(const VariablesMap& variables) override;

/**
 * Can be used when deriving RTCommand::assignVariables_impl() to either retrieve a value 
 * from the given set of variables or return with false.
 */
#define RTCOMMAND_GET_VAR_OR_FAIL(Variables, Name, Type, Var) \
    if (!Variables.count(Name))                               \
        return false;                                         \
    Var = Variables[Name].as<Type>();

#define RTCOMMAND_GET_QSTRING_OR_FAIL(Variables, Name, Var)                 \
    {                                                                       \
        std::string _##Var;                                                 \
        RTCOMMAND_GET_VAR_OR_FAIL(Variables, Name, std::string, _##Var)     \
        Var = QString::fromStdString(_##Var);                               \
    }

#define ADD_RTCOMMAND_OPTIONS(Options)     \
    namespace po = boost::program_options; \
    Options.add_options()
