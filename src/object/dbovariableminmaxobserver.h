/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DBOVARIABLEOBSERVER_H_
#define DBOVARIABLEOBSERVER_H_

class DBOVariable;

/**
 * @brief Interface for observers of the DBOVariable minimum/maximum information
 *
 * Sub-classes need to override notifyMinMax(), which will be called if information changes.
 */
class DBOVariableMinMaxObserver
{
public:
    DBOVariableMinMaxObserver() {}
    virtual ~DBOVariableMinMaxObserver () {}

    virtual void notifyMinMax (DBOVariable *variable)=0;
};


#endif /* DBOVARIABLEOBSERVER_H_ */
