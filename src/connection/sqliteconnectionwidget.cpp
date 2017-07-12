#include "sqliteconnectionwidget.h"
#include "sqliteconnection.h"
#include "mysqlserver.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QInputDialog>
#include <QStackedWidget>
#include <QListWidget>
#include <QFileDialog>

SQLiteConnectionWidget::SQLiteConnectionWidget(SQLiteConnection &connection, QWidget *parent)
    : QWidget(parent), connection_(connection), file_list_ (nullptr), add_button_(nullptr), delete_button_(nullptr), open_button_(nullptr)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout *layout = new QVBoxLayout ();

    QLabel *files_label = new QLabel ("File Selection");
    files_label->setFont(font_bold);
    layout->addWidget(files_label);

    file_list_ = new QListWidget ();
    file_list_->setWordWrap(true);
    file_list_->setTextElideMode (Qt::ElideNone);
    file_list_->setSelectionBehavior( QAbstractItemView::SelectItems );
    file_list_->setSelectionMode( QAbstractItemView::SingleSelection );
    layout->addWidget(file_list_);

    add_button_ = new QPushButton ("Add");
    connect (add_button_, SIGNAL(clicked()), this, SLOT(addFileSlot()));
    layout->addWidget(add_button_);

    delete_button_ = new QPushButton ("Delete");
    connect (delete_button_, SIGNAL(clicked()), this, SLOT(deleteFileSlot()));
    layout->addWidget(delete_button_);
    layout->addStretch();

    open_button_ = new QPushButton ("Open");
    connect (open_button_, SIGNAL(clicked()), this, SLOT(openFileSlot()));
    layout->addWidget(open_button_);

    updateFileListSlot ();

    setLayout (layout);
}

void SQLiteConnectionWidget::addFileSlot ()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open SQLite3 File"));

    if (filename.size() > 0)
    {
        if (!connection_.hasFile(filename.toStdString()))
            connection_.addFile(filename.toStdString());
    }
}

void SQLiteConnectionWidget::deleteFileSlot ()
{
    QString filename = file_list_->currentItem()->text();
    if (filename.size() > 0)
    {
        assert (connection_.hasFile(filename.toStdString()));
        connection_.removeFile (filename.toStdString());
    }
}

void SQLiteConnectionWidget::openFileSlot ()
{
    QString filename = file_list_->currentItem()->text();
    if (filename.size() > 0)
    {
        assert (connection_.hasFile(filename.toStdString()));
        connection_.openFile(filename.toStdString());

        emit databaseOpenedSignal();
    }

}

void SQLiteConnectionWidget::updateFileListSlot ()
{
    file_list_->clear();

    for (auto it : connection_.fileList())
    {
        QListWidgetItem *item = new QListWidgetItem(tr(it.first.c_str()), file_list_);
        if (it.first == connection_.lastFilename())
            file_list_->setCurrentItem(item);
    }
}

