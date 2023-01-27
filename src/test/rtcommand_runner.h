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

#include <memory>

#include <QObject>
#include <QThread>

#include "util/tbbhack.h"

class QSignalSpy;

namespace rtcommand
{

struct RTCommand;
class RTCommandChain;
class RTCommandRunnerStash;

/**
 * Class for executing runtime commands in a separate thread.
 */
class RTCommandRunner : public QThread
{
    Q_OBJECT
public:
    RTCommandRunner(RTCommandRunnerStash* stash);
    virtual ~RTCommandRunner();

    void addCommand(std::unique_ptr<RTCommand>&& cmd);
    void addCommands(RTCommandChain&& cmds);

    int numCommands() const;

protected:
    virtual void run() override;

private:
    void addCommand_internal(RTCommand* cmd);

    bool initWaitCondition(RTCommand* cmd);
    bool execWaitCondition(RTCommand* cmd);
    bool cleanupWaitCondition(RTCommand* cmd);
    bool executeCommand(RTCommand* cmd);

    void logMsg(const std::string& msg, RTCommand* cmd = nullptr);

    RTCommandRunnerStash* stash_;

    tbb::concurrent_queue<std::shared_ptr<RTCommand>> commands_;
    tbb::concurrent_queue<std::shared_ptr<RTCommand>> processed_commands_;
};

} // namespace rtcommand
