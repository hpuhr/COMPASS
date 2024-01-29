#ifndef DBCONTENT_TARGETMODEL_H
#define DBCONTENT_TARGETMODEL_H

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
    void createNewTarget(unsigned int utn);
    dbContent::Target& target(unsigned int utn);
    const dbContent::Target& target(unsigned int utn) const;

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

protected:
    DBContentManager& dbcont_manager_;

    QStringList table_columns_ {"Use", "UTN", "Comment", "#Updates", "Begin", "End", "Duration", "ACIDs", "ACADs",
                                "M3/A", "MC Min", "MC Max", "MOPS"};

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

#endif // DBCONTENT_TARGETMODEL_H
