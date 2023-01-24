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

#include "test_lab_phil.h"

#include "compass.h"
#include "mainwindow.h"

#include <iostream>

#include <QMenu>
#include <QDialog>
#include <QMainWindow>
#include <QTimer>
#include <QPainter>
#include <QCheckBox>
#include <QTime>
#include <QComboBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QToolBar>
#include <QSlider>
#include <QMenuBar>
#include <QMessageBox>
#include <QLabel>

#include "ui_test_event_injections.h"
#include "ui_test_set.h"
#include "ui_test_common.h"

/**
*/
TestLabPhil::TestLabPhil()
:   TestLab("Phil's Lab")
{
}

#define ADD_TEST(name, func) \
    addTest(menu, name, [ = ] () { return TestLabPhil::func(); });

/**
*/
void TestLabPhil::addTestsToMenu_impl(QMenu* menu)
{
    ADD_TEST("UI Injection Test", uiInjectionTest);
    ADD_TEST("Generate Object Name Test", uiObjectNameGenerationTest);
    ADD_TEST("UI Set Test", uiSetTest);
}

/**
*/
bool TestLabPhil::uiInjectionTest()
{
    QMainWindow* mw = new QMainWindow;
    mw->setObjectName("main_window");

    QWidget* main_widget = new QWidget;
    main_widget->setObjectName("main_widget");
    mw->setCentralWidget(main_widget);

    QMenuBar* menu_bar = new QMenuBar(mw);
    menu_bar->setObjectName("menu_bar");
    mw->setMenuBar(menu_bar);

    QToolBar* tool_bar = mw->addToolBar("Tool Bar");
    tool_bar->setObjectName("tool_bar");
    auto tool1 = tool_bar->addAction("Tool 1 [1]");
    auto tool2 = tool_bar->addAction("Tool 2 [2]");
    QObject::connect(tool1, &QAction::triggered, [ = ] { QMessageBox::information(mw, "Tool 1", "Tool 1"); });
    QObject::connect(tool2, &QAction::triggered, [ = ] { QMessageBox::information(mw, "Tool 2", "Tool 2"); });
    
    QMenu* menu1 = new QMenu("&Menu 1");
    menu1->setObjectName("menu1");
    menu_bar->addMenu(menu1);

    QMenu* menu2 = new QMenu("Menu 2");
    menu2->setObjectName("menu2");
    menu_bar->addMenu(menu2);

    QAction* action1 = new QAction("Action 1");
    action1->setObjectName("action1");
    menu1->addAction(action1);
    QObject::connect(action1, &QAction::triggered, [ = ] { QMessageBox::information(mw, "Action1", "Action1"); });

    QAction* action2 = new QAction("Action 2");
    action2->setObjectName("action2");
    menu1->addAction(action2);
    QObject::connect(action2, &QAction::triggered, [ = ] { QMessageBox::information(mw, "Action2", "Action2"); });

    QMenu* menu3 = new QMenu("Menu 3");
    menu3->setObjectName("menu3");
    menu1->addMenu(menu3);

    QAction* action3 = new QAction("Action 3");
    action3->setObjectName("action3");
    menu3->addAction(action3);
    QObject::connect(action3, &QAction::triggered, [ = ] { QMessageBox::information(mw, "Action3", "Action3"); });

    QAction* action4 = new QAction("Action 4");
    action4->setObjectName("action4");
    menu3->addAction(action4);
    QObject::connect(action4, &QAction::triggered, [ = ] { QMessageBox::information(mw, "Action4", "Action4"); });

    QAction* action5 = new QAction("Action 5");
    action5->setObjectName("action5");
    menu2->addAction(action5);
    QObject::connect(action5, &QAction::triggered, [ = ] { QMessageBox::information(mw, "Action5", "Action5"); });

    QAction* action6 = new QAction("Action 6");
    action6->setObjectName("action6");
    action6->setCheckable(true);
    menu2->addAction(action6);

    QHBoxLayout* layout_h = new QHBoxLayout;
    QVBoxLayout* layout_v = new QVBoxLayout;
    QVBoxLayout* layout_v2 = new QVBoxLayout;
    QFormLayout* layout_f = new QFormLayout;

    main_widget->setLayout(layout_h);
    layout_h->addLayout(layout_v);
    layout_h->addLayout(layout_v2);
    layout_v->addLayout(layout_f);

    QTabWidget* tab_widget = new QTabWidget;
    tab_widget->setObjectName("tab_widget");
    tab_widget->addTab(new QWidget, "Tab 1");
    tab_widget->addTab(new QWidget, "Tab 2");
    tab_widget->addTab(new QWidget, "Tab 3");
    layout_h->addWidget(tab_widget);

    QSpinBox* spin_box = new QSpinBox;
    spin_box->setObjectName("spin_box");
    spin_box->setMaximum(99999);
    layout_f->addWidget(spin_box);

    QDoubleSpinBox* dbl_spin_box = new QDoubleSpinBox;
    dbl_spin_box->setObjectName("dbl_spin_box");
    dbl_spin_box->setMaximum(99999999);
    layout_f->addWidget(dbl_spin_box);

    QLineEdit* line_edit = new QLineEdit;
    line_edit->setObjectName("line_edit");
    layout_f->addWidget(line_edit);

    QTextEdit* text_edit = new QTextEdit;
    text_edit->setObjectName("text_edit");
    layout_f->addWidget(text_edit);

    QComboBox* combo_box = new QComboBox;
    combo_box->setObjectName("combo_box");
    combo_box->addItem("Item 1");
    combo_box->addItem("Item 2");
    combo_box->addItem("Item 3");
    layout_f->addWidget(combo_box);

    QCheckBox* check_box = new QCheckBox;
    check_box->setObjectName("check_box");
    layout_f->addWidget(check_box);

    QSlider* slider = new QSlider;
    slider->setObjectName("slider");
    slider->setOrientation(Qt::Horizontal);
    slider->setMaximum(1000);
    layout_f->addWidget(slider);

    QPushButton* push_button = new QPushButton("Button");
    push_button->setObjectName("push_button");
    layout_f->addWidget(push_button);
    QObject::connect(push_button, &QPushButton::pressed, [ = ] { QMessageBox::information(mw, "Button", "Button"); });

    layout_v->addStretch(1);

    auto add_injection_callback = [ = ] (const std::function<void()>& cb, const QString& name)
    {
        QPushButton* inject_button = new QPushButton(name);
        layout_v2->addWidget(inject_button);
        QObject::connect(inject_button, &QPushButton::pressed, cb);
    };

#if 0
    add_injection_callback([ = ]() { ui_test::injectSpinBoxEvent(mw, "spin_box", 666); }, "SpinBoxText");
    add_injection_callback([ = ]() { ui_test::injectDoubleSpinBoxEvent(mw, "dbl_spin_box", 666.666); }, "DblSpinBoxText");
    add_injection_callback([ = ]() { ui_test::injectLineEditEvent(mw, "line_edit", "Belsnickel is here"); }, "LineEditText");
    add_injection_callback([ = ]() { ui_test::injectTextEditEvent(mw, "text_edit", "Cheer or fear\nBelsnickel is here"); }, "TextEditText");
    add_injection_callback([ = ]() { ui_test::injectComboBoxEditEvent(mw, "combo_box", "Item 2"); }, "ComboSelection");
    add_injection_callback([ = ]() { ui_test::injectClickEvent(mw, "check_box", 2, 2); }, "CheckBoxClick");
    add_injection_callback([ = ]() { ui_test::injectClickEvent(mw, "push_button"); }, "ButtonClick");
    add_injection_callback([ = ]() { ui_test::injectKeyCmdEvent(mw, "main_window", QKeySequence("Alt+M")); }, "MainMenuCommand");
    add_injection_callback([ = ]() { ui_test::injectMenuBarEvent(mw, "menu_bar", { "Menu 1", "Menu 3", "Action 4" }); }, "MainMenuAction");
    add_injection_callback([ = ]() { ui_test::injectMenuBarEvent(mw, "menu_bar", { "Menu 2", "Action 6" }); }, "MainMenuCheckableAction");
    add_injection_callback([ = ]() { ui_test::injectTabSelectionEvent(mw, "tab_widget", "Tab 2"); }, "TabSelection");
    add_injection_callback([ = ]() { ui_test::injectToolSelectionEvent(mw, "tool_bar", "Tool 2"); }, "ToolBarAction");
    add_injection_callback([ = ]() { ui_test::injectSliderEditEvent(mw, "slider", 34.246); }, "SliderEdit");
#else
    add_injection_callback([ = ]() { ui_test::setUIElement(mw, "spin_box", "666"); }, "SpinBoxText");
    add_injection_callback([ = ]() { ui_test::setUIElement(mw, "dbl_spin_box", "666.666"); }, "DblSpinBoxText");
    add_injection_callback([ = ]() { ui_test::setUIElement(mw, "line_edit", "Belsnickel is here"); }, "LineEditText");
    add_injection_callback([ = ]() { ui_test::setUIElement(mw, "text_edit", "Cheer or fear\nBelsnickel is here\n"); }, "TextEditText");
    add_injection_callback([ = ]() { ui_test::setUIElement(mw, "combo_box", "Item 2"); }, "ComboSelection");
    add_injection_callback([ = ]() { ui_test::setUIElement(mw, "check_box", "true"); }, "CheckBoxClick");
    add_injection_callback([ = ]() { ui_test::setUIElement(mw, "push_button", ""); }, "ButtonClick");
    add_injection_callback([ = ]() { ui_test::injectKeyCmdEvent(mw, "main_window", QKeySequence("Alt+M")); }, "MainMenuCommand");
    add_injection_callback([ = ]() { ui_test::setUIElement(mw, "menu_bar", "Menu 1|Menu 3|Action 4"); }, "MainMenuAction");
    add_injection_callback([ = ]() { ui_test::setUIElement(mw, "menu_bar", "Menu 2|Action 6"); }, "MainMenuCheckableAction");
    add_injection_callback([ = ]() { ui_test::setUIElement(mw, "tab_widget", "Tab 2"); }, "TabSelection");
    add_injection_callback([ = ]() { ui_test::setUIElement(mw, "tool_bar", "Tool 2"); }, "ToolBarAction");
    add_injection_callback([ = ]() { ui_test::setUIElement(mw, "slider", "34.246"); }, "SliderEdit");
#endif

    QDialog dlg;
    QHBoxLayout* d_layout = new QHBoxLayout;
    dlg.setLayout(d_layout);

    d_layout->addWidget(mw);

    dlg.resize(800, 600);
    dlg.exec();

    return true;
}

