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

#include "ui_test_event_injections.h"
#include "ui_test_find.h"
#include "ui_test_common.h"

#include "logger.h"
#include "compass.h"

#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QObject>
#include <QMenu>
#include <QMenuBar>
#include <QComboBox>
#include <QTabWidget>
#include <QToolBar>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QCheckBox>
#include <QAbstractButton>
#include <QScrollBar>
#include <QTest>
#include <QTimer>
#include <QDialog>

namespace ui_test
{

/**
 * Injects key clicks into a widget.
*/
bool injectKeyEvent(QWidget* root,
                    const QString& obj_name,
                    const Qt::Key& key, 
                    int delay)
{
    auto w = findObjectAs<QWidget>(root, obj_name);
    if (w.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectKeyEvent", obj_name, w.first);
        return false;
    }

    loginf << "Injecting key " << QKeySequence(key).toString(QKeySequence::NativeText).toStdString();

    QTest::keyClick(w.second, key, Qt::NoModifier, delay);

    return true;
}

/**
 * Injects key clicks into a widget.
*/
bool injectKeysEvent(QWidget* root,
                     const QString& obj_name,
                     const QString& keys, 
                     PageBreakMode pb_mode,
                     int delay)
{
    auto w = findObjectAs<QWidget>(root, obj_name);
    if (w.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectKeysEvent", obj_name, w.first);
        return false;
    }

    //nothing to do?
    if (keys.isEmpty())
        return true;

    //split text at page breaks
    //@TODO: Add Qt::KeepEmptyParts in future Qt version
    QStringList lines = keys.split(QRegExp("\n|\r\n|\r"));

    const int nl = lines.count();

    auto logLine = [ & ] (const QString& line) { loginf << "injectKeysEvent: Injecting key sequence " << line.toStdString(); };

    if (pb_mode == PageBreakMode::Forbidden)
    {
        if (nl != 1 || lines[ 0 ].isEmpty())
        {
            loginf << "injectKeysEvent: Page break found but forbidden";
            return false;
        }
        //inject single line
        logLine(lines[ 0 ]);
        QTest::keyClicks(w.second, lines[ 0 ], Qt::NoModifier, delay);
    }
    else if (pb_mode == PageBreakMode::Remove)
    {
        //inject all non-empty lineS
        for (int i = 0; i < nl; ++i)
        {
            if (!lines[ i ].isEmpty())
            {
                logLine(lines[ i ]);
                QTest::keyClicks(w.second, lines[ i ], Qt::NoModifier, delay);
            }  
        }      
    }
    else if (pb_mode == PageBreakMode::SplitText)
    {
        for (int i = 0; i < nl; ++i)
        {
            //inject non-empty text lines
            if (!lines[ i ].isEmpty())
            {
                logLine(lines[ i ]);
                QTest::keyClicks(w.second, lines[ i ], Qt::NoModifier, delay);
            }
            //inject newlines as enter keys
            if (i < nl - 1)
            {
                loginf << "injectKeysEvent: Injecting page break";
                QTest::keyClick(w.second, Qt::Key_Enter, Qt::NoModifier, delay);
            }    
        }
    }

    return true;
}

/**
 * Injects (modified) key command(s) into windows or widgets.
 * @TODO: In future Qt versions base this function on QTest::keySequence() and QKeySequence.
 */
bool injectKeyCmdEvent(QWidget* root,
                       const QString& obj_name,
                       const Qt::Key& key,
                       Qt::KeyboardModifiers modifier,
                       int delay)
{
    auto obj = findObject(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectKeyCmdEvent", obj_name, obj.first);
        return false;
    }
    if (!obj.second->isWidgetType() && !obj.second->isWindowType())
    {
        logObjectError("injectKeyCmdEvent", obj_name, FindObjectErrCode::WrongType);
        return false;
    }
    
    auto injectionMsg = [ & ] (const std::string& obj_type) 
    {
        loginf << "Injecting command";
    };
 
    if (obj.second->isWidgetType())
    {
        injectionMsg("widget");
        QTest::keyClick(dynamic_cast<QWidget*>(obj.second), key, modifier, delay);
    }
    else //window
    {
        injectionMsg("window");
        QTest::keyClick(dynamic_cast<QWindow*>(obj.second), key, modifier, delay);
    }

    return true;
}

/**
 * Injects mouse clicks into widgets or windows.
 */
bool injectClickEvent(QWidget* root,
                      const QString& obj_name, 
                      int x, 
                      int y, 
                      Qt::MouseButton button, 
                      int delay)
{
    auto obj = findObject(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectClickEvent", obj_name, obj.first);
        return false;
    }

    if (!obj.second->isWidgetType() && !obj.second->isWindowType())
    {
        logObjectError("injectClickEvent", obj_name, FindObjectErrCode::WrongType);
        return false;
    }

    //by default the click will by applied to the widget/window center...
    QPoint pos;

    //...but we can provide values
    if (x >= 0 && y >= 0)
    {
        pos = QPoint(x, y);
    }

    auto injectionMsg = [ & ] (const std::string& obj_type) 
    {
        loginf << "Injecting mouse button '" << (int)button << "' click at (" << std::to_string(x) << "," << std::to_string(y) << ")";
    };
 
    if (obj.second->isWidgetType())
    {
        injectionMsg("widget");
        QTest::mouseClick(dynamic_cast<QWidget*>(obj.second), button, 0, pos, delay);
    }
    else //window
    {
        injectionMsg("window");
        QTest::mouseClick(dynamic_cast<QWindow*>(obj.second), button, 0, pos, delay);
    }

    return true;
}

/**
 * 
 */
bool injectPostModalEvent(const EventFunc& modal_trigger_cb,
                          const EventFunc& post_modal_cb)
{
    bool post_call_ok = false;

    //we queue in what should happen after the modal object is shown as an event beforehand via single shot timer
    auto cb = [ & ] ()
    {
        post_call_ok = post_modal_cb();
    };
    QTimer::singleShot(0, cb);

    bool trigger_ok = modal_trigger_cb();

    //to be on the safe side we process events here, so our post callback is processed no matter what
    QApplication::processEvents();

    //check if ok
    if (!trigger_ok || !post_call_ok)
        return false;
    
    return true;
}

namespace
{
    /**
     * Traverses the given menu via the given menu path (= a list of menu entry labels)
     * by injecting click events onto the menu and its submenus/actions.
     */
    bool traverseMenu(QWidget* menu_widget, const QStringList& menu_path, int delay)
    {
        //finds the action with the given text in the given widget (widget functionality used by qmenu and qmenubar)
        auto findAction = [ = ] (QWidget* w, const QString& txt, bool is_menu)
        {
            for (auto a : w->actions())
            {
                if (!a || a->isSeparator() || (is_menu && !a->menu()))
                    continue;

                QString txt_cur = is_menu ? normalizedMenuName(a->text()) : 
                                            normalizedActionName(a->text());
                if (txt_cur == txt)
                    return a;
            }
            return (QAction*)nullptr;
        };

        //get the geometry of the given action in the given menu widget
        auto actionGeometry = [ = ] (QWidget* w, QAction* action)
        {
            QMenu*    menu     = dynamic_cast<QMenu*>(w);
            QMenuBar* menu_bar = dynamic_cast<QMenuBar*>(w);

            //the menu widget could be either a QMenu or a QMenuBar
            if (menu)
                return menu->actionGeometry(action);
            if (menu_bar)
                return menu_bar->actionGeometry(action);

            return QRect();
        };

        size_t np = menu_path.size();

        QWidget* current_menu   = menu_widget;  //we start with the main menu
        QAction* current_action = nullptr;

        //iterate over path strings
        for (size_t i = 0; i < np; ++i)
        {
            QString p = menu_path[ i ].trimmed();

            //all path items should be menus except the last one
            bool is_menu = (i < np - 1);

            //find action in current menu for given item text
            current_action = findAction(current_menu, p, is_menu);
            if (!current_action)
            {
                loginf << "traverseMenu: Item text '" << p.toStdString() << "' not found";
                return false;
            }

            //get geometry of action in menu
            QRect r = actionGeometry(current_menu, current_action);

            //inject click at action
            if (!injectClickEvent(current_menu, "", r.x() + r.width() / 2, r.y() + r.height() / 2, Qt::LeftButton, delay))
                return false;

            //set current menu to the menu associated with the current action
            if (is_menu)
                current_menu = current_action->menu();
        }

        loginf << "traverseMenu: Final action is '" << current_action->text().toStdString() << "'";

        return true;
    }

