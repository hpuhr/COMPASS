
#pragma once

#include <QString>
#include <QIcon>
#include <QCursor>
#include <QKeySequence>

class QWidget;
class QShortcut;

/**
 * Class for registering and switching view 'tools'.
 * A tool consists of various attributes e.g. an id, an icon, a keyboard shortcut, a mouse cursor etc.
 * A default tool can be set, which is used when the currend tool is ended via endCurrentTool().
 */
class ViewToolSwitcher : public QObject
{
    Q_OBJECT
public:
    struct Tool
    {
        int          id = -1;
        QString      name;
        QKeySequence key_combination;
        QIcon        icon;
        QCursor      cursor;
        QShortcut*   shortcut = nullptr;
    };

    ViewToolSwitcher() = default;
    virtual ~ViewToolSwitcher() = default;

    void setDataWidget(QWidget* data_widget);
    void setDefaultTool(int default_id, bool send_update = true);

    void addTool(int id, 
                 const QString& name, 
                 const QKeySequence& key_combination = QKeySequence(), 
                 const QIcon& icon = QIcon(), 
                 const QCursor& cursor = QCursor());
    
    void setCurrentTool(int id, bool force_update = false);
    void endCurrentTool();

    int currentTool() const;
    QCursor currentCursor() const;
    QString currentName() const;
    const Tool* getTool(int id) const;
    bool hasTool(int id) const; 

    void update();

signals:
    void toolChanged(int id, const QCursor& cursor);

private:
    std::map<int, Tool> tools_;

    int current_tool_ = -1;
    int default_tool_ = -1;

    QWidget* data_widget_ = nullptr;
};
