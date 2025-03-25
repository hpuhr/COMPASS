/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "result/report/sectioncontenttext.h"

#include "resultmanager.h"
//#include "latexvisitor.h"
#include "logger.h"

#include <QLabel>
#include <QVBoxLayout>

#include <cassert>

namespace ResultReport
{

/**
 */
SectionContentText::SectionContentText(const std::string& name, 
                                       Section* parent_section, 
                                       ResultManager& result_man)
:   SectionContent(name, parent_section, result_man)
{

}

/**
 */
void SectionContentText::addText(const std::string& text)
{
    texts_.push_back(text);
}

/**
 */
void SectionContentText::addToLayout (QVBoxLayout* layout)
{
    assert (layout);

    for (auto& text : texts_)
    {
        QLabel* label = new QLabel((text+"\n\n").c_str());
        label->setWordWrap(true);

        layout->addWidget(label);
    }
}

/**
 */
void SectionContentText::accept(LatexVisitor& v)
{
    loginf << "SectionContentText: accept";
    //@TODO
    //v.visit(this);
}

/**
 */
const std::vector<std::string>& SectionContentText::texts() const
{
    return texts_;
}

}
