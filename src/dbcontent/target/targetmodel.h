#pragma once

#include "target.h"
#include "json.hpp"

#include <QAbstractItemModel>

#include <map>

#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index_container.hpp>

class DBContentManager;

namespace dbContent {

    class ReconstructorTarget;

struct target_tag
{
};

typedef boost::multi_index_container<
    Target,
    boost::multi_index::indexed_by<
        boost::multi_index::random_access<>,  // this index represents insertion order
        boost::multi_index::ordered_unique<boost::multi_index::tag<target_tag>,
            boost::multi_index::member<
        Target, const unsigned int, &Target::utn_> >
        > >
    TargetCache;

class TargetModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    TargetModel(DBContentManager& dbcont_manager);
    virtual ~TargetModel();

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant& value, int role) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    const dbContent::Target& getTargetOf (const QModelIndex& index);

    void setUseTargetData (unsigned int utn, bool value);
    void setTargetDataComment (unsigned int utn, std::string comment);
    void setUseAllTargetData (bool value);
    void clearComments ();

    void setUseByFilter ();

    bool hasTargetsInfo();
    void clearTargetsInfo();
    bool existsTarget(unsigned int utn) const;
    //void createNewTarget(unsigned int utn);
    void createNewTargets(const std::map<unsigned int, dbContent::ReconstructorTarget>& targets);
    dbContent::Target& target(unsigned int utn);
    const dbContent::Target& target(unsigned int utn) const;
    unsigned int size() const;

    void removeDBContentFromTargets(const std::string& dbcont_name);

    void storeTargetsEvalInfo();

    nlohmann::json asJSON() const;
    nlohmann::json targetAsJSON(unsigned int utn) const;
    nlohmann::json targetStatsAsJSON() const;
    nlohmann::json utnsAsJSON() const;

    void loadFromDB();
    void saveToDB();
    void saveToDB(unsigned int utn);

    void clear();

    const QStringList& tableHeaders() const { return table_columns_; }

    const std::vector<int>& mainColumns() const { return main_columns_; }
    const std::vector<int>& evalColumns() const { return eval_columns_; }
    const std::vector<int>& durationColumns() const { return duration_columns_; }
    const std::vector<int>& modeSColumns() const { return mode_s_columns_; }
    const std::vector<int>& modeACColumns() const { return mode_ac_columns_; }

    bool showMainColumns() { return show_main_columns_; }
    bool showEvalColumns() { return show_eval_columns_; }
    bool showDurationColumns() { return show_duration_columns_; }
    bool showModeSColumns() { return show_mode_s_columns_; }
    bool showModeACColumns() { return show_mode_ac_columns_; }

    void showMainColumns(bool show);
    void showEvalColumns(bool show);
    void showDurationColumns(bool show);
    void showModeSColumns(bool show);
    void showModeACColumns(bool show);

    void updateEvalItems();

    nlohmann::json rawCellData(int row, int column) const;
    unsigned int rowStyle(int row) const;
    unsigned int columnStyle(int column) const;

    enum Columns
    {
        ColUTN = 0,
        ColComment,
        ColCategory,
        ColUseEval,
        ColUseEvalDetails,
        ColNumUpdates, 
        ColBegin, 
        ColEnd, 
        ColDuration, 
        ColACIDs, 
        ColACADs, 
        ColMode3A, 
        ColModeCMin, 
        ColModeCMax
    };

protected:
    DBContentManager& dbcont_manager_;

    QStringList               table_columns_   { "UTN", "Comment", "Category", "Use Eval", "Use Eval Details",
                               "#Updates", "Begin", "End", "Duration", "ACIDs", "ACADs", "M3/A", "MC Min", "MC Max"};
    std::vector<int>          main_columns_    { ColUTN, ColComment, ColCategory };
    std::vector<int>          eval_columns_    { ColUseEval, ColUseEvalDetails };
    std::vector<int>          duration_columns_{ ColNumUpdates, ColBegin, ColEnd, ColDuration };
    std::vector<int>          mode_s_columns_  { ColACIDs, ColACADs };
    std::vector<int>          mode_ac_columns_ { ColMode3A, ColModeCMin, ColModeCMax };

    bool show_main_columns_     = true;
    bool show_eval_columns_     = false;
    bool show_duration_columns_ = false;
    bool show_mode_s_columns_   = true;
    bool show_mode_ac_columns_  = false;
    
    TargetCache target_data_;
};

}

