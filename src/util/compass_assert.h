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

#define compass_assert(expr) \
    (BOOST_LIKELY(!!(expr))? ((void)0): ::boost::assertion_failed(#expr, BOOST_CURRENT_FUNCTION, __FILE__, __LINE__, (std::stringstream() << boost::stacktrace::stacktrace()).str(), false))

#define compass_assert_msg(expr, msg) \
    (BOOST_LIKELY(!!(expr))? ((void)0): ::boost::assertion_failed(msg, BOOST_CURRENT_FUNCTION, __FILE__, __LINE__, (std::stringstream() << boost::stacktrace::stacktrace()).str(), true))

namespace boost
{
    inline void assertion_failed(char const * expr, 
                                 char const * function,
                                 char const * file, 
                                 long line,
                                 const std::string& stack_trace,
                                 bool expr_is_message)
    {
        //compile message
        msghandler::Message msg;
        msg.severity    = msghandler::Severity::Abort;
        msg.content     = expr_is_message ? std::string(expr) : "Assertion '" + std::string(expr) + "' failed";
        msg.file        = std::string(file);
        msg.line        = (int)line;
        msg.stack_trace = stack_trace;
        msg.user_level  = false;

        //report critical error
        msghandler::MessageHandler::reportCriticalError(msg);

        //handle exception
        msghandler::MessageHandler::handleException();
    }
}
