#ifndef EVALUATIONDATASOURCEWIDGET_H
#define EVALUATIONDATASOURCEWIDGET_H

#include <QFrame>

class ActiveDataSource;

class QCheckBox;
class QGridLayout;

class EvaluationDataSourceWidget : public QFrame
{
    Q_OBJECT

protected slots:
    void dboNameChangedSlot();
    /// @brief Updates the sensor active checkboxes
    void toggleDataSourceSlot();

public:
    EvaluationDataSourceWidget(const std::string& title, std::string& dbo_name,
                               std::map<int, ActiveDataSource>& data_sources,
                               QWidget* parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags());

    virtual ~EvaluationDataSourceWidget();

protected:
    std::string title_;
    std::string& dbo_name_;

    /// Container with checkboxes for all sensors (sensor number -> checkbox)
    std::map<int, ActiveDataSource>& data_sources_;

    std::map<int, QCheckBox*> data_sources_checkboxes_;

    void updateCheckboxesChecked();
    void updateCheckboxesDisabled();
};

#endif // EVALUATIONDATASOURCEWIDGET_H
