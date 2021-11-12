#ifndef ASTERIXJSONPARSERWIDGET_H
#define ASTERIXJSONPARSERWIDGET_H

#include <QWidget>

class ASTERIXJSONParser;
class ASTERIXJSONParserDetailWidget;

class QSplitter;
class QTableView;
class QSortFilterProxyModel;


class ASTERIXJSONParserWidget : public QWidget
{
    Q_OBJECT
public slots:
    void currentRowChanged(const QModelIndex& current, const QModelIndex& previous);

public:
    explicit ASTERIXJSONParserWidget(ASTERIXJSONParser& parser, QWidget* parent = nullptr);
    virtual ~ASTERIXJSONParserWidget();

    void resizeColumnsToContents();

    void selectModelRow (unsigned int row);

private:
    ASTERIXJSONParser& parser_;

    QSplitter* splitter_{nullptr};
    QTableView* table_view_{nullptr};
    QSortFilterProxyModel* proxy_model_{nullptr};

    ASTERIXJSONParserDetailWidget* detail_widget_{nullptr};
};

#endif // ASTERIXJSONPARSERWIDGET_H
