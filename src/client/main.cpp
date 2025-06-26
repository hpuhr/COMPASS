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

#include <iostream>

#include "client.h"
#include "compass.h"
#include "taskmanager.h"

#include <QThread>
#include <QFontDatabase>

using namespace std;

int main(int argc, char** argv)
{
    try
    {
        // Enable Qt high-DPI scaling
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

        Client client(argc, argv);

        if (client.quitRequested())
            return 0;

        // make system your application font (applies to all widgets)
        if (COMPASS::instance().isAppImage())
        {
            QFont system_font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);

            system_font.setPointSizeF(system_font.pointSizeF() * COMPASS::instance().appFontScale());
            client.setFont(system_font);
        }

        if (!client.run())
            return -1;

        return client.exec();
    }
    catch (std::exception& ex)
    {
        cerr << "main: caught exception '" << ex.what() << "'" << endl;

        return -1;
    }
    catch (...)
    {
        cerr << "main: caught exception" << endl;

        return -1;
    }
}
