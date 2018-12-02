#ifndef JSONOBJECTPARSERWIDGET_H
#define JSONOBJECTPARSERWIDGET_H

#include <QWidget>

class JSONObjectParser;

class JSONObjectParserWidget : public QWidget
{
    Q_OBJECT
public:
    explicit JSONObjectParserWidget(JSONObjectParser& parser, QWidget *parent = nullptr);

    void setParser (JSONObjectParser& parser);
private:
    JSONObjectParser* parser_ {nullptr};
};

#endif // JSONOBJECTPARSERWIDGET_H