    /**
     * Traverses the given menu by injecting key events, in search for the given string.
     */
    QAction* traverseMenuFor(QWidget* menu, const QString& text, bool is_menu, int delay)
    {
        auto activeAction = [ = ] (QWidget* w)
        {
            QMenu*    menu     = dynamic_cast<QMenu*>(w);
            QMenuBar* menu_bar = dynamic_cast<QMenuBar*>(w);

            //the menu widget could be either a QMenu or a QMenuBar
            if (menu)
                return menu->activeAction();
            if (menu_bar)
                return menu_bar->activeAction();

            return (QAction*)nullptr;
        };

        auto checkActiveAction = [ & ] ()
        {
            //the active action is the currently highlighted action
            auto active = activeAction(menu);
            if (!active || (is_menu && !active->menu()))
                return false;

            QString txt_cur = is_menu ? normalizedMenuName(active->text()) : 
                                        normalizedActionName(active->text());
            return (txt_cur == text);
        };

        const int n = menu->actions().count();

        //injects the given key and stops if the active action is the sought one, or if n injections are reached
        auto runFinder = [ & ] (Qt::Key key)
        {
            int cnt = 0;
            while (!checkActiveAction() && cnt++ <= n)
                QTest::keyEvent(QTest::KeyAction::Click, menu, key, Qt::NoModifier, delay);

            return true;
        };

        //run up and down the menu in search for the right action
        if (!runFinder(Qt::Key_Down) || !runFinder(Qt::Key_Up))
            return nullptr;

        //is the active action the sought one?
        if (!checkActiveAction())
            return nullptr;

        return activeAction(menu);
    }

