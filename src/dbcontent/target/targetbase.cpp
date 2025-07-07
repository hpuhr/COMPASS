#include "targetbase.h"

const nlohmann::json TargetBase::emitter_specs_ = {
    { "LightAircraft", {
                          {"avg_size_m", 10},
                          {"max_speed_knots", 140},
                          {"max_accel_mps2", 2.0},
                          {"process_noise_factor_ground", 1.5},
                          {"process_noise_factor_air", 1.5},
                          {"ground_only", false}
                      }},
    { "SmallAircraft", {
                          {"avg_size_m", 20},
                          {"max_speed_knots", 250},
                          {"max_accel_mps2", 2.5},
                          {"process_noise_factor_ground", 1.5},
                          {"process_noise_factor_air", 1.5},
                          {"ground_only", false}
                      }},
    { "MediumAircraft", {
                           {"avg_size_m", 35},
                           {"max_speed_knots", 450},
                           {"max_accel_mps2", 3.0},
                           {"process_noise_factor_ground", 1.0},
                           {"process_noise_factor_air", 1.0},
                           {"ground_only", false}
                       }},
    { "HighVortexLargeAircraft", {
                                    {"avg_size_m", 60},
                                    {"max_speed_knots", 480},
                                    {"max_accel_mps2", 2.5},
                                    {"process_noise_factor_ground", 1.0},
                                    {"process_noise_factor_air", 0.8},
                                    {"ground_only", false}
                                }},
    { "HeavyAircraft", {
                          {"avg_size_m", 70},
                          {"max_speed_knots", 510},
                          {"max_accel_mps2", 2.2},
                          {"process_noise_factor_ground", 1.0},
                          {"process_noise_factor_air", 0.8},
                          {"ground_only", false}
                      }},
    { "HighSpeedManoeuvrable", {
                                  {"avg_size_m", 20},
                                  {"max_speed_knots", 700},
                                  {"max_accel_mps2", 9.0},
                                  {"process_noise_factor_ground", 1.5},
                                  {"process_noise_factor_air", 2.0},
                                  {"ground_only", false}
                              }},
    { "Rotocraft", {
                      {"avg_size_m", 15},
                      {"max_speed_knots", 160},
                      {"max_accel_mps2", 1.5},
                      {"process_noise_factor_ground", 1.0},
                      {"process_noise_factor_air", 1.0},
                      {"ground_only", false}
                  }},
    { "OtherAirborne", {
                          {"avg_size_m", 12},
                          {"max_speed_knots", 120},
                          {"max_accel_mps2", 1.0},
                          {"process_noise_factor_ground", 1.5},
                          {"process_noise_factor_air", 1.5},
                          {"ground_only", false}
                      }},
    { "AnyAircraft", {
                        {"avg_size_m", 35},
                        {"max_speed_knots", 450},
                        {"max_accel_mps2", 3.0},
                        {"process_noise_factor_ground", 1.0},
                        {"process_noise_factor_air", 1.0},
                        {"ground_only", false}
                    }},
    { "Vehicle", {
                    {"avg_size_m", 8},
                    {"max_speed_knots", 50},
                    {"max_accel_mps2", 1.2},
                    {"process_noise_factor_ground", 1.5},
                    {"ground_only", true}
                }},
    { "Obstacle", {
                     {"avg_size_m", 1},
                     {"max_speed_knots", 0},
                     {"max_accel_mps2", 0},
                     {"process_noise_factor_ground", 0.01},
                     {"ground_only", true}
                 }},
    { "FFT", {
                     {"avg_size_m", 1},
                     {"max_speed_knots", 0},
                     {"max_accel_mps2", 0},
                     {"process_noise_factor_ground", 0.01},
                     {"ground_only", true}
                 }}
};

TargetBase::TargetBase()
{}

std::string TargetBase::emitterCategoryStr() const
{
    return toString(targetCategory());
}


