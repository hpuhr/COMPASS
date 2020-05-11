#ifndef TEXTFIELDOCTVALIDATOR_H
#define TEXTFIELDOCTVALIDATOR_H

#include <QValidator>
#include <QLineEdit>

#include "logger.h"

class TextFieldOctValidator : public QValidator
{
public:
    TextFieldOctValidator(unsigned int max_length, QObject* parent = nullptr)
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
        QRegExp rx("[0-7]+");
        if (rx.exactMatch(input)) {
            return QValidator::Acceptable;
        }
        return QValidator::Invalid;
    }

protected:
    unsigned int max_length_;
};


#endif // TEXTFIELDOCTVALIDATOR_H
