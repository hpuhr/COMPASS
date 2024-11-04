#pragma once

struct RadarBiasInfo
{
    bool bias_valid_ {false};
    double azimuth_bias_deg_ {0};
    double range_bias_m_ {0};
    double range_gain_ {1};
};
