#ifndef EVALUATIONDATASOURCEWIDGET_H
#define EVALUATIONDATASOURCEWIDGET_H

#include <QFrame>

class ActiveDataSource;
class DBObjectComboBox;

class QCheckBox;
class QGridLayout;

class EvaluationDataSourceWidget : public QFrame
{
    Q_OBJECT

signals:
    void dboNameChangedSignal(const std::string& dbo_name);

protected slots:
    void dboNameChangedSlot();
    /// @brief Updates the sensor active checkboxes
    void toggleDataSourceSlot();


public:
    EvaluationDataSourceWidget(const std::string& title, const std::string& dbo_name,
                               std::map<int, ActiveDataSource>& data_sources,
                               QWidget* parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags());

    virtual ~EvaluationDataSourceWidget();

protected:
    std::string title_;
    std::string dbo_name_;

    /// Container with checkboxes for all sensors (sensor number -> checkbox)
    std::map<int, ActiveDataSource>& data_sources_;

    DBObjectComboBox* dbo_combo_ {nullptr};

    QGridLayout* data_source_layout_ {nullptr};
    std::map<int, QCheckBox*> data_sources_checkboxes_;

    void updateDataSources();
    void updateCheckboxesChecked();
    void updateCheckboxesDisabled();
};

#endif // EVALUATIONDATASOURCEWIDGET_H
