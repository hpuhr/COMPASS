#pragma once

#include "target.h"
#include "configurable.h"
#include "json.hpp"

#include <QAbstractItemModel>

#include <memory>
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

class TargetModel : public QAbstractItemModel, public Configurable
{
    Q_OBJECT

public:
    TargetModel(const std::string& class_id, const std::string& instance_id, DBContentManager& dbcont_manager);
    virtual ~TargetModel();

    virtual void generateSubConfigurable(const std::string& class_id, const std::string& instance_id) override {};

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

    nlohmann::json asJSON() const;
    nlohmann::json targetAsJSON(unsigned int utn) const;
    nlohmann::json targetStatsAsJSON() const;
    nlohmann::json utnsAsJSON() const;

    void loadFromDB();
    void saveToDB();
    void saveToDB(unsigned int utn);

    void clear();

    // use filter stuff

    bool removeShortTargets() const;
    void removeShortTargets(bool value);

    unsigned int removeShortTargetsMinUpdates() const;
    void removeShortTargetsMinUpdates(unsigned int value);

    double removeShortTargetsMinDuration() const;
    void removeShortTargetsMinDuration(double value);

    bool removePsrOnlyTargets() const;
    void removePsrOnlyTargets(bool value);

    bool filterModeACodes() const;
    void filterModeACodes(bool value);
    bool filterModeACodeBlacklist() const;
    void filterModeACodeBlacklist(bool value);

    bool removeModeCValues() const;
    void removeModeCValues(bool value);

    float removeModeCMinValue() const;
    void removeModeCMinValue(float value);

    std::string filterModeACodeValues() const;
    std::set<std::pair<int,int>> filterModeACodeData() const; // single ma,-1 or range ma1,ma2
    void filterModeACodeValues(const std::string& value);

    bool filterTargetAddresses() const;
    void filterTargetAddresses(bool value);
    bool filterTargetAddressesBlacklist() const;
    void filterTargetAddressesBlacklist(bool value);

    std::string filterTargetAddressValues() const;
    std::set<unsigned int> filterTargetAddressData() const;
    void filterTargetAddressValues(const std::string& value);

    bool removeModeACOnlys() const;
    void removeModeACOnlys(bool value);

    bool removeNotDetectedDBContents() const;
    void removeNotDetectedDBContents(bool value);

    bool removeNotDetectedDBContent(const std::string& dbcontent_name) const;
    void removeNotDetectedDBContents(const std::string& dbcontent_name, bool value);

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

    static const QStringList      TableHeaders;

    // static const std::vector<int> MainColumns;
    // static const std::vector<int> EvalColumns;
    // static const std::vector<int> DurationColumns;
    // static const std::vector<int> ModeSColumns;
    // static const std::vector<int> ModeACColumns;

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

    // utn use filter stuff

    bool remove_short_targets_ {true};
    unsigned int remove_short_targets_min_updates_ {10};
    double remove_short_targets_min_duration_ {60.0};

    bool remove_psr_only_targets_ {true};
    bool remove_modeac_onlys_ {false};

    bool filter_mode_a_codes_{false};
    bool filter_mode_a_code_blacklist_{true};
    std::string filter_mode_a_code_values_;

    bool remove_mode_c_values_{false};
    float remove_mode_c_min_value_;

    bool filter_target_addresses_{false};
    bool filter_target_addresses_blacklist_{true};
    std::string filter_target_address_values_;

    bool remove_not_detected_dbos_{false};
    nlohmann::json remove_not_detected_dbo_values_;

    virtual void checkSubConfigurables() override {};
};

}

