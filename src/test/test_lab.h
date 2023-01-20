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

#include <functional>

#include <QString>

class QMenu;

/**
*/
class TestLab
{
public:
    typedef std::function<bool()> TestFunc;

    TestLab(const QString& lab_name);
    virtual ~TestLab() = default;

protected:
    virtual void addTestsToMenu_impl(QMenu* menu) = 0;

    static void addTest(QMenu* menu, const QString& name, const TestFunc& func);

private:
    friend class TestLabCollection;

    void addTestsToMenu(QMenu* menu);

    QString lab_name_;
};

/**
*/
class TestLabCollection
{
public:
    TestLabCollection() = default;
    virtual ~TestLabCollection() = default;

    void appendTestLabs(QMenu* menu);
};
