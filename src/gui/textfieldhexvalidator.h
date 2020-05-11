#ifndef TEXTFIELDHEXVALIDATOR_H
#define TEXTFIELDHEXVALIDATOR_H

#include <QValidator>
#include <QLineEdit>

#include "logger.h"

class TextFieldHexValidator : public QValidator
{
public:
    TextFieldHexValidator(unsigned int max_length, QObject* parent = nullptr)
        : QValidator(parent), max_length_(max_length)
    {
        //setNotation(QDoubleValidator::StandardNotation);
    }

    QValidator::State validate(QString &input, int &pos) const
    {
        if (!input.size())
            return QValidator::Acceptable;

        if (input.size() > max_length_ )
            return QValidator::Invalid;

        // match against needed regexp
        QRegExp rx("[0-9a-fA-F]+");
        if (rx.exactMatch(input)) {
            return QValidator::Acceptable;
        }
        return QValidator::Invalid;
    }

protected:
    unsigned int max_length_;
};

#endif // TEXTFIELDHEXVALIDATOR_H
