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

class QSignalSpy;

namespace rtcommand
{

struct RTCommand;

/**
 * Obtains data structures and calls for the command runner 
 * needed to reside in the main thread.
 */
class RTCommandRunnerStash : public QObject
{
    Q_OBJECT
public:
    RTCommandRunnerStash();
    virtual ~RTCommandRunnerStash();

    bool spySignalReceived() const;

public slots:
    bool spyForSignal(const QString& obj_name, const QString& signal_name);
    void removeSpy();
    bool executeCommand(const RTCommand* command) const;
    
private:
    std::unique_ptr<QSignalSpy> spy_;
};

} // namespace rtcommand
