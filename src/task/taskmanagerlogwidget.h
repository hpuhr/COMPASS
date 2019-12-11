#ifndef TASKMANAGERLOGWIDGET_H
#define TASKMANAGERLOGWIDGET_H

#include <QPlainTextEdit>

class TaskManagerLogWidget : public QPlainTextEdit
{
    Q_OBJECT

public:
    TaskManagerLogWidget(QWidget* parent=nullptr);

    void appendSuccess(const std::string& text);
    void appendInfo(const std::string& text);
    void appendWarning(const std::string& text);
    void appendError(const std::string& text);
};

#endif // TASKMANAGERLOGWIDGET_H
