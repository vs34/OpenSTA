// OpenSTA, Static Timing Analyzer
// Copyright (c) 2025, Parallax Software, Inc.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
// 
// The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software.
// 
// Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 
// This notice may not be removed or altered from any source distribution.

#pragma once

#include <string>

namespace sta {

// Return true if name is a bus.
bool
isBusName(const char *name,
	  const char brkt_left,
	  const char brkt_right,
	  char escape);

// Parse name as a bus.
// signal
//  bus_name = nullptr
// bus[bit]
//  bus_name = "bus"
//  index = bit
// Caller must delete returned bus_name string.
void
parseBusName(const char *name,
	     const char brkt_left,
	     const char brkt_right,
	     char escape,
	     // Return values.
	     bool &is_bus,
             std::string &bus_name,
	     int &index);
// Allow multiple different left/right bus brackets.
void
parseBusName(const char *name,
	     const char *brkts_left,
	     const char *brkts_right,
	     char escape,
	     // Return values.
	     bool &is_bus,
	     std::string &bus_name,
	     int &index);

// Parse a bus range, such as BUS[4:0].
// bus_name is set to null if name is not a range.
// Caller must delete returned bus_name string.
void
parseBusName(const char *name,
             const char brkt_left,
             const char brkt_right,
             char escape,
             // Return values.
             bool &is_bus,
             bool &is_range,
             std::string &bus_name,
             int &from,
             int &to,
             bool &subscript_wild);

// brkt_lefts and brkt_rights are corresponding strings of legal
// bus brackets such as "[(<" and "])>".
void
parseBusName(const char *name,
             const char *brkts_left,
             const char *brkts_right,
             const char escape,
             // Return values.
             bool &is_bus,
             bool &is_range,
             std::string &bus_name,
             int &from,
             int &to,
             bool &subscript_wild);

// Insert escapes before ch1 and ch2 in token.
std::string
escapeChars(const char *token,
	    const char ch1,
	    const char ch2,
	    const char escape);

} // namespace
