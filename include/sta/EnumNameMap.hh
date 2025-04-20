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

#include <map>
#include <string>

namespace sta {

// Helper for mapping enum values to strings and back.
template <class ENUM>
class EnumNameMap
{
public:
  EnumNameMap(std::initializer_list<std::pair<const ENUM, std::string>> enum_names);
  const char *find(ENUM key) const;
  ENUM find(std::string name,
	    ENUM unknown_key) const;
  void find(std::string name,
	    // Return values.
	    ENUM &key,
	    bool &exists) const;
  
private:
  std::map<ENUM, std::string> enum_map_;
  std::map<std::string, ENUM> name_map_;
};

template <class ENUM>
EnumNameMap<ENUM>::EnumNameMap(std::initializer_list<std::pair<const ENUM,std::string>> enum_names) :
  enum_map_(enum_names)
{
  for (const auto& [key, name] : enum_map_)
    name_map_[name] = key;
}

template <class ENUM>
const char *
EnumNameMap<ENUM>::find(ENUM key) const
{
  auto find_iter = enum_map_.find(key);
  if (find_iter != enum_map_.end())
    return find_iter->second.c_str();
  else
    return nullptr;
}

template <class ENUM>
void
EnumNameMap<ENUM>::find(std::string name,
			// Return values.
			ENUM &key,
			bool &exists) const
{
  auto find_iter = name_map_.find(name);
  if (find_iter != name_map_.end()) {
    key = find_iter->second;
    exists = true;
  }
  else
    exists = false;
}

template <class ENUM>
ENUM
EnumNameMap<ENUM>::find(std::string name,
			ENUM unknown_key) const
{
  auto find_iter = name_map_.find(name);
  if (find_iter != name_map_.end())
    return find_iter->second;
  else
    return unknown_key;
}

} // namespace
