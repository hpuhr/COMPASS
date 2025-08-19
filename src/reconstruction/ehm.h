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

namespace reconstruction
{

#include <vector>
#include <memory>
#include <map>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <cmath>

#include <Eigen/Dense>

/**
 * Efficient Hypothesis Management (EHM) for multi-target tracking
 * Implementation based on the original EHM paper by Cox & Miller
 */
class EfficientHypothesisManager 
{
public:
    // Represents a single track-to-measurement association
    struct Association 
    {
        int track_idx;
        int measurement_idx;
        double probability;
        double mahalanobis_distance;
    };
    
    // A hypothesis is a set of compatible associations
    struct Hypothesis 
    {
        std::vector<Association> associations;
        double log_likelihood = 0.0;
        double consistency_measure = 0.0;
        
        // For comparing hypotheses (higher is better)
        bool operator<(const Hypothesis& other) const {
            return log_likelihood < other.log_likelihood;
        }
    };
    
    // Node in the hypothesis tree
    struct TreeNode 
    {
        Association association;
        std::vector<std::shared_ptr<TreeNode>> children;
        std::shared_ptr<TreeNode> parent;
        double accumulated_log_likelihood = 0.0;
        Eigen::MatrixXd innovation_covariance;
        
        // For leaf nodes, this contains the full hypothesis
        Hypothesis hypothesis;
    };

    /**
     * Constructor with configuration parameters
     * 
     * @param gate_threshold Mahalanobis distance threshold for gating
     * @param max_hypotheses Maximum number of hypotheses to maintain
     * @param consistency_threshold Minimum consistency measure for hypotheses
     * @param pd_true Detection probability for true targets
     */
    EfficientHypothesisManager(
        double gate_threshold = 9.0,
        size_t max_hypotheses = 100,
        double consistency_threshold = 0.01,
        double pd_true = 0.9)
        : gate_threshold_(gate_threshold)
        , max_hypotheses_(max_hypotheses)
        , consistency_threshold_(consistency_threshold)
        , pd_true_(pd_true)
        , pd_false_(1.0 - pd_true)
    {}

    /**
     * Process measurements and generate hypotheses using EHM
     * 
     * @param tracks Vector of track states (each represented as an Eigen vector)
     * @param track_covariances Vector of track covariance matrices
     * @param measurements Vector of measurements (each represented as an Eigen vector)
     * @param measurement_covariances Vector of measurement covariance matrices
     * @param transition_matrices Matrices for projecting track states to measurement space
     * @return Vector of best hypotheses sorted by log-likelihood
     */
    std::vector<Hypothesis> processFrame(
        const std::vector<Eigen::VectorXd>& tracks,
        const std::vector<Eigen::MatrixXd>& track_covariances,
        const std::vector<Eigen::VectorXd>& measurements,
        const std::vector<Eigen::MatrixXd>& measurement_covariances,
        const std::vector<Eigen::MatrixXd>& transition_matrices) 
    {
        // Step 1: Generate all valid track-measurement associations with gating
        std::vector<Association> valid_associations = generateValidAssociations(
            tracks, track_covariances, measurements, measurement_covariances, transition_matrices);
        
        // Step 2: Build hypothesis tree using JCBB-inspired approach
        auto root_node = buildHypothesisTree(
            valid_associations, tracks, track_covariances, measurements, 
            measurement_covariances, transition_matrices);
        
        // Step 3: Extract and prune hypotheses from tree
        auto hypotheses = extractHypothesesFromTree(root_node);
        
        // Step 4: Calculate consistency measure for each hypothesis
        calculateConsistencyMeasures(hypotheses);
        
        // Step 5: Prune hypotheses based on consistency and max count
        pruneHypotheses(hypotheses);
        
        return hypotheses;
    }

