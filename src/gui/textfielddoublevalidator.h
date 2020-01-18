#ifndef TEXTFIELDDOUBLEVALIDATOR_H
#define TEXTFIELDDOUBLEVALIDATOR_H

#include <QDoubleValidator>

#include "logger.h"

class TextFieldDoubleValidator : public QDoubleValidator
{
public:
    TextFieldDoubleValidator (QObject* parent=nullptr)
        : QDoubleValidator(parent)
    {
        setNotation(QDoubleValidator::StandardNotation);
    }

    TextFieldDoubleValidator (double bottom, double top, int decimals, QObject* parent=nullptr)
        : QDoubleValidator(bottom, top, decimals, parent)
    {
        setNotation(QDoubleValidator::StandardNotation);
    }

    QValidator::State validate(QString& s, int& pos) const
    {
        if (s.isEmpty()) //|| (s.startsWith("-") && s.length() == 1)
            return QValidator::Intermediate;

        if (decimals())
        {
            if ((s.startsWith(".") && s.size() == 1)
                    || (s.startsWith("0.") && s.size() == 2))
                return QValidator::Intermediate;

            // check length of decimal places
            QChar point = '.';
            if(s.indexOf(point) != -1)
            {
                int lengthDecimals = s.length() - s.indexOf(point) - 1;

                if (lengthDecimals > decimals())
                    return QValidator::Invalid;
            }

            // check range of value
            bool ok;
            double value = s.toDouble(&ok);

            if (ok && bottom() <= value && value <= top())
                return QValidator::Acceptable;

            return QValidator::Invalid;

        }
        else // int
        {
            // check range of value
            bool ok;
            int value = s.toInt(&ok);

            if (ok && bottom() <= value && value <= top())
                return QValidator::Acceptable;

            return QValidator::Invalid;
        }
    }

};

#endif // TEXTFIELDDOUBLEVALIDATOR_H
