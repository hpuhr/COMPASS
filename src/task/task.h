#ifndef TASK_H
#define TASK_H

#include <QObject>

class QWidget;

class Task : public QObject
{
public:
    Task(const std::string& name, bool can_run)
        : name_(name), can_run_(can_run)
    {

    }
    virtual ~Task() {}

    std::string name() const
    {
        return name_;
    }

    virtual QWidget* widget ()=0;

protected:
    std::string name_;

    bool can_run_ {false};
};

#endif // TASK_H

