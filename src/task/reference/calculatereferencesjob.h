#ifndef CALCULATEREFERENCESJOB_H
#define CALCULATEREFERENCESJOB_H

#include "job.h"
#include "dbcontent/dbcontentcache.h"
#include "calculatereferencestarget.h"
#include "calculatereferencesstatusdialog.h"

#include <QMap>
#include <QPair>

class CalculateReferencesTask;
class CalculateReferencesStatusDialog;
class DBContent;


class CalculateReferencesJob : public Job
{
    Q_OBJECT

public slots:
    void insertDoneSlot();

public:
    CalculateReferencesJob(CalculateReferencesTask& task,
                           CalculateReferencesStatusDialog& status_dialog,
                           std::shared_ptr<dbContent::Cache> cache);
    virtual ~CalculateReferencesJob();

    nlohmann::json viewPointsJSON() const { return viewpoint_json_; }

    virtual void run();

protected:
    CalculateReferencesTask& task_;
    CalculateReferencesStatusDialog& status_dialog_;
    std::shared_ptr<dbContent::Cache> cache_;

    std::vector<std::unique_ptr<CalculateReferences::Target>> targets_;

    std::shared_ptr<Buffer> result_;

    PositionCountsMapStruct used_pos_counts_; // dbcont -> used, unused
    CalcInfoVectorStruct info_;
    std::map<unsigned int, unsigned int> reftraj_counts_; // utn -> cnt

    bool insert_done_ {false};

    nlohmann::json viewpoint_json_;

    void createTargets();
    void filterPositionUsage(std::map<unsigned int, std::unique_ptr<CalculateReferences::Target>>& target_map);
    void finalizeTargets();
    void calculateReferences();
    void writeReferences();
};

#endif // CALCULATEREFERENCESJOB_H
