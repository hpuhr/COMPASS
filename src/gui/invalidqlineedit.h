#ifndef INVALIDQLINEEDIT_H
#define INVALIDQLINEEDIT_H

#include <QLineEdit>

class InvalidQLineEdit : public QLineEdit
{
  public:
    explicit InvalidQLineEdit(QWidget* parent = Q_NULLPTR) : QLineEdit(parent) {}
    explicit InvalidQLineEdit(const QString& text, QWidget* parent = Q_NULLPTR)
        : QLineEdit(text, parent)
    {
    }

    void setValid(bool value)
    {
        if (value)
            setStyleSheet(
                "QLineEdit { background: rgb(255, 255, 255); selection-background-color:"
                " rgb(200, 200, 200); }");
        else
            setStyleSheet(
                "QLineEdit { background: rgb(255, 100, 100); selection-background-color:"
                " rgb(255, 200, 200); }");
    }
};

#endif  // INVALIDQLINEEDIT_H
