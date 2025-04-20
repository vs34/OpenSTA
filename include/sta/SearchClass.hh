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

#include <limits>

#include "Vector.hh"
#include "Set.hh"
#include "Map.hh"
#include "UnorderedMap.hh"
#include "StringSet.hh"
#include "MinMaxValues.hh"
#include "Delay.hh"
#include "NetworkClass.hh"
#include "GraphClass.hh"

namespace sta {

class Search;
class Corner;
class Path;
class PathEnd;
class PathGroup;
class Tag;
class TagIndexLess;
class TagMatchLess;
class TagLess;
class TagHash;
class TagEqual;
class TagGroup;
class TagGroupHash;
class TagGroupEqual;
class ClkInfo;
class ClkInfoHash;
class ClkInfoEqual;
class VertexPathIterator;
class PathAnalysisPt;
class PathAnalysisPtIterator;
class MinPulseWidthCheck;
class MinPeriodCheck;
class MaxSkewCheck;
class CharPtrLess;
class SearchPred;
class BfsFwdIterator;
class ClkDelays;

// Tag compare using tag matching (tagMatch) critera.
class TagMatchLess
{
public:
  explicit TagMatchLess(bool match_crpr_clk_pin,
			const StaState *sta);
  bool operator()(const Tag *tag1,
		  const Tag *tag2) const;

protected:
  bool match_crpr_clk_pin_;
  const StaState *sta_;
};

class TagMatchHash
{
public:
  TagMatchHash(bool match_crpr_clk_pin,
	       const StaState *sta);
  size_t operator()(const Tag *tag) const;

protected:
  bool match_crpr_clk_pin_;
  const StaState *sta_;
};

class TagMatchEqual
{
public:
  TagMatchEqual(bool match_crpr_clk_pin,
		const StaState *sta);
  bool operator()(const Tag *tag1, 
		  const Tag *tag2) const;

protected:
  bool match_crpr_clk_pin_;
  const StaState *sta_;
};

typedef int PathAPIndex;
typedef uint32_t TagIndex;
typedef Vector<Tag*> TagSeq;
typedef Vector<MinPulseWidthCheck*> MinPulseWidthCheckSeq;
typedef Vector<MinPeriodCheck*> MinPeriodCheckSeq;
typedef Vector<MaxSkewCheck*> MaxSkewCheckSeq;
typedef StringSet PathGroupNameSet;
typedef Vector<PathEnd*> PathEndSeq;
typedef Vector<Arrival> ArrivalSeq;
typedef Map<Vertex*, size_t> VertexPathCountMap;
typedef UnorderedMap<Tag*, size_t, TagMatchHash, TagMatchEqual> PathIndexMap;
typedef Vector<Slack> SlackSeq;
typedef Delay Crpr;
typedef Vector<Path*> PathSeq;
typedef std::vector<const Path*> ConstPathSeq;

enum class ReportPathFormat { full,
			      full_clock,
			      full_clock_expanded,
			      shorter,
			      endpoint,
			      summary,
			      slack_only,
			      json
};

static const TagIndex tag_index_bit_count = 28;
static const TagIndex tag_index_max = (1 << tag_index_bit_count) - 1;
static const TagIndex tag_index_null = tag_index_max;
static const int path_ap_index_bit_count = 8;
// One path analysis point per corner min/max.
static const int corner_count_max = (1 << path_ap_index_bit_count) / 2;

} // namespace
