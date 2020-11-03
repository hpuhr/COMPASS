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

#ifndef VIEWCONTAINERCONFIGWIDGET_H_
#define VIEWCONTAINERCONFIGWIDGET_H_

#include <QWidget>

class ViewContainer;
class View;
class QLabel;
class QVBoxLayout;

class ViewControlWidget : public QWidget
{
    Q_OBJECT
  public:
    ViewControlWidget(View* view, QWidget* parent = nullptr);
    ~ViewControlWidget();

  private slots:
    void loadingStartedSlot();
    void loadingFinishedSlot();
    void loadingTimeSlot(double s);
    void removeViewSlot();

  signals:
    void viewDeleted();

  private:
    View* view_;

    QLabel* load_;
    QString time_;
};

class ViewContainerConfigWidget : public QWidget
{
    Q_OBJECT
  public:
    ViewContainerConfigWidget(ViewContainer* view_container, QWidget* parent = nullptr);
    virtual ~ViewContainerConfigWidget();

    const QString& name() { return name_; }

    void addView(const std::string& class_name);

    //    void addTemplateView (std::string template_name);

  public slots:
    void updateSlot();
    void closeSlot();

  private:
    ViewContainer* view_container_;
    std::vector<ViewControlWidget*> view_widgets_;
    QVBoxLayout* layout_;

    QString name_;
};

#endif  // VIEWCONTAINERCONFIGWIDGET_H_
