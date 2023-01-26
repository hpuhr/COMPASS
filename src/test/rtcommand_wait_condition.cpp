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

#include "rtcommand_wait_condition.h"
#include "rtcommand.h"

#include "ui_test_find.h"

#include <QSignalSpy>
#include <QElapsedTimer>
#include <QMainWindow>

namespace rtcommand
{

/*****************************************************************************
 * WaitConditionSignal
 *****************************************************************************/

/**
 */
WaitConditionSignal::WaitConditionSignal(const QString& obj_name,
                                         const QString& signal,
                                         int timeout_ms,
                                         QObject* parent)
:   WaitCondition(timeout_ms)
{
    spy_.reset(createSpy(obj_name, signal, parent));
}

/**
 */
WaitConditionSignal::~WaitConditionSignal() = default;

/**
 */
QSignalSpy* WaitConditionSignal::createSpy(const QString& obj_name,
                                           const QString& signal,
                                           QObject* parent)
{
    if (!parent)
    {
        auto main_window = mainWindow();
        if (!main_window)
            return nullptr;

        parent = main_window;
    }
    
    auto obj = ui_test::findObject(parent, obj_name);
    if (obj.first != ui_test::FindObjectErrCode::NoError)
        return nullptr;
    
    auto spy = new QSignalSpy(obj.second, ("2" + signal.toStdString()).c_str());
    if (!spy->isValid())
    {
        delete spy;
        return nullptr;
    }

    return spy;
}

/**
 */
bool WaitConditionSignal::valid() const
{
    return (spy_ != nullptr);
}

/**
 */
bool WaitConditionSignal::expired() const
{
    return (!spy_ || spy_->count() > 0);
}

/**
 */
bool WaitConditionSignal::wait() const
{
    if (expired())
        return true;
    
    return spy_->wait(timeout_ms_);
}

/*****************************************************************************
 * WaitConditionDelay
 *****************************************************************************/

/**
 */
WaitConditionDelay::WaitConditionDelay(int msecs)
:   msecs_(msecs)
{
    if (valid())
    {
        timer_.reset(new QElapsedTimer);
        timer_->start();
    }
}

/**
 */
WaitConditionDelay::~WaitConditionDelay() = default;

/**
 */
bool WaitConditionDelay::valid() const
{
    return (msecs_ >= 0);
}

/**
 */
bool WaitConditionDelay::expired() const
{
    return (!timer_ || timer_->hasExpired(msecs_));
}

/**
 */
bool WaitConditionDelay::wait() const
{
    if (expired())
        return true;

    const int t_remain = std::max(0, (int)(msecs_ - timer_->elapsed()));
    QThread::msleep(t_remain);

    return true;
}

/**
 * Wait for the given condition to expire.
 */
bool waitForCondition(const WaitCondition& condition)
{
    if (!condition.valid())
        return false;

    return condition.wait();
}

/**
 * Wait for the given condition to expire.
 */
bool waitForCondition(const std::function<bool()>& condition, int timeout_ms)
{
    if (!condition)
        return false;

    QElapsedTimer t;
    t.start();

    while (!condition() && (timeout_ms < 0 || t.elapsed() < timeout_ms))
        QThread::msleep(10);

    return true;
}


} // namespace rtcommand
