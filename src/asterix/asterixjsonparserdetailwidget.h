#ifndef ASTERIXJSONPARSERDETAILWIDGET_H
#define ASTERIXJSONPARSERDETAILWIDGET_H

#include <QWidget>

class ASTERIXJSONParser;

class QLabel;

class ASTERIXJSONParserDetailWidget : public QWidget
{
    Q_OBJECT

public slots:
    void currentIndexChangedSlot (unsigned int index);

signals:


public:
    explicit ASTERIXJSONParserDetailWidget(ASTERIXJSONParser& parser, QWidget *parent = nullptr);

private:
    ASTERIXJSONParser& parser_;

    QLabel* info_label_ {nullptr}; // shows type of mapping, or missing details
    QLabel* json_key_label_ {nullptr};

    QLabel* dbo_var_label_ {nullptr};


};

#endif // ASTERIXJSONPARSERDETAILWIDGET_H
