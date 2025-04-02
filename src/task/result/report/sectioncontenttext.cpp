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

#include "task/result/report/sectioncontenttext.h"

#include "taskmanager.h"
//#include "latexvisitor.h"
#include "logger.h"

#include <QLabel>
#include <QVBoxLayout>

#include <cassert>

namespace ResultReport
{

const std::string SectionContentText::FieldTexts = "texts";

/**
 */
SectionContentText::SectionContentText(unsigned int id,
                                       const std::string& name, 
                                       Section* parent_section)
:   SectionContent(Type::Text, id, name, parent_section)
{
}

/**
 */
SectionContentText::SectionContentText(Section* parent_section)
:   SectionContent(Type::Text, parent_section)
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

/**
 */
void SectionContentText::toJSON_impl(nlohmann::json& root_node) const
{
    root_node[ FieldTexts ] = texts_;
}

/**
 */
bool SectionContentText::fromJSON_impl(const nlohmann::json& j)
{
    if (!j.is_object() ||
        !j.contains(FieldTexts))
    {
        logerr << "SectionContentText: fromJSON: Error: Section content text does not obtain needed fields";
        return false;
    }

    texts_ = j[ FieldTexts ].get<std::vector<std::string>>();

    return true;
}

}