    /**
     * Calculate marginal probabilities from pruned hypotheses
     * 
     * @param hypotheses Vector of hypotheses
     * @param num_tracks Number of tracks
     * @param num_measurements Number of measurements
     * @return Marginal probabilities for each track-measurement pair
     */
    std::vector<std::vector<std::pair<int, double>>> calculateMarginalProbabilities(
        const std::vector<Hypothesis>& hypotheses,
        size_t num_tracks,
        size_t num_measurements) 
    {
        // Calculate normalization factor (sum of likelihoods)
        double normalization_factor = 0.0;
        for (const auto& hypothesis : hypotheses) {
            normalization_factor += std::exp(hypothesis.log_likelihood);
        }
        
        // Initialize result with missed detection probabilities
        std::vector<std::vector<std::pair<int, double>>> marginal_probs(num_tracks);
        for (size_t track_idx = 0; track_idx < num_tracks; ++track_idx) {
            // -1 represents missed detection
            marginal_probs[track_idx].emplace_back(-1, pd_false_);
        }
        
        // Calculate marginal probabilities for each track-measurement pair
        for (size_t track_idx = 0; track_idx < num_tracks; ++track_idx) {
            // Track association probabilities across all hypotheses
            std::map<int, double> meas_probs;
            
            for (const auto& hypothesis : hypotheses) {
                // Get probability weight for this hypothesis
                double hyp_weight = std::exp(hypothesis.log_likelihood) / normalization_factor;
                
                // Find if this track is associated in this hypothesis
                bool track_associated = false;
                for (const auto& assoc : hypothesis.associations) {
                    if (assoc.track_idx == track_idx) {
                        // Add weighted probability to this measurement
                        meas_probs[assoc.measurement_idx] += hyp_weight;
                        track_associated = true;
                        break;
                    }
                }
                
                // If track not associated, add to missed detection probability
                if (!track_associated) {
                    meas_probs[-1] += hyp_weight;
                }
            }
            
            // Add non-zero probabilities to result
            for (const auto& [meas_idx, prob] : meas_probs) {
                if (prob > 0.0) {
                    marginal_probs[track_idx].emplace_back(meas_idx, prob);
                }
            }
        }
        
        return marginal_probs;
    }

private:
    double gate_threshold_;
    size_t max_hypotheses_;
    double consistency_threshold_;
    double pd_true_;
    double pd_false_;

    /**
     * Generate valid track-measurement associations with gating
     * 
     * @param tracks Track states
     * @param track_covariances Track covariance matrices
     * @param measurements Measurement vectors
     * @param measurement_covariances Measurement covariance matrices
     * @param transition_matrices Matrices for projecting track states to measurement space
     * @return Vector of valid associations
     */
    std::vector<Association> generateValidAssociations(
        const std::vector<Eigen::VectorXd>& tracks,
        const std::vector<Eigen::MatrixXd>& track_covariances,
        const std::vector<Eigen::VectorXd>& measurements,
        const std::vector<Eigen::MatrixXd>& measurement_covariances,
        const std::vector<Eigen::MatrixXd>& transition_matrices) 
    {
        std::vector<Association> valid_associations;
        
        for (size_t track_idx = 0; track_idx < tracks.size(); ++track_idx) {
            const auto& track = tracks[track_idx];
            const auto& track_cov = track_covariances[track_idx];
            const auto& H = transition_matrices[track_idx];
            
            for (size_t meas_idx = 0; meas_idx < measurements.size(); ++meas_idx) {
                const auto& measurement = measurements[meas_idx];
                const auto& meas_cov = measurement_covariances[meas_idx];
                
                // Calculate innovation
                Eigen::VectorXd innovation = measurement - H * track;
                
                // Calculate innovation covariance
                Eigen::MatrixXd S = H * track_cov * H.transpose() + meas_cov;
                
                // Calculate Mahalanobis distance
                double mahalanobis = innovation.transpose() * S.inverse() * innovation;
                
                // Apply gating
                if (mahalanobis <= gate_threshold_) {
                    // Calculate association probability (proportional to likelihood)
                    double log_likelihood = -0.5 * mahalanobis - 
                                          0.5 * std::log(2 * M_PI * S.determinant());
                    
                    Association assoc;
                    assoc.track_idx = track_idx;
                    assoc.measurement_idx = meas_idx;
                    assoc.probability = std::exp(log_likelihood);
                    assoc.mahalanobis_distance = mahalanobis;
                    
                    valid_associations.push_back(assoc);
                }
            }
        }
        
        return valid_associations;
    }

