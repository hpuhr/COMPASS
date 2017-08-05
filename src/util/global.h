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

#ifndef DBGLOBAL_H_
#define DBGLOBAL_H_

#include <qsize.h>

static QSize UI_ICON_SIZE (20,20);
static const unsigned int UI_ICON_BUTTON_MAX_WIDTH=40;
static const bool UI_ICON_BUTTON_FLAT=true;
static const unsigned int FRAME_SIZE=1;
static const std::string META_OBJECT_NAME="Meta";

/// If an old ogre library (1.6.x) is used.
//#define USE_OLD_OGRE 0
/// If set, disables ogre logging.
//#define DISABLE_OGRE_LOG 1

/// Definition of different presentation modes
//enum PRESENTATION_MODE
//{
//  PRESENTATION_MODE_POINT_DETECTION,
//  PRESENTATION_MODE_POINT_RADAR,
//  PRESENTATION_MODE_SYMBOL_DETECTION,
//  PRESENTATION_MODE_SYMBOL_RADAR
//};

/// Definition of different view modes
//enum VIEW_MODE
//{
//  VIEW_MODE_2D, VIEW_MODE_3D
//};

/// Definition if different display object types
//enum DOType {
//  DO_UNKNOWN,
//  DO_POINTS,
//  DO_TILEDPOINTS,
//  DO_LINES
//};

/// Definition of different display object query flags
//enum DOQueryFlag {
//  QUERY_NONE        = 0,
//  QUERY_POINTS      = 1<<1,
//  QUERY_TILEDPOINTS = 1<<2,
//  QUERY_LINES       = 1<<3,
//  QUERY_RECTS       = 1<<4
//};

#endif
