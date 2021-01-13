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

#ifndef EVALUATIONTARGETPOSITION_H
#define EVALUATIONTARGETPOSITION_H

class EvaluationTargetPosition
{
public:
    EvaluationTargetPosition() {}
    EvaluationTargetPosition(double latitude, double longitude, bool has_altitude, bool altitude_calculated,
                             float altitude)
        : latitude_(latitude), longitude_(longitude), has_altitude_(has_altitude),
          altitude_calculated_(altitude_calculated), altitude_(altitude)
    {}

    double latitude_ {0};
    double longitude_ {0};
    bool has_altitude_ {false};
    bool altitude_calculated_ {false}; // indicates if secondary or derived
    float altitude_ {0};
};

#endif // EVALUATIONTARGETPOSITION_H
