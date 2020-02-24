#ifndef TEXTFIELDDOUBLEVALIDATOR_H
#define TEXTFIELDDOUBLEVALIDATOR_H

#include <QDoubleValidator>
#include <QLineEdit>

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
            if ((s.startsWith("0") && s.size() == 1)
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

    static void displayValidityAsColor (QLineEdit* line_edit)
    {
        assert (line_edit);

        if (line_edit->hasAcceptableInput())
            line_edit->setStyleSheet("QLineEdit { background: rgb(255, 255, 255); "
                                                     "selection-background-color: rgb(200, 200, 200); }");
        else
            line_edit->setStyleSheet("QLineEdit { background: rgb(255, 100, 100); "
                                                     "selection-background-color: rgb(255, 200, 200); }");
    }

};

#endif // TEXTFIELDDOUBLEVALIDATOR_H
