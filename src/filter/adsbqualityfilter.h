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

#ifndef ADSBQUALITYFILTER_H
#define ADSBQUALITYFILTER_H

#include "dbfilter.h"

class ADSBQualityFilter : public DBFilter
{
public:
    ADSBQualityFilter(const std::string& class_id, const std::string& instance_id,
                      Configurable* parent);
    virtual ~ADSBQualityFilter();

    virtual std::string getConditionString(const std::string& dbo_name, bool& first,
                                           std::vector<DBOVariable*>& filtered_variables);

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    virtual bool filters(const std::string& dbo_name);
    virtual void reset();

    virtual void saveViewPointConditions (nlohmann::json& filters);
    virtual void loadViewPointConditions (const nlohmann::json& filters);

    bool useV0() const;
    void useV0(bool value);

    bool useV1() const;
    void useV1(bool value);

    bool useV2() const;
    void useV2(bool value);

    bool useMinNUCP() const;
    void useMinNUCP(bool value);

    unsigned int minNUCP() const;
    void minNUCP(unsigned int value);

    bool useMinNIC() const;
    void useMinNIC(bool value);

    unsigned int minNIC() const;
    void minNIC(unsigned int value);

    bool useMinNACp() const;
    void useMinNACp(bool value);

    unsigned int minNACp() const;
    void minNACp(unsigned int value);

    bool useMinSILv1() const;
    void useMinSILv1(bool value);

    unsigned int minSILv1() const;
    void minSILv1(unsigned int value);

    bool useMinSILv2() const;
    void useMinSILv2(bool value);

    unsigned int minSILv2() const;
    void minSILv2(unsigned int value);

protected:
    bool use_v0_ {false};
    bool use_v1_ {false};
    bool use_v2_ {false};

    bool use_min_nucp_ {false};
    unsigned int min_nucp_ {0};

    bool use_min_nic_ {false};
    unsigned int min_nic_ {0};

    bool use_min_nacp_ {false};
    unsigned int min_nacp_ {0};

    bool use_min_sil_v1_ {false};
    unsigned int min_sil_v1_ {0};

    bool use_min_sil_v2_ {false};
    unsigned int min_sil_v2_ {0};

    virtual void checkSubConfigurables();
};

#endif // ADSBQUALITYFILTER_H
