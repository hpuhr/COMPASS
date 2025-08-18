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

#include "jpda.h"
#include "jpda_example.h"
#include "logger.h"
#include <Eigen/Dense>
#include <iostream>
#include <random>

using namespace reconstruction;

// Implementation of a simple constant velocity Kalman filter
class SimpleKalmanFilter : public AbstractKalmanFilter {
public:
    SimpleKalmanFilter(double process_noise = 1.0, double measurement_noise = 1.0)
        : process_noise_(process_noise), measurement_noise_(measurement_noise) {}
    
    void predict(Track& track, double timestamp) override {
        double dt = timestamp - track.timestamp();
        if (dt <= 0) return;
        
        // State transition matrix for constant velocity model
        Eigen::MatrixXd F = Eigen::MatrixXd::Identity(4, 4);
        F(0, 2) = dt;
        F(1, 3) = dt;
        
        // Process noise covariance
        Eigen::MatrixXd Q = Eigen::MatrixXd::Zero(4, 4);
        double dt3 = dt * dt * dt / 3.0;
        double dt2 = dt * dt / 2.0;
        Q(0, 0) = Q(1, 1) = dt3 * process_noise_;
        Q(0, 2) = Q(1, 3) = Q(2, 0) = Q(3, 1) = dt2 * process_noise_;
        Q(2, 2) = Q(3, 3) = dt * process_noise_;
        
        // Predict state
        track.state() = F * track.state();
        
        // Predict covariance
        track.covariance() = F * track.covariance() * F.transpose() + Q;
        
        // Update timestamp
        track.set_timestamp(timestamp);
    }
    
    std::pair<Eigen::VectorXd, Eigen::MatrixXd> innovation(
        const Track& track, const Measurement& measurement) override {
        
        // Measurement matrix (we only observe position)
        Eigen::MatrixXd H = Eigen::MatrixXd::Zero(2, 4);
        H(0, 0) = H(1, 1) = 1.0;
        
        // Measurement noise
        Eigen::MatrixXd R = Eigen::MatrixXd::Identity(2, 2) * measurement_noise_;
        
        // Calculate innovation
        Eigen::VectorXd predicted_measurement = H * track.state();
        Eigen::VectorXd innovation_vector = measurement.state() - predicted_measurement;
        
        // Calculate innovation covariance
        Eigen::MatrixXd innovation_covariance = H * track.covariance() * H.transpose() + R;
        
        return {innovation_vector, innovation_covariance};
    }
    
    void update(Track& track, const Measurement& measurement) override {
        // Measurement matrix (we only observe position)
        Eigen::MatrixXd H = Eigen::MatrixXd::Zero(2, 4);
        H(0, 0) = H(1, 1) = 1.0;
        
        // Measurement noise
        Eigen::MatrixXd R = Eigen::MatrixXd::Identity(2, 2) * measurement_noise_;
        
        // Calculate innovation and covariance
        auto [innovation_vector, innovation_covariance] = this->innovation(track, measurement);
        
        // Calculate Kalman gain
        Eigen::MatrixXd K = track.covariance() * H.transpose() * innovation_covariance.inverse();
        
        // Update state
        track.state() = track.state() + K * innovation_vector;
        
        // Update covariance
        Eigen::MatrixXd I = Eigen::MatrixXd::Identity(4, 4);
        track.covariance() = (I - K * H) * track.covariance();
    }
    
private:
    double process_noise_;
    double measurement_noise_;
};

// Helper function to create a simple track with position and velocity
Track createTrack(double x, double y, double vx, double vy, double timestamp) {
    Eigen::VectorXd state(4);
    state << x, y, vx, vy;
    
    Eigen::MatrixXd covariance = Eigen::MatrixXd::Identity(4, 4);
    covariance(0, 0) = covariance(1, 1) = 1.0; // Position uncertainty
    covariance(2, 2) = covariance(3, 3) = 0.1; // Velocity uncertainty
    
    return Track(timestamp, state, covariance);
}

// Helper function to create a position measurement
Measurement createMeasurement(double x, double y, double timestamp, double noise = 1.0) {
    Eigen::VectorXd state(2);
    state << x, y;
    
    Eigen::MatrixXd covariance = Eigen::MatrixXd::Identity(2, 2) * noise;
    
    return Measurement(timestamp, state, covariance);
}

int main() {
    // Initialize random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> noise(0.0, 1.0);
    
    // Create a Kalman filter
    auto kalman_filter = std::make_shared<SimpleKalmanFilter>(1.0, 1.0);
    
    // Create a JPDA associator
    JPDAAssociator jpda(kalman_filter, 9.0, 0.1);
    
    // Create some tracks
    std::vector<Track> tracks;
    tracks.push_back(createTrack(0.0, 0.0, 1.0, 1.0, 0.0));
    tracks.push_back(createTrack(10.0, 10.0, -1.0, -1.0, 0.0));
    
    // Simulate for 10 timesteps
    for (int t = 1; t <= 10; ++t) {
        double timestamp = t * 1.0;
        
        // Generate noisy measurements for each track
        std::vector<Measurement> measurements;
        
        // True positions of tracks at current time
        double track1_x = 0.0 + 1.0 * timestamp;
        double track1_y = 0.0 + 1.0 * timestamp;
        double track2_x = 10.0 - 1.0 * timestamp;
        double track2_y = 10.0 - 1.0 * timestamp;
        
        // Add noisy measurements (including clutter)
        measurements.push_back(createMeasurement(track1_x + noise(gen), track1_y + noise(gen), timestamp));
        measurements.push_back(createMeasurement(track2_x + noise(gen), track2_y + noise(gen), timestamp));
        
        // Add a clutter measurement
        if (t % 3 == 0) {
            measurements.push_back(createMeasurement(5.0 + noise(gen) * 3, 5.0 + noise(gen) * 3, timestamp));
        }
        
        // Associate measurements to tracks
        auto associations = jpda.associate(tracks, measurements, timestamp);
        
        // Print associations
        std::cout << "Time: " << timestamp << std::endl;
        for (size_t i = 0; i < tracks.size(); ++i) {
            std::cout << "  Track " << i << " associations:" << std::endl;
            for (const auto& [meas_idx, prob] : associations[i]) {
                if (meas_idx < 0) {
                    std::cout << "    Missed detection: " << prob << std::endl;
                } else {
                    std::cout << "    Measurement " << meas_idx << ": " << prob << std::endl;
                }
            }
            
            // Update track with most likely measurement
            double max_prob = 0.0;
            int best_meas_idx = -1;
            for (const auto& [meas_idx, prob] : associations[i]) {
                if (meas_idx >= 0 && prob > max_prob) {
                    max_prob = prob;
                    best_meas_idx = meas_idx;
                }
            }
            
            if (best_meas_idx >= 0) {
                kalman_filter->update(tracks[i], measurements[best_meas_idx]);
                std::cout << "    Updated with measurement " << best_meas_idx << std::endl;
            }
            
            // Print track state
            std::cout << "    Position: (" << tracks[i].state()(0) << ", " << tracks[i].state()(1) << ")" << std::endl;
            std::cout << "    Velocity: (" << tracks[i].state()(2) << ", " << tracks[i].state()(3) << ")" << std::endl;
        }
        std::cout << std::endl;
    }
    
    return 0;
}
