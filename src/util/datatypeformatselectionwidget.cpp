#include "datatypeformatselectionwidget.h"
#include "logger.h"

DataTypeFormatSelectionWidget::DataTypeFormatSelectionWidget(std::string& data_type_str, Format& format)
    : QPushButton (), data_type_str_(data_type_str), format_(format)
{
    logdbg  << "DataTypeFormatSelectionWidget: constructor";

    //update (data_type_str, format);

    showValues();
    createMenu();

    connect( &menu_, SIGNAL(triggered(QAction*)), this, SLOT(triggerSlot(QAction*)));
    connect( this, SIGNAL(clicked()), this, SLOT(showMenuSlot()) );
}

DataTypeFormatSelectionWidget::~DataTypeFormatSelectionWidget()
{
}

//void DataTypeFormatSelectionWidget::update (std::string& data_type_str, Format& format)
//{
//    loginf << "DataTypeFormatSelectionWidget: update: data type '" << data_type_str
//           << "' format '" << format << "'";

//    data_type_str_ = data_type_str;
//    format_ = format_;

//    showValues();
//}

void DataTypeFormatSelectionWidget::showValues()
{
    if (data_type_str_.size() && format_.size())
    {
        std::string tmp_str = data_type_str_ + +":"+ format_;
        setText (QString::fromStdString(tmp_str));
    }
    else
        setText("");
}

void DataTypeFormatSelectionWidget::createMenu()
{
    logdbg  << "DataTypeFormatSelectionWidget: createMenu";
    //    menu_.addAction( "" );

    for (auto dt_it : format_.getAllFormatOptions())
    {
        logdbg  << "DataTypeFormatSelectionWidget: createMenu: dt " << static_cast<unsigned> (dt_it.first);
        std::string data_type_str = Property::asString(dt_it.first);
        logdbg  << "DataTypeFormatSelectionWidget: createMenu: dt str " << data_type_str;
        QMenu* m2 = menu_.addMenu( QString::fromStdString(data_type_str));

        for (auto ft_it: dt_it.second)
        {
            logdbg  << "DataTypeFormatSelectionWidget: createMenu: format " << ft_it;
            QAction* action = m2->addAction(QString::fromStdString(ft_it));

            QVariantMap vmap;
            vmap.insert( QString::fromStdString(data_type_str), QVariant(QString::fromStdString(ft_it)));
            action->setData( QVariant( vmap ) );
        }
    }
    logdbg  << "DataTypeFormatSelectionWidget: createMenu: end";
}

void DataTypeFormatSelectionWidget::showMenuSlot()
{
    logdbg  << "DataTypeFormatSelectionWidget: showMenuSlot";

    menu_.exec( QCursor::pos() );
}

void DataTypeFormatSelectionWidget::triggerSlot( QAction* action )
{
    loginf  << "DataTypeFormatSelectionWidget: triggerSlot";

    QVariantMap vmap = action->data().toMap();
    std::string data_type_str, format_str;

    if (action->text().size() != 0)
    {
        data_type_str = vmap.begin().key().toStdString();
        format_str = vmap.begin().value().toString().toStdString();
    }

    loginf  << "DataTypeFormatSelectionWidget: triggerSlot: got data type '" << data_type_str
            << "' format '" << format_str << "'";

    if (data_type_str.size() && format_str.size())
    {
        PropertyDataType data_type = Property::asDataType(data_type_str);
        format_.set(data_type, format_str);


        std::string tmp_str = data_type_str +":"+ format_;
        setText (QString::fromStdString(tmp_str));
    }
    else
    {
        format_.set(PropertyDataType::BOOL, "");
        setText ("");
    }

    //  emit selectionChanged();
}
