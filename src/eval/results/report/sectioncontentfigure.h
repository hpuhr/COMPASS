#ifndef EVALUATIONRESULTSREPORTSECTIONCONTENTFIGURE_H
#define EVALUATIONRESULTSREPORTSECTIONCONTENTFIGURE_H

#include "eval/results/report/sectioncontent.h"

#include <QObject>

#include "json.hpp"

namespace EvaluationResultsReport
{
    using namespace std;

class SectionContentFigure : public QObject, public SectionContent
{
    Q_OBJECT

public slots:
    void viewSlot();

public:
    SectionContentFigure(const string& name, const string& caption,
                         std::unique_ptr<nlohmann::json::object_t> viewable_data,
                         Section* parent_section, EvaluationManager& eval_man); // const string& path

    virtual void addToLayout (QVBoxLayout* layout) override;

    virtual void accept(LatexVisitor& v) const override;

    void view () const;
    std::string getSubPath() const;

protected:
    string caption_;
    std::unique_ptr<nlohmann::json::object_t> viewable_data_;
};

}
#endif // EVALUATIONRESULTSREPORTSECTIONCONTENTFIGURE_H
