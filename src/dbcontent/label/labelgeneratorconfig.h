#pragma once

#include "json.hpp"

namespace dbContent
{

struct LabelGeneratorConfig
{
    LabelGeneratorConfig();

    bool auto_label_ {true};

    nlohmann::json label_directions_;
    nlohmann::json label_lines_;
    nlohmann::json label_config_;
    nlohmann::json label_ds_ids_; // dsid str -> label flag

    bool declutter_labels_ {true};
    unsigned int max_declutter_labels_ {200};

    bool filter_mode3a_active_;
    std::string filter_mode3a_values_;

    bool filter_modec_min_active_;
    float filter_modec_min_value_ {0};
    bool filter_modec_max_active_;
    float filter_modec_max_value_ {0};
    bool filter_modec_null_wanted_ {false};

    bool filter_ti_active_;
    std::string filter_ti_values_;

    bool filter_ta_active_;
    std::string filter_ta_values_;

    bool filter_primary_only_active_ {false};

    float label_opacity_ {0.9};

    bool  auto_lod_    {true};
    float current_lod_ {1}; // 1, 2, 3, float for filter function

    bool use_utn_as_id_ {false};
};

}

