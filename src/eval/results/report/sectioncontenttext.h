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

#ifndef EVALUATIONRESULTSREPORTSECTIONCONTENTTEXT_H
#define EVALUATIONRESULTSREPORTSECTIONCONTENTTEXT_H

#include "eval/results/report/sectioncontent.h"

#include <vector>

namespace EvaluationResultsReport
{
    using namespace std;

    class SectionContentText : public SectionContent
    {
    public:
        SectionContentText(const string& name, Section* parent_section, EvaluationManager& eval_man);

        void addText (const string& text);

        virtual void addToLayout (QVBoxLayout* layout) override;
        virtual void accept(LatexVisitor& v) override;

        const vector<string>& texts() const;

    protected:
        vector<string> texts_;

    };


}
#endif // EVALUATIONRESULTSREPORTSECTIONCONTENTTEXT_H
