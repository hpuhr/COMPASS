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

#include "test_lab.h"
#include "logger.h"

#include "test_lab_phil.h"

#include <QMenu>

/**
*/
TestLab::TestLab(const QString& lab_name)
:   lab_name_(lab_name)
{
}

/**
*/
void TestLab::addTestsToMenu(QMenu* menu)
{
    auto submenu = menu->addMenu(lab_name_);
    addTestsToMenu_impl(submenu);
}

/**
*/
void TestLab::addTest(QMenu* menu, const QString& name, const TestFunc& func)
{
    QAction* action = menu->addAction(name);

    auto cb = [ = ] ()
    {
        bool ok = func();
        loginf << "Test '" << name.toStdString() << "' " << (ok ? "[succeeded]" : "[failed]"); 
    };

    QObject::connect(action, &QAction::triggered, cb);
}

/**
*/
void TestLabCollection::appendTestLabs(QMenu* menu)
{
    auto test_lab_menu = menu->addMenu("Test Labs");

    //append test labs here
    TestLabPhil().addTestsToMenu(test_lab_menu);
}
