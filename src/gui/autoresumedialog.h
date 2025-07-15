#pragma once

#include <QDialog>
#include <QTimer>

class AutoResumeDialog : public QDialog
{
    Q_OBJECT

signals:
    void resumeSignal();
    void stayPausedSignal();

public slots:
    void stayClickedSlot();
    void resumeClickedSlot();

    void timerSlot();

public:
    AutoResumeDialog(unsigned int resume_time_s, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~AutoResumeDialog();

protected:
    QTimer timer_;

    QPushButton* resume_button_ {nullptr};

    int time_remaining_ {0};
};

