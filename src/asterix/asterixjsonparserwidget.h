#ifndef ASTERIXJSONPARSERWIDGET_H
#define ASTERIXJSONPARSERWIDGET_H

#include <QWidget>

class ASTERIXJSONParser;

class ASTERIXJSONParserWidget : public QWidget
{
    Q_OBJECT
  public slots:

public:
  explicit ASTERIXJSONParserWidget(ASTERIXJSONParser& parser, QWidget* parent = nullptr);

  void setParser(ASTERIXJSONParser& parser);

private:
  ASTERIXJSONParser* parser_{nullptr};
};

#endif // ASTERIXJSONPARSERWIDGET_H
