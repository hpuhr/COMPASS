#pragma once

#include <QWidget>

class FFTManager;
class FFTsConfigurationDialog;
class DSTypeSelectionComboBox;

class QLabel;
class QLineEdit;
class QPushButton;
class QGridLayout;

class FFTEditWidget : public QWidget
{
    Q_OBJECT

public slots:
    //void nameEditedSlot(const QString& value);

    void modeSAddressEditedSlot(const QString& value_str);
    void modeAEditedSlot(const QString& value_str);
    void modeCEditedSlot(const QString& value_str);

    void latitudeEditedSlot(const QString& value_str);
    void longitudeEditedSlot(const QString& value_str);
    void altitudeEditedSlot(const QString& value_str);

    void deleteSlot();

public:
    FFTEditWidget(FFTManager& ds_man, FFTsConfigurationDialog& dialog);

    void showFFT(const std::string& name);
    void clear();

    void updateContent();

protected:
    FFTManager& ds_man_;
    FFTsConfigurationDialog& dialog_;

    bool has_current_fft_ {false};
    std::string current_name_;
    bool current_fft_in_db_ {false};

    QWidget* main_widget_{nullptr};

    QLineEdit* name_edit_{nullptr};

    // secondary attributes
    QLineEdit* mode_s_address_edit_{nullptr};
    QLineEdit* mode_3a_edit_{nullptr};
    QLineEdit* mode_c_edit_{nullptr};

    // position
    QLineEdit* latitude_edit_{nullptr};
    QLineEdit* longitude_edit_{nullptr};
    QLineEdit* altitude_edit_{nullptr};

    QPushButton* delete_button_{nullptr};
};

