#pragma once

#include "util/timewindow.h"

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

namespace Utils {
class TimeWindowCollection;
}

class TimeWindowCollectionWidget : public QWidget
{
    Q_OBJECT

private slots:
    void addTimeWindow();
    void editTimeWindow(QListWidgetItem* item);

public:
    explicit TimeWindowCollectionWidget(Utils::TimeWindowCollection& collection, QWidget* parent = nullptr);

    void refreshList();

    bool somethingChangedFlag() const;

private:
    QString timeWindowToString(const Utils::TimeWindow& tw) const;

    Utils::TimeWindowCollection& collection_;
    QListWidget* list_widget_;
    QPushButton* add_button_;

    bool something_changed_flag_ {false};
};
