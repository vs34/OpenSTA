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

#include "PathAnalysisPt.hh"

#include "StringUtil.hh"
#include "Corner.hh"
#include "Search.hh"

namespace sta {

PathAnalysisPt::PathAnalysisPt(Corner *corner,
			       PathAPIndex index,
			       const MinMax *path_min_max,
			       DcalcAnalysisPt *dcalc_ap) :
  corner_(corner),
  index_(index),
  path_min_max_(path_min_max),
  tgt_clk_ap_(nullptr),
  dcalc_ap_(dcalc_ap)
{
}

std::string
PathAnalysisPt::to_string() const
{
  std::string name = corner_->name();
  name += '/';
  name += path_min_max_->to_string();
  return name;
}

void
PathAnalysisPt::setTgtClkAnalysisPt(PathAnalysisPt *path_ap)
{
  tgt_clk_ap_ = path_ap;
}

PathAnalysisPt *
PathAnalysisPt::insertionAnalysisPt(const EarlyLate *early_late) const
{
  return insertion_aps_[early_late->index()];
}

void
PathAnalysisPt::setInsertionAnalysisPt(const EarlyLate *early_late,
				       PathAnalysisPt *ap)
{
  insertion_aps_[early_late->index()] = ap;
}

} // namespace
