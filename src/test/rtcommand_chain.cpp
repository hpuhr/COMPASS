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

#include "rtcommand_chain.h"
#include "ui_test_cmd.h"

namespace rtcommand
{

/**
 */
RTCommandChain::RTCommandChain(RTCommandChain&& other)
{
    commands_ = std::move(other.commands_);
}

/**
 */
void RTCommandChain::addCommand(RTCommand* cmd)
{
    commands_.emplace(cmd);
}

/**
 */
void RTCommandChain::attachWaitCondition(const RTCommandWaitCondition& condition)
{
    if (commands_.empty())
        throw std::runtime_error("RTCommandChain::appendWaitCondition: no command to append to");

    auto last = commands_.back().get();
    last->condition = condition;
}

/**
 */
RTCommandChain::RTCommandPtr RTCommandChain::pop()
{
    if (commands_.empty())
        return nullptr;

    auto ptr = std::move(commands_.front());
    commands_.pop();

    return ptr;
}

/**
 */
RTCommandChain& RTCommandChain::empty()
{
    addCommand(new RTCommandEmpty);
    return *this;
}

/**
 */
RTCommandChain& RTCommandChain::waitForSignal(const QString& obj, const QString& signal, int timeout_ms)
{
    RTCommandWaitCondition wc;
    wc.type       = RTCommandWaitCondition::Type::Signal;
    wc.obj        = obj;
    wc.value      = signal;
    wc.timeout_ms = timeout_ms;

    attachWaitCondition(wc);

    return *this;
}

/**
 */
RTCommandChain& RTCommandChain::waitFor(int msec)
{
    RTCommandWaitCondition wc;
    wc.type       = RTCommandWaitCondition::Type::Delay;
    wc.timeout_ms = msec;

    attachWaitCondition(wc);

    return *this;
}

/**
 */
RTCommandChain& RTCommandChain::uiset(const QString& obj, const QString& value, int delay)
{
    auto c = new ui_test::RTCommandUISet;
    c->obj   = obj;
    c->value = value;
    c->delay = delay;

    addCommand(c);

    return *this;
}

} // namespace rtcommand
