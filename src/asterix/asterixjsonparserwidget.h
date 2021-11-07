#ifndef ASTERIXJSONPARSERWIDGET_H
#define ASTERIXJSONPARSERWIDGET_H

#include <QWidget>

class ASTERIXJSONParser;

class QTableView;
class QSortFilterProxyModel;


class ASTERIXJSONParserWidget : public QWidget
{
    Q_OBJECT
public slots:
    void currentRowChanged(const QModelIndex& current, const QModelIndex& previous);

public:
    explicit ASTERIXJSONParserWidget(ASTERIXJSONParser& parser, QWidget* parent = nullptr);

    void setParser(ASTERIXJSONParser& parser);

    void resizeColumnsToContents();

private:
    ASTERIXJSONParser* parser_{nullptr};

    QTableView* table_view_{nullptr};
    QSortFilterProxyModel* proxy_model_{nullptr};
};

#endif // ASTERIXJSONPARSERWIDGET_H