/**
*/
bool TestLabPhil::uiObjectNameGenerationTest()
{
    auto testObjNameGen = [ & ] (const QString& str)
    {
        QString res = ui_test::generateObjectName(str);
        std::cout << "Object name: '" << str.toStdString() << "' => '" << res.toStdString() << "'" << std::endl;
    } ;

    testObjNameGen("  My Func  1   ");
    testObjNameGen(" My Func 2:  ");
    testObjNameGen(" My Func 3 :  ");
    testObjNameGen("My Func 4...");
    testObjNameGen("My Func 5 - The Best...");

    return true;
}

/**
*/
bool TestLabPhil::uiSetTest()
{
    auto main_window = &COMPASS::instance().mainWindow();

    QDialog dlg;
    QVBoxLayout* layoutv = new QVBoxLayout;
    QHBoxLayout* layouth = new QHBoxLayout;
    dlg.setLayout(layoutv);

    layoutv->addLayout(layouth);

    layouth->addWidget(new QLabel("Object: "));

    QLineEdit* obj_edit = new QLineEdit;
    layouth->addWidget(obj_edit);

    layouth->addWidget(new QLabel("Value: "));

    QLineEdit* value_edit = new QLineEdit;
    layouth->addWidget(value_edit);

    QPushButton* button = new QPushButton("Set");
    layouth->addWidget(button);

    QLabel* status_label = new QLabel("");

    layoutv->addWidget(status_label);
    layoutv->addStretch();

    auto cb = [ = ] () 
    {
        QString obj   = obj_edit->text();
        QString value = value_edit->text();

        bool ok = ui_test::setUIElement(main_window, obj, value);

        status_label->setText(ok ? "Success" : "Failed");
    };

    QObject::connect(button, &QPushButton::pressed, cb);

    QString init_obj = "";
    QString init_val = "";

    obj_edit->setText(init_obj);
    value_edit->setText(init_val);

    dlg.exec();

    return true;
}
