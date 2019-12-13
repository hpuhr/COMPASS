#ifndef TASK_H
#define TASK_H

#include <QObject>

class TaskManager;
class QWidget;

class Task : public QObject
{
    Q_OBJECT

signals:
    void statusChangedSignal (std::string task_name); // emitted when settings where changes which influence the prerequisites
    void doneSignal (std::string task_name); // emitted when task is done

public:
    Task(const std::string& name, const std::string& gui_name, bool gui_only, bool expert_only,
         TaskManager& task_manager)
        : name_(name), gui_name_(gui_name), gui_only_(gui_only), expert_only_(expert_only), task_manager_(task_manager)
    {

    }
    virtual ~Task() {}

    std::string name() const
    {
        return name_;
    }

    virtual QWidget* widget ()=0;

    bool guiOnly() const
    {
        return gui_only_;
    }

    std::string guiName() const
    {
        return gui_name_;
    }

    virtual bool checkPrerequisites ()=0; // returns true can be shown, false if not yet
    virtual bool canRun() { return !gui_only_; } // returns true if can be run, to be overriden
    virtual bool isRecommended ()=0; // returns true if it is recommended to run this task
    virtual bool isRequired ()=0; // returns true if it is required to run this task

    bool expertOnly() const
    {
        return expert_only_;
    }


    bool done() const
    {
        return done_;
    }

    virtual void run () { assert(!gui_only_); } // to be overriden by tasks that can run TODO

protected:
    std::string name_;
    std::string gui_name_;
    bool gui_only_ {false};
    bool expert_only_ {false};
    bool done_ {false};

    TaskManager& task_manager_;
};

#endif // TASK_H

