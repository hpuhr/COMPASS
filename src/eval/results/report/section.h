#ifndef EVALUATIONRESULTSREPORTSECTION_H
#define EVALUATIONRESULTSREPORTSECTION_H

#include "eval/results/report/treeitem.h"

#include <QWidget>

#include <memory>
#include <vector>

class EvaluationManager;

namespace EvaluationResultsReport
{
    using namespace std;

    class RootItem;
    class SectionContent;
    class SectionContentText;
    class SectionContentTable;

    class Section : public TreeItem
    {
    public:
        Section(string heading, TreeItem* parent_item, EvaluationManager& eval_man);

        virtual TreeItem *child(int row) override;
        virtual int childCount() const override;
        virtual int columnCount() const override;
        virtual QVariant data(int column) const override;
        virtual int row() const override;

        string heading() const;

        bool hasSubSection (const std::string& heading);
        Section& getSubSection (const std::string& heading);
        void addSubSection (const std::string& heading);

        QWidget* getContentWidget();

        bool hasText (const std::string& name);
        SectionContentText& getText (const std::string& name);
        void addText (const std::string& name);

        bool hasTable (const std::string& name);
        SectionContentTable& getTable (const std::string& name);
        void addTable (const std::string& name, unsigned int num_columns, vector<string> headings);

    protected:
        string heading_; // name same as heading
        EvaluationManager& eval_man_;

        vector<shared_ptr<SectionContent>> content_;

        unique_ptr<QWidget> content_widget_;

        vector<shared_ptr<Section>> sub_sections_;

        Section* findSubSection (const std::string& heading); // nullptr if not found
        SectionContentText* findText (const std::string& name); // nullptr if not found
        SectionContentTable* findTable (const std::string& name); // nullptr if not found

        void createContentWidget();
    };

}


#endif // EVALUATIONRESULTSREPORTSECTION_H
