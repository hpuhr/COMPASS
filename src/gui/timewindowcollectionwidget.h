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

public:
    explicit TimeWindowCollectionWidget(Utils::TimeWindowCollection& collection, QWidget* parent = nullptr);

private slots:
    void addTimeWindow();
    void editTimeWindow(QListWidgetItem* item);

private:
    void refreshList();
    QString timeWindowToString(const Utils::TimeWindow& tw) const;

    Utils::TimeWindowCollection& collection_;
    QListWidget* list_widget_;
    QPushButton* add_button_;
};