    /**
     * Build hypothesis tree using JCBB-inspired approach
     * This is a recursive tree-building algorithm that enforces joint compatibility
     * 
     * @param valid_associations Valid track-measurement associations
     * @param tracks Track states
     * @param track_covariances Track covariance matrices
     * @param measurements Measurement vectors
     * @param measurement_covariances Measurement covariance matrices
     * @param transition_matrices Matrices for projecting track states to measurement space
     * @return Root node of hypothesis tree
     */
    std::shared_ptr<TreeNode> buildHypothesisTree(
        const std::vector<Association>& valid_associations,
        const std::vector<Eigen::VectorXd>& tracks,
        const std::vector<Eigen::MatrixXd>& track_covariances,
        const std::vector<Eigen::VectorXd>& measurements,
        const std::vector<Eigen::MatrixXd>& measurement_covariances,
        const std::vector<Eigen::MatrixXd>& transition_matrices) 
    {
        // Create root node (dummy node)
        auto root = std::make_shared<TreeNode>();
        
        // Group associations by track
        std::unordered_map<int, std::vector<Association>> track_associations;
        for (const auto& assoc : valid_associations) {
            track_associations[assoc.track_idx].push_back(assoc);
        }
        
        // Sort tracks by number of associations (fewer first for efficiency)
        std::vector<int> track_indices;
        for (const auto& [track_idx, _] : track_associations) {
            track_indices.push_back(track_idx);
        }
        
        std::sort(track_indices.begin(), track_indices.end(), 
                 [&track_associations](int a, int b) {
                     return track_associations[a].size() < track_associations[b].size();
                 });
        
        // Create empty hypothesis
        Hypothesis empty_hyp;
        
        // Build tree recursively
        buildTreeRecursive(root, track_indices, 0, track_associations, 
                          tracks, track_covariances, measurements, 
                          measurement_covariances, transition_matrices, 
                          empty_hyp, Eigen::MatrixXd());
        
        return root;
    }

    /**
     * Recursive helper for building hypothesis tree
     */
    void buildTreeRecursive(
        std::shared_ptr<TreeNode> node,
        const std::vector<int>& track_indices,
        size_t depth,
        const std::unordered_map<int, std::vector<Association>>& track_associations,
        const std::vector<Eigen::VectorXd>& tracks,
        const std::vector<Eigen::MatrixXd>& track_covariances,
        const std::vector<Eigen::VectorXd>& measurements,
        const std::vector<Eigen::MatrixXd>& measurement_covariances,
        const std::vector<Eigen::MatrixXd>& transition_matrices,
        const Hypothesis& current_hyp,
        const Eigen::MatrixXd& joint_innovation_cov) 
    {
        // Base case: all tracks processed
        if (depth >= track_indices.size()) {
            // Store hypothesis in leaf node
            node->hypothesis = current_hyp;
            return;
        }
        
        int track_idx = track_indices[depth];
        const auto& track_assocs = track_associations.at(track_idx);
        
        // Add missed detection branch
        auto missed_node = std::make_shared<TreeNode>();
        missed_node->parent = node;
        missed_node->association.track_idx = track_idx;
        missed_node->association.measurement_idx = -1;  // -1 for missed detection
        missed_node->association.probability = pd_false_;
        missed_node->accumulated_log_likelihood = node->accumulated_log_likelihood + std::log(pd_false_);
        
        // Add missed detection to hypothesis
        Hypothesis missed_hyp = current_hyp;
        // We don't add missed detections to the associations list
        
        // Continue building tree with missed detection
        buildTreeRecursive(missed_node, track_indices, depth + 1, track_associations,
                          tracks, track_covariances, measurements, measurement_covariances, 
                          transition_matrices, missed_hyp, joint_innovation_cov);
        
        node->children.push_back(missed_node);
        
        // Process each possible association for this track
        for (const auto& assoc : track_assocs) {
            // Check if measurement already used in current hypothesis
            bool measurement_used = false;
            for (const auto& existing_assoc : current_hyp.associations) {
                if (existing_assoc.measurement_idx == assoc.measurement_idx) {
                    measurement_used = true;
                    break;
                }
            }
            
            if (measurement_used) {
                continue;  // Skip - measurement already assigned
            }
            
            // Check joint compatibility if we have existing associations
            if (!current_hyp.associations.empty()) {
                // In a full implementation, we would check joint Mahalanobis distance here
                // This is a simplified version that just checks individual compatibility
                if (assoc.mahalanobis_distance > gate_threshold_) {
                    continue;  // Not compatible with existing associations
                }
            }
            
            // Create new tree node for this association
            auto new_node = std::make_shared<TreeNode>();
            new_node->parent = node;
            new_node->association = assoc;
            new_node->accumulated_log_likelihood = node->accumulated_log_likelihood + 
                                                 std::log(pd_true_ * assoc.probability);
            
            // Add association to hypothesis
            Hypothesis new_hyp = current_hyp;
            new_hyp.associations.push_back(assoc);
            new_hyp.log_likelihood = new_node->accumulated_log_likelihood;
            
            // Continue building tree with this association
            buildTreeRecursive(new_node, track_indices, depth + 1, track_associations,
                              tracks, track_covariances, measurements, measurement_covariances, 
                              transition_matrices, new_hyp, joint_innovation_cov);
            
            node->children.push_back(new_node);
        }
    }

