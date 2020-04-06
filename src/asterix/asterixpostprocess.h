#ifndef ASTERIXPOSTPROCESS_H
#define ASTERIXPOSTPROCESS_H

#include "json.hpp"

class ASTERIXImportTask;

class ASTERIXPostProcess
{
public:
    ASTERIXPostProcess();

    void postProcess (unsigned int category, nlohmann::json& record);

protected:
    friend class ASTERIXImportTask; // uses the members for config

    bool override_active_ {false};
    unsigned int override_sac_org_ {0};
    unsigned int override_sic_org_ {0};

    unsigned int override_sac_new_ {0};
    unsigned int override_sic_new_ {0};

    float override_tod_offset_ {0};

    std::map<std::pair<unsigned int, unsigned int>, double> cat002_last_tod_period_;
    std::map<std::pair<unsigned int, unsigned int>, double> cat002_last_tod_;

    void postProcessCAT001 (int sac, int sic, nlohmann::json& record);
    void postProcessCAT002 (int sac, int sic, nlohmann::json& record);
    void postProcessCAT020 (int sac, int sic, nlohmann::json& record);
    void postProcessCAT021 (int sac, int sic, nlohmann::json& record);
    void postProcessCAT048 (int sac, int sic, nlohmann::json& record);
    void postProcessCAT062 (int sac, int sic, nlohmann::json& record);
};

#endif // ASTERIXPOSTPROCESS_H