    /**
     * Traverses the given menu via the given menu path (= a list of menu entry labels)
     * by injecting key events onto the menu and its submenus.
     */
    bool traverseMenuKeys(QWidget* menu_widget, const QStringList& menu_path, int delay, bool close)
    {
        size_t np = menu_path.size();

        QWidget* current_menu   = menu_widget;  //we start with the main menu
        QAction* current_action = nullptr;

        //iterate over path strings
        for (size_t i = 0; i < np; ++i)
        {
            QString p = menu_path[ i ].trimmed();

            //all path items should be menus except the last one
            bool is_menu = (i < np - 1);

            //find action in current menu for given item text
            current_action = traverseMenuFor(current_menu, p, is_menu, delay);
            if (!current_action)
            {
                loginf << "traverseMenuKeys: Item text '" << p.toStdString() << "' not found";
                return false;
            }

            //set current menu to the menu associated with the current action
            if (is_menu)
            {
                //move right to step into the new menu
                if (!injectKeyEvent(current_menu, "", Qt::Key_Right, delay))
                    return false;

                current_menu = current_action->menu();
            }
        }

        loginf << "traverseMenu: Final action is '" << current_action->text().toStdString() << "'";

        //fire the final action via enter key
        if (!injectKeyEvent(current_menu, "", Qt::Key_Return, delay))
            return false;

        return true;
    }
}

/**
 * Injects a trigger event to a subitem of the given menu bar.
 */
bool injectMenuBarEvent(QWidget* root,
                        const QString& obj_name,
                        const QStringList& path_to_action,
                        int delay)
{
    auto obj = findObjectAs<QMenuBar>(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectMenuBarEvent", obj_name, obj.first);
        return false;
    }

    //@TODO: should this return false?
    if (path_to_action.empty())
        return true;
    
    return traverseMenu(obj.second, path_to_action, delay);
}

/**
 * Injects a trigger event to a subitem of the given menu.
 */
bool injectMenuEvent(QWidget* root,
                     const QString& obj_name,
                     const QStringList& path_to_action,
                     int delay)
{
    auto obj = findObjectAs<QMenu>(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectMenuEvent", obj_name, obj.first);
        return false;
    }

    //@TODO: should this return false?
    if (path_to_action.empty())
        return true;
    
    //seems that for popup menus the key based traversal works better
    return traverseMenuKeys(obj.second, path_to_action, delay, true);
}

/**
 * Injects a trigger event to a subitem of the current popup menu.
 */
bool injectPopupMenuEvent(const QStringList& path_to_action,
                          int delay)
{
    //is there an active popup?
    auto popup = QApplication::activePopupWidget();
    if (!popup)
    {
        logObjectError("injectPopupMenuEvent", "popup", FindObjectErrCode::NotFound);
        return false;
    }

    //popup should be a menu
    QMenu* menu = dynamic_cast<QMenu*>(popup);
    if (!menu)
    {
        logObjectError("injectPopupMenuEvent", "popup", FindObjectErrCode::WrongType);
        return false;
    }

    //traverse menu
    return injectMenuEvent(popup, "", path_to_action, delay);
}

/**
 * Inject a combo selection event into the given combo box object.
 * 'entry_txt' is the text of the combo box entry to be selected.
 */
bool injectComboBoxEditEvent(QWidget* root,
                             const QString& obj_name,
                             const QString& entry_txt,
                             int delay)
{
    auto obj = findObjectAs<QComboBox>(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectComboBoxEditEvent", obj_name, obj.first);
        return false;
    }

    int idx = obj.second->findText(entry_txt);
    if (idx < 0)
    {
        loginf << "injectComboBoxEditEvent: Entry '" << entry_txt.toStdString() << "' not found";
        return false;
    }

    //move either up or down using key injections
    int cur_idx = obj.second->currentIndex();
    Qt::Key key = (idx >= cur_idx ? Qt::Key_Down : Qt::Key_Up);

    int cnt = 0;
    while (obj.second->currentIndex() != idx && cnt++ < obj.second->count())
    {
        if(!injectKeyEvent(obj.second, "", key))
            return false;
    }

    return (obj.second->currentIndex() == idx);
}

/**
 * Inject a tab selection event into the given tab widget object.
 * 'tab_txt' is the text of the tab to be selected.
 */
bool injectTabSelectionEvent(QWidget* root,
                             const QString& obj_name,
                             const QString& tab_txt,
                             int delay)
{
    auto obj = findObjectAs<QTabWidget>(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectTabSelectionEvent", obj_name, obj.first);
        return false;
    }

    if (obj.second->count() < 1)
    {
        loginf << "injectTabSelectionEvent: Tab empty";
        return false;
    }

    int idx = -1;
    for (int i = 0; i < obj.second->count(); ++i)
    {
        if (obj.second->tabText(i) == tab_txt)
        {
            idx = i;
            break;
        }
    }
        
    if (idx < 0)
    {
        loginf << "injectTabSelectionEvent: Tab text '" << tab_txt.toStdString() << "' not found";
        return false;
    }

    int cur_idx = obj.second->currentIndex();

    //move either left or right using key injections
    //@TODO: handle vertical tabs
    Qt::Key key = (idx >= cur_idx ? Qt::Key_Right : Qt::Key_Left);

    int cnt = 0;
    while (obj.second->currentIndex() != idx && cnt++ <= obj.second->count() + 1)
    {
        if (!injectKeyEvent(obj.second->tabBar(), "", key, delay))
            return false;
    }
        
    return (obj.second->currentIndex() == idx);
}

/**
 * Inject a tool triggering event into the given tool bar object.
 * 'tool_name' is the display name of the tool to be triggered.
 */
bool injectToolSelectionEvent(QWidget* root,
                              const QString& obj_name,
                              const QString& tool_name,
                              int delay)
{
    auto obj = findObjectAs<QToolBar>(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectToolSelectionEvent", obj_name, obj.first);
        return false;
    }

    int      idx    = -1;
    QAction* action = nullptr;
    for (int i = 0; i < obj.second->actions().count(); ++i)
    {
        QAction* a  = obj.second->actions()[ i ];
        if (a->isSeparator())
            continue;

        const QString tn = normalizedToolName(a->text());
        if (tn == tool_name)
        {
            idx    = i;
            action = a;
            break;
        }
    }

    if (idx < 0)
    {
        loginf << "injectToolSelectionEvent: Tool name '" << tool_name.toStdString() << "' not found";
        return false;
    }

    QWidget* w = obj.second->widgetForAction(action);
    if (!w)
    {
        loginf << "injectToolSelectionEvent: Tool '" << tool_name.toStdString() << "' obtains no widget";
        return false;
    }

    if (!injectClickEvent(w, "", -1, -1, Qt::LeftButton, delay))
        return false;

    return true;
}

/**
 * Injects new text into the given edit object (clearing the old content).
 */
bool injectLineEditEvent(QWidget* root,
                         const QString& obj_name,
                         const QString& text,
                         int delay)
{
    auto obj = findObjectAs<QLineEdit>(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectLineEditEvent", obj_name, obj.first);
        return false;
    }

    //highlight text
    if (!injectKeyCmdEvent(obj.second, "", Qt::Key_A, Qt::ControlModifier, delay))
        return false;
 
    //fill with new content
    if (!injectKeysEvent(obj.second, "", text, PageBreakMode::Forbidden, delay))
        return false;

    return true;
}

/**
 * Injects new text into the given edit object (clearing the old content).
 * The given text may contain line breaks.
 */
bool injectTextEditEvent(QWidget* root,
                         const QString& obj_name,
                         const QString& text,
                         int delay)
{
    auto obj = findObjectAs<QTextEdit>(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectTextEditEvent", obj_name, obj.first);
        return false;
    }

    //highlight text
    if (!injectKeyCmdEvent(obj.second, "", Qt::Key_A, Qt::ControlModifier, delay))
        return false;

    //fill with new content
    if (!injectKeysEvent(obj.second, "", text, PageBreakMode::SplitText, delay))
        return false;

    return true;
}

/**
 * Injects an edit action into the given spin box object.
 */
bool injectSpinBoxEvent(QWidget* root,
                        const QString& obj_name,
                        int value,
                        int delay)
{
    auto obj = findObjectAs<QSpinBox>(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectSpinBoxEvent", obj_name, obj.first);
        return false;
    }

    QString txt = QString::number(value);

    //clear line edit
    if (!injectKeyCmdEvent(obj.second, "", Qt::Key_A, Qt::ControlModifier, delay))
        return false;
 
    //fill with new content
    if (!injectKeysEvent(obj.second, "", txt, PageBreakMode::Forbidden, delay))
        return false;

    return (obj.second->value() == value);
}


/**
 * Injects an edit action into the given spin box object.
 */
bool injectDoubleSpinBoxEvent(QWidget* root,
                              const QString& obj_name,
                              double value,
                              int delay)
{
    auto obj = findObjectAs<QDoubleSpinBox>(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectDoubleSpinBoxEvent", obj_name, obj.first);
        return false;
    }

    QString txt = QString::number(value, 'f', obj.second->decimals());
    double value_trunc = txt.toDouble();

    //clear line edit
    if (!injectKeyCmdEvent(obj.second, "", Qt::Key_A, Qt::ControlModifier, delay))
        return false;
 
    //fill with new content
    if (!injectKeysEvent(obj.second, "", txt, PageBreakMode::Forbidden, delay))
        return false;

    return (obj.second->value() == value_trunc);
}

namespace 
{
    bool findSliderValue(QAbstractSlider* slider, double percent, bool invert, int delay)
    {
        double t         = std::min(1.0, std::max(0.0, percent / 100.0));
        int    value     = (1.0 - t) * slider->minimum() + t * slider->maximum();

        //std::cout << "min:         " << slider->minimum() << std::endl;
        //std::cout << "max:         " << slider->maximum() << std::endl;
        //std::cout << "current:     " << slider->value() << std::endl;
        //std::cout << "t:           " << t << std::endl;
        //std::cout << "value:       " << value << std::endl;

        int    single_step = slider->singleStep();
        int    page_step   = slider->pageStep();

        //std::cout << "single step: " << single_step << std::endl;
        //std::cout << "page step:   " << page_step << std::endl;

        auto computeMaxSteps = [ = ] (int v, int vtarget, int step)
        {
            return std::abs(v - vtarget) / step + 2;
        };

        //first inject page steps until we are below page step accuracy
        {
            const int max_steps_page = computeMaxSteps(value, slider->value(), page_step);

            //std::cout << "max page steps: " << max_steps_page << std::endl;

            Qt::Key key_plus  = !invert ? Qt::Key_PageUp   : Qt::Key_PageDown;
            Qt::Key key_minus = !invert ? Qt::Key_PageDown : Qt::Key_PageUp;

            int cnt = 0;
            while (std::abs(value - slider->value()) >= page_step && cnt++ <= max_steps_page)
                if (!injectKeyEvent(slider, "", slider->value() <= value ? key_plus : key_minus, delay))
                    return false;

            //std::cout << "remaining: " << std::abs(value - slider->value()) << " after " << cnt << "/" << max_steps_page << " iteration(s)" << std::endl;
        }

        //then inject single steps until we are below single step accuracy (usually single step size = 1, resulting in the sought target value)
        {
            const int max_steps_single = computeMaxSteps(value, slider->value(), single_step);

            //std::cout << "max single steps: " << max_steps_single << std::endl;

            Qt::Key key_plus  = (!invert || COMPASS::isAppImage()) ? Qt::Key_Right : Qt::Key_Left;
            Qt::Key key_minus = (!invert || COMPASS::isAppImage()) ? Qt::Key_Left  : Qt::Key_Right;
        
            int cnt = 0;
            while (std::abs(value - slider->value()) >= single_step && cnt++ <= max_steps_single)
                if (!injectKeyEvent(slider, "", slider->value() <= value ? key_plus :  key_minus, delay))
                    return false;

            //std::cout << "remaining: " << std::abs(value - slider->value()) << " after " << cnt << "/" << max_steps_single << " iteration(s)" << std::endl;
        }

        //std::cout << "final value: " << slider->value() << ", target: " << value << ", remaining: " << std::abs(value - slider->value()) << std::endl;

        return (std::abs(value - slider->value()) < single_step);
    }
}

/**
 * Injects a slider edit action into the given slider object.
 * The given percent [0,100] will result in a discrete slider value between the slider's minimum() and maximum().
 * Note that first pageStep()'s will be injected to get a rough position, and the singleStep()'s will be used for fine tuning.
 * Thus, depending on the slider range and the page step size, a certain number of slider changes will be triggered.
 * Also note that if the single step size is bigger than 1, an exact result cannot be guaranteed.
 */
bool injectSliderEditEvent(QWidget* root,
                           const QString& obj_name,
                           double percent,
                           int delay)
{
    auto obj = findObjectAs<QAbstractSlider>(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectSliderEditEvent", obj_name, obj.first);
        return false;
    }

    return findSliderValue(obj.second, percent, false, delay);
}

/**
 * Injects a scroll edit action into the given scrollbar object.
 * The given percent [0,100] will result in a discrete slider value between the slider's minimum() and maximum().
 * Note that first pageStep()'s will be injected to get a rough position, and the singleStep()'s will be used for fine tuning.
 * Thus, depending on the slider range and the page step size, a certain number of slider changes will be triggered.
 * Also note that if the single step size is bigger than 1, an exact result cannot be guaranteed.
 */
bool injectScrollEditEvent(QWidget* root,
                           const QString& obj_name,
                           double percent,
                           int delay)
{
    auto obj = findObjectAs<QScrollBar>(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectScrollEditEvent", obj_name, obj.first);
        return false;
    }

    return findSliderValue(obj.second, percent, true, delay);
}

/**
 * Injects a mouse click into the given combo box object, in order to achieve the desired state.
 */
bool injectCheckBoxEvent(QWidget* root,
                         const QString& obj_name,
                         bool on,
                         int delay)
{
    auto obj = findObjectAs<QCheckBox>(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectCheckBoxEvent", obj_name, obj.first);
        return false;
    }

    //check state ok?
    if (obj.second->isChecked() == on)
        return true;

    //toggle check state
    if (!injectClickEvent(obj.second, "", 2, 2, Qt::LeftButton, delay))
        return false;

    return obj.second->isChecked() == on;
}

/**
 * Traverses the menu opened by a click on the given button object.
 * Expects that a click on the button will open a popup menu,
 * which is traversed along the given path.
 */
bool injectButtonMenuEvent(QWidget* root,
                           const QString& obj_name,
                           const QStringList& path_to_action,
                           int x,
                           int y,
                           int delay)
{
    auto obj = findObjectAs<QAbstractButton>(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectButtonMenuEvent", obj_name, obj.first);
        return false;
    }

    return injectPostModalEvent([ = ] () { return injectClickEvent(obj.second, "", x, y, Qt::LeftButton, delay); },
                                [ = ] () { return injectPopupMenuEvent(path_to_action, delay); });
}

/**
*/
bool injectDialogEvent(QWidget* root,
                           const QString& obj_name,
                           bool accept,
                           int delay)
{
    auto obj = findObjectAs<QDialog>(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectDialogEvent", obj_name, obj.first);
        return false;
    }

    //@TODO: with events!
    if (accept)
        obj.second->accept();
    else
        obj.second->reject();

    return true;
}

} // namespace ui_test
