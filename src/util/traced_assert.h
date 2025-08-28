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

#define BOOST_ENABLE_ASSERT_HANDLER
#include <boost/assert.hpp>
#include <boost/stacktrace.hpp>

#include "msghandler.h"

#include <cstdlib>

#define traced_assert(expr) \
    (BOOST_LIKELY(!!(expr))? ((void)0): ::boost::assertion_failed(#expr, BOOST_CURRENT_FUNCTION, __FILE__, __LINE__, boost::stacktrace::stacktrace(), false), ::std::abort())

#define traced_assert_msg(expr, msg) \
    (BOOST_LIKELY(!!(expr))? ((void)0): ::boost::assertion_failed(msg, BOOST_CURRENT_FUNCTION, __FILE__, __LINE__, boost::stacktrace::stacktrace(), true), ::std::abort())

namespace boost
{
    inline void assertion_failed(char const * expr, 
                                 char const * function,
                                 char const * file, 
                                 long line,
                                 const boost::stacktrace::stacktrace& stack_trace,
                                 bool expr_is_message)
    {
        std::stringstream ss;
        ss << stack_trace;

        //compile message
        msghandler::Message msg;
        msg.severity    = msghandler::Severity::Abort;
        msg.content     = expr_is_message ? std::string(expr) : "Assertion '" + std::string(expr) + "' failed";
        msg.file        = std::string(file);
        msg.line        = (int)line;
        msg.stack_trace = ss.str();
        msg.user_level  = false;

        //log assert msg
        msghandler::MessageHandler::log(logerr, msg);

        //then abort
        std::abort();
    }

    inline void assertion_failed(char const * expr, 
                                 char const * function,
                                 char const * file, 
                                 long line)
    {
        std::stringstream ss;

        //compile message
        msghandler::Message msg;
        msg.severity    = msghandler::Severity::Abort;
        msg.content     = "Assertion '" + std::string(expr) + "' failed";
        msg.file        = std::string(file);
        msg.line        = (int)line;
        msg.stack_trace = ss.str();
        msg.user_level  = false;

        //log assert msg
        msghandler::MessageHandler::log(logerr, msg);

        //then abort
        std::abort();
    }
}