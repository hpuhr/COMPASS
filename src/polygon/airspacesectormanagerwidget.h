/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * AirspaceSectorManagerWidget.h
 *
 *  Created on: Nov 25, 2013
 *      Author: sk
 */

#ifndef AIRSPACESECTORMANAGERWIDGET_H_
#define AIRSPACESECTORMANAGERWIDGET_H_

#include <QWidget>
#include <QComboBox>
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>

#include "AirspaceSectorManager.h"

class AirspaceSectorTreeWidget;
class AirspaceSectorWidget;

class SectorSelectComboBox : public QComboBox
{
    Q_OBJECT
public:
    /// @brief Constructor.
    SectorSelectComboBox (QWidget *parent = 0)
    : QComboBox (parent)
    {
        loadSectors ();
    }

    /// @brief Destructor.
    virtual ~SectorSelectComboBox () { }

    void loadSectors ()
    {
        const std::map <std::string, AirspaceSector*> &sectors = AirspaceSectorManager::getInstance().getSectors ();
        std::map <std::string, AirspaceSector*>::const_iterator it;

        for (it = sectors.begin(); it != sectors.end(); it++)
        {
            addItem(it->first.c_str());
        }
    }

    /// @brief Returns the currently selected data source
    std::string getSectorName ()
    {
        return currentText().toStdString();
    }

};

//class ACGXMLImportDialog : public QDialog
//{
//    Q_OBJECT
//public:
//    ACGXMLImportDialog(QWidget* parent=NULL) : QDialog( parent ), file_label_(0), sector_select_(0), ok_button_(0), cancel_button_(0)
//    {
//        createGUIElements ();
//    }
//    /// @brief Destructor
//    virtual ~ACGXMLImportDialog() {}

//    std::string getFilename ()
//    {
//        assert (file_label_);
//        return file_label_->text().toStdString();
//    }

//    std::string getSectorName ()
//    {
//        assert (sector_select_);
//        return sector_select_->getSectorName();
//    }

//protected slots:
//    void selectFile ()
//    {
//        QString filename = QFileDialog::getOpenFileName (this, tr("Please select a XML File"), tr("/home/sk"),tr("XML FIles (*.xml)"));
//        file_label_->setText(filename);
//    }

//    /// @brief Accepts the changes made in the dialog
//    void okSlot()
//    {
//        accept();
//    }
//    /// @brief Cancels the changes made in the dialog
//    void cancelSlot()
//    {
//        reject();
//    }


//protected:
//    QLabel *file_label_;

//    SectorSelectComboBox *sector_select_;

//    QPushButton *ok_button_;
//    QPushButton *cancel_button_;

//    /// @brief Inits the dialog
//    void createGUIElements ()
//    {
//        setWindowTitle("Sector Import from ACG XML");

//        QFont font_bold;
//        font_bold.setBold(true);

//        QFont font_big;
//        font_big.setPointSize(18);

//        QGridLayout *layout = new QGridLayout ();

//        QLabel *file = new QLabel ("Select a filename");
//        file->setFont(font_bold);
//        layout->addWidget(file, 0, 0);

//        file_label_ = new QLabel ();
//        layout->addWidget(file, 0, 1);

//        QPushButton *select = new QPushButton ("Select");
//        layout->addWidget(select, 0, 2);
//        connect( select, SIGNAL(pressed()), this, SLOT(selectFile()) );

//        QLabel *sector = new QLabel ("Select a Base Sector");
//        file->setFont(font_bold);
//        layout->addWidget(sector, 1, 0);

//        sector_select_ = new SectorSelectComboBox ();
//        layout->addWidget(sector_select_, 1, 2);

//        cancel_button_ = new QPushButton ("Cancel");
//        connect(cancel_button_, SIGNAL(pressed()), this, SLOT(cancelSlot()));
//        layout->addWidget(cancel_button_, 3, 0);


//        ok_button_ = new QPushButton ("OK");
//        connect(ok_button_, SIGNAL(pressed()), this, SLOT(okSlot()));
//        layout->addWidget(ok_button_, 3, 2);

//        setLayout (layout);
//    }
//};

class AirspaceSectorManagerWidget : public QWidget
{
    Q_OBJECT
public slots:
    void addNewSector ();
    void addSectorsByShapeFile ();
    //void addSectorsByACGXMLFile ();

public:
    AirspaceSectorManagerWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~AirspaceSectorManagerWidget();

protected:
    void createElements ();

    AirspaceSectorTreeWidget *yggdrasil_;
    AirspaceSectorWidget *sector_widget_;
};

#endif /* AIRSPACESECTORMANAGERWIDGET_H_ */