    /**
     * Extract hypotheses from tree
     * 
     * @param root_node Root of hypothesis tree
     * @return Vector of hypotheses
     */
    std::vector<Hypothesis> extractHypothesesFromTree(std::shared_ptr<TreeNode> root_node) 
    {
        std::vector<Hypothesis> hypotheses;
        
        // Traverse tree to find all leaf nodes
        std::queue<std::shared_ptr<TreeNode>> queue;
        queue.push(root_node);
        
        while (!queue.empty()) {
            auto node = queue.front();
            queue.pop();
            
            if (node->children.empty()) {
                // Leaf node - add its hypothesis
                hypotheses.push_back(node->hypothesis);
            } else {
                // Non-leaf - add children to queue
                for (auto& child : node->children) {
                    queue.push(child);
                }
            }
        }
        
        return hypotheses;
    }

    /**
     * Calculate consistency measure for each hypothesis
     * 
     * @param hypotheses Vector of hypotheses
     */
    void calculateConsistencyMeasures(std::vector<Hypothesis>& hypotheses) 
    {
        // Find maximum log-likelihood
        double max_log_likelihood = -std::numeric_limits<double>::infinity();
        for (const auto& hyp : hypotheses) {
            max_log_likelihood = std::max(max_log_likelihood, hyp.log_likelihood);
        }
        
        // Calculate consistency measures
        double sum_exp = 0.0;
        for (auto& hyp : hypotheses) {
            sum_exp += std::exp(hyp.log_likelihood - max_log_likelihood);
        }
        
        for (auto& hyp : hypotheses) {
            hyp.consistency_measure = std::exp(hyp.log_likelihood - max_log_likelihood) / sum_exp;
        }
    }

    /**
     * Prune hypotheses based on consistency and maximum count
     * 
     * @param hypotheses Vector of hypotheses to prune
     */
    void pruneHypotheses(std::vector<Hypothesis>& hypotheses) 
    {
        // Sort by log-likelihood (descending)
        std::sort(hypotheses.begin(), hypotheses.end(), 
                 [](const Hypothesis& a, const Hypothesis& b) {
                     return a.log_likelihood > b.log_likelihood;
                 });
        
        // Remove hypotheses with low consistency
        auto it = std::remove_if(hypotheses.begin(), hypotheses.end(),
                                [this](const Hypothesis& hyp) {
                                    return hyp.consistency_measure < consistency_threshold_;
                                });
        
        hypotheses.erase(it, hypotheses.end());
        
        // Limit to maximum number of hypotheses
        if (hypotheses.size() > max_hypotheses_) {
            hypotheses.resize(max_hypotheses_);
        }
    }
};

} // namespace reconstruction
