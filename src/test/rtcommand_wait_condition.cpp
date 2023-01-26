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

#include "ui_test_find.h"

#include <QSignalSpy>
#include <QElapsedTimer>

namespace rtcommand
{

/**
 */
WaitConditionSignal::WaitConditionSignal(QObject* parent,
                                         const QString& obj_name, 
                                         const QString& signal,
                                         int timeout_ms)
:   WaitCondition(timeout_ms)
{
    auto obj = ui_test::findObject(parent, obj_name);
    if (obj.first == ui_test::FindObjectErrCode::NoError)
    {
        spy_.reset(new QSignalSpy(obj.second, SIGNAL(signal.toStdString().c_str())));
    }
}

/**
 */
WaitConditionSignal::~WaitConditionSignal() = default;

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

} // namespace rtcommand
