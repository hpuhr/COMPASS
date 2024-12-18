#include "autoresumedialog.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <cassert>

AutoResumeDialog::AutoResumeDialog(unsigned int resume_time_s, QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{
    setWindowTitle("Resume to Live:Running");
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    //setModal(true);
    setWindowModality(Qt::ApplicationModal);

    time_remaining_ = resume_time_s;

    QVBoxLayout* main_layout = new QVBoxLayout();

    QLabel* text = new QLabel("Please confirm to stay in Live:Paused mode or\n"
                              " select Resume to switch back to Live:Running.");
    main_layout->addWidget(text);

    QHBoxLayout* button_layout = new QHBoxLayout();

    QPushButton* stay_button = new QPushButton("Confirm");
    connect(stay_button, &QPushButton::clicked, this, &AutoResumeDialog::stayClickedSlot);
    button_layout->addWidget(stay_button);

    button_layout->addStretch();

    resume_button_ = new QPushButton("Resume ("+QString::number(time_remaining_)+"s)");
    connect(resume_button_, &QPushButton::clicked, this, &AutoResumeDialog::resumeClickedSlot);
    button_layout->addWidget(resume_button_);

    main_layout->addLayout(button_layout);

    setLayout(main_layout);

    connect(&timer_, &QTimer::timeout, this, &AutoResumeDialog::timerSlot);
    timer_.start(1000);
}

AutoResumeDialog::~AutoResumeDialog()
{
}

void AutoResumeDialog::stayClickedSlot()
{
    emit stayPausedSignal();
}

void AutoResumeDialog::timerSlot()
{
    assert (resume_button_);

    time_remaining_ -= 1;

    if (time_remaining_ <= 0)
        emit resumeSignal();
    else
    {
        resume_button_->setText("Resume ("+QString::number(time_remaining_)+"s)");
        timer_.start(1000);
    }
}

void AutoResumeDialog::resumeClickedSlot()
{
    emit resumeSignal();
}
