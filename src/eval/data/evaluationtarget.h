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

#pragma once

#include "dbcontent/target/target.h"
#include "evaluationdefs.h"

/**
 */
class EvaluationTarget : public dbContent::Target
{
public:
    typedef std::function<bool(const Evaluation::RequirementSumResultID&)> InterestEnabledFunc;
    typedef std::map<Evaluation::RequirementSumResultID, double>           InterestMap;

    EvaluationTarget(unsigned int utn, 
                     const nlohmann::json& info);
    virtual ~EvaluationTarget();

    void interestFactors(const InterestMap& interests);
    const InterestMap& interestFactors() const;
    double totalInterest(const InterestEnabledFunc& enabled_func = InterestEnabledFunc()) const;

    void dbContentRef(const std::string& db_content);
    std::string dbContentRef() const;

    void dbContentTest(const std::string& db_content);
    std::string dbContentTest() const;

    unsigned int refCount() const;
    unsigned int testCount() const;

    static const std::string KEY_INTEREST_FACTORS;
    static const std::string KEY_DBCONTENT_REF;
    static const std::string KEY_DBCONTENT_TEST;

    static const std::string FieldInterestID;
    static const std::string FieldInterestFactor;

protected:
    bool readInterestFactors(InterestMap& ifactors, const nlohmann::json& j);
    void writeInterestFactors(const InterestMap& ifactors, nlohmann::json& j);

    mutable InterestMap interest_factors_;
};
