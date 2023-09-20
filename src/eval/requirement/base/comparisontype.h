#ifndef EVALUATIONREQUIREMENTCOMPARISONTYPE
#define EVALUATIONREQUIREMENTCOMPARISONTYPE

#include <string>
#include <stdexcept>

namespace EvaluationRequirement
{

enum COMPARISON_TYPE {
    LESS_THAN=0, // <
    LESS_THAN_OR_EQUAL, // <=
    GREATER_THAN, // >
    GREATER_THAN_OR_EQUAL // >=
};

extern std::string comparisonTypeString(COMPARISON_TYPE type);
extern std::string comparisonTypeLongString(COMPARISON_TYPE type);

}

#endif // EVALUATIONREQUIREMENTCOMPARISONTYPE
