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

#include "msghandler.h"
#include "compass.h"
#include "dbinterface.h"
#include "stringconv.h"

#include <QMessageBox>
#include <QApplication>
#include <QMetaObject>
#include <QThread>

#include <boost/thread/mutex.hpp>

namespace msghandler
{

Message MessageHandler::critical_error_msg_     = Message();
bool    MessageHandler::critical_error_msg_set_ = false;

/**
 */
std::string MessageHandler::severityToString(Severity severity)
{
    switch(severity)
    {
        case Severity::Info:
            return "info";
        case Severity::Warning:
            return "warning";
        case Severity::Error:
            return "error";
        case Severity::Critical:
            return "critical";
        case Severity::Abort:
            return "abort";
    }
    return "";
}

/**
 */
LogStream MessageHandler::getStream(log4cpp::CategoryStream& strm,
                                    Severity severity,
                                    const std::string& component,
                                    int err_code,
                                    const std::string& file,
                                    int line,
                                    const nlohmann::json& info,
                                    const std::string& stack_trace,
                                    bool user_level)
{
    auto cb = [ &strm, severity, component, err_code, file, line, info, stack_trace, user_level ] (const std::string& content)
    {
        Message msg;
        msg.content     = content;
        msg.severity    = severity;
        msg.component   = component;
        msg.err_code    = err_code;
        msg.file        = file;
        msg.line        = line;
        msg.info        = info;
        msg.stack_trace = stack_trace;
        msg.user_level  = user_level;

        //log message on stream end
        MessageHandler::log(strm, msg);
    };

    return LogStream(cb);
}

/**
 */
void MessageHandler::log(log4cpp::CategoryStream& strm,
                         const Message& msg)
{
#if 0
    strm << "[Logged Message] (user level = " << msg.user_level << ")\n"
         << "severity:     " << MessageHandler::severityToString(msg.severity) << "\n"
         << "component:    " << msg.component << "\n"
         << "errcode:      " << msg.err_code << "\n"
         << "file:         " << msg.file << (msg.line >= 0 ? " (Line " + std::to_string(msg.line) + ")" : "") << "\n"
         << "info:         " << (msg.info.is_null() ? "-" : msg.info.dump()) << "\n"
         << "stacktrace:   " << (msg.stack_trace.empty() ? "-" : "\n" + msg.stack_trace) << "\n"
         << "show to user: " << (msg.user_level ? "true" : "false") << "\n"
         << "abort app:    " << (msg.severity == Severity::Abort ? "true" : "false") << "\n"
         << msg.content
         << "\n";
#endif

    if (msg.user_level)
        MessageHandler::handleUserLevelMessage(strm, msg);
    else
        MessageHandler::handleSystemLevelMessage(strm, msg);
}

/**
 */
void MessageHandler::logMessageFancy(log4cpp::CategoryStream& strm,
                                     const Message& msg)
{
    if (msg.severity == Severity::Info)
    {
        strm << msg.content;
    }
    else if (msg.severity == Severity::Warning ||
             msg.severity == Severity::Error)
    {
        strm << msg.file << " Line" << msg.line << ": " << msg.content;
    }
    else if (msg.severity == Severity::Critical ||
             msg.severity == Severity::Abort)
    {
        bool aborting = msg.severity == Severity::Abort;
        bool show_st  = aborting && !msg.stack_trace.empty();

        strm << "Encountered critical error" << (aborting ? ", going into shutdown" : "") << "\n"
             << "\n"
             << "Error:       " << (msg.content.empty() ? "Unknown error" : msg.content) << "\n"
             << "Error Code:  " << msg.err_code << "\n"
             << "Component:   " << (msg.component.empty() ? "Unknown" : msg.component) << "\n"
             << "File:        " << msg.file << "\n"
             << "Line:        " << msg.line << "\n"
             << (show_st ? "\n" : "")
             << (show_st ? msg.stack_trace : "")
             << (show_st ? "\n" : "")
             << (aborting ? "Aborting..." : "");
    }
}

/**
 */
bool MessageHandler::shutdownCOMPASS()
{
    //@TODO
    return true;
}

/**
 */
void MessageHandler::addToTaskLog(const Message& msg)
{
    if (!COMPASS::instance().dbInterface().ready())
        return;

    //execute in thread space of QApplication (main thread)
    if (msg.severity == Severity::Info)
    {
        QMetaObject::invokeMethod(qApp, 
                                  [ msg ] () { COMPASS::instance().logInfo(msg.component, {}, msg.info) << msg.content; }, 
                                  Qt::AutoConnection);
    }
    else if (msg.severity == Severity::Warning)
    {
        QMetaObject::invokeMethod(qApp, 
                                  [ msg ] () { COMPASS::instance().logWarn(msg.component, {}, msg.info) << msg.content; }, 
                                  Qt::AutoConnection);
    }
    else if (msg.severity == Severity::Error)
    {
        QMetaObject::invokeMethod(qApp, 
                                  [ msg ] () { COMPASS::instance().logError(msg.component, msg.err_code, msg.info) << msg.content; }, 
                                  Qt::AutoConnection);
    }
}

/**
 */
bool MessageHandler::showMessage(const Message& msg)
{
    try
    {
        // no gui application no ui
        if (!QApplication::instance() || QApplication::closingDown())
            return false;

        // get some parent (if existing)
        auto parent = QApplication::activeWindow();

        if (msg.severity == Severity::Info)
        {
            QMessageBox::information(parent, "Information", QString::fromStdString(msg.content));
        }
        else if (msg.severity == Severity::Warning)
        {
            QMessageBox::warning(parent, "Warning", QString::fromStdString(msg.content));
        }
        else if (msg.severity == Severity::Error)
        {
            QMessageBox::critical(parent, "Error", QString::fromStdString(msg.content));
        }
        else if (msg.severity == Severity::Critical ||
                msg.severity == Severity::Abort)
        {
            if (!showAbortMessage(msg))
                return false;
        }
    }
    catch(...)
    {
        return false;
    }

    return true;
}

/**
 */
bool MessageHandler::showAbortMessage(const Message& msg)
{
    try
    {
        // no gui application no ui
        if (!QApplication::instance() || QApplication::closingDown())
            return false;

        bool is_shutdown = msg.severity == Severity::Abort;

        auto parent = QApplication::activeWindow();

        std::string component = (msg.component.empty() ? "Unknown" : msg.component);

        std::stringstream ss;
        ss << "<html><body><table>";
        ss << "<tr><td><b>Error:      </b></td><td>" << msg.content  << "</td></tr>";
        ss << "<tr><td><b>Code:       </b></td><td>" << msg.err_code << "</td></tr>";
        ss << "<tr><td><b>Component:  </b></td><td>" << component    << "</td></tr>";
        ss << "<tr><td><b>File:       </b></td><td>" << msg.file     << "</td></tr>";
        ss << "<tr><td><b>Line:       </b></td><td>" << msg.line     << "</td></tr>";
        ss << "</table></body></html>";

        QString info = QString::fromStdString(ss.str());
        QString txt;
        
        if (is_shutdown)
        {
            txt = "The application has encountered a critical error and will shut down.";
        }
        else
        {
            txt = "The application has encountered a critical error";
        }

        QMessageBox msg_box(parent);
        msg_box.setIcon(QMessageBox::Critical);
        msg_box.setTextFormat(Qt::TextFormat::RichText);
        msg_box.setWindowTitle("Critical Error");
        msg_box.setText(txt);
        msg_box.setInformativeText(info);
        msg_box.setStandardButtons(QMessageBox::Button::Ok);
        msg_box.setDefaultButton(QMessageBox::Button::Ok);

        msg_box.exec();
    }
    catch(...)
    {
        return false;
    }
    
    return true;
}

/**
 */
bool MessageHandler::compileBugReport(const Message& msg)
{
    loginf << "Compiling bug report";

    //@TODO
    return true;
}

/**
 */
void MessageHandler::handleSystemLevelMessage(log4cpp::CategoryStream& strm,
                                              const Message& msg)
{
    //log message first
    MessageHandler::logMessageFancy(strm, msg);

    //report critical error
    if (msg.severity == Severity::Critical)
        MessageHandler::reportCriticalError(msg);
}

/**
 */
void MessageHandler::handleUserLevelMessage(log4cpp::CategoryStream& strm,
                                            const Message& msg)
{
    //log message first
    MessageHandler::logMessageFancy(strm, msg);

    //add to task log
    MessageHandler::addToTaskLog(msg);

    //report critical error
    if (msg.severity == Severity::Critical)
        MessageHandler::reportCriticalError(msg);
}

/**
 * Reports a critical error to the message handler.
 * - thread-safe
 * - critical error can only be set once
 */
void MessageHandler::reportCriticalError(const Message& msg)
{
    static boost::mutex m;
    boost::mutex::scoped_lock lock(m);

    if (critical_error_msg_set_)
        return;

    critical_error_msg_set_ = true;
    critical_error_msg_     = msg;
}

/**
 */
bool MessageHandler::hasCriticalError()
{
    return critical_error_msg_set_;
}

/**
 */
Message MessageHandler::criticalError()
{
    return critical_error_msg_;
}

/**
 * Handles any set critical error.
 * - thread-safe
 */
bool MessageHandler::handleCriticalError()
{
    static boost::mutex m;
    boost::mutex::scoped_lock lock(m);

    if (!critical_error_msg_set_)
        return false;

    auto msg = critical_error_msg_;

    //callback to be executed on main thread
    auto cb = [ msg ] ()
    {
        //log exception
        MessageHandler::logMessageFancy(logerr, msg);

        //show UI
        MessageHandler::showAbortMessage(msg);
    };

    //queue exception handling to main thread's event loop
    //(threads will wait for the main thread to execute it => do not call this if the main thread is blocked!)
    QMetaObject::invokeMethod(qApp, cb, qApp->thread() == QThread::currentThread() ? Qt::DirectConnection : Qt::BlockingQueuedConnection);

    //handled => reset flag
    critical_error_msg_set_ = false;

    return true;
}

/**
 * Handles an exception.
 * - thread-safe
 * - non-reentrant
 * - handling of the exception is passed to main thread
 * - threads go to sleep until abort() is called from the main thread
 */
void MessageHandler::handleException()
{
    static bool handled = false;
    static boost::mutex m;

    {
        //lock for other threads
        boost::mutex::scoped_lock lock(m);

        //handle if not yet handled
        if (!handled)
        {
            handled = true;

            auto msg = critical_error_msg_;

            //callback to be executed on main thread
            auto cb = [ msg ] ()
            {
                //log exception
                MessageHandler::logMessageFancy(logerr, msg);

                //show UI
                MessageHandler::showAbortMessage(msg);

                //then abort
                std::abort();
            };

            //queue exception handling to main thread's event loop
            QMetaObject::invokeMethod(qApp, cb, Qt::AutoConnection);
        }
    }

    // main thread should process events to be able to handle exception
    if (qApp->thread() == QThread::currentThread())
        QApplication::processEvents();

    // put to sleep until abort()
    std::promise<void>().get_future().wait(); 
}

}
