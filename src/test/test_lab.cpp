
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
