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

#include "MinMax.hh"
#include "Transition.hh"
#include "NetworkClass.hh"
#include "GraphClass.hh"
#include "Delay.hh"
#include "SdcClass.hh"

namespace sta {

class StaState;
class DcalcAnalysisPt;
class Corner;

class CheckSlewLimits
{
public:
  CheckSlewLimits(const StaState *sta);
  // Return pins with the min/max slew limit slack.
  // net=null check all nets
  // corner=nullptr checks all corners.
  PinSeq checkSlewLimits(const Net *net,
                         bool violators,
                         const Corner *corner,
                         const MinMax *min_max);
  // corner=nullptr checks all corners.
  void checkSlew(const Pin *pin,
		 const Corner *corner,
		 const MinMax *min_max,
		 bool check_clks,
		 // Return values.
		 // Corner is nullptr for no slew limit.
		 const Corner *&corner1,
		 const RiseFall *&rf,
		 Slew &slew,
		 float &limit,
		 float &slack) const;
  void findLimit(const LibertyPort *port,
                 const Corner *corner,
                 const MinMax *min_max,
                 // Return values.
                 float &limit,
                 bool &exists) const;

protected:
  void checkSlew1(const Pin *pin,
                  const Vertex *vertex,
                  const Corner *corner,
                  const MinMax *min_max,
                  bool check_clks,
                  // Return values.
                  const Corner *&corner1,
                  const RiseFall *&rf1,
                  Slew &slew1,
                  float &limit1,
                  float &slack1) const;
  void checkSlew2(const Pin *pin,
                  const Vertex *vertex,
                  const Corner *corner,
                  const MinMax *min_max,
                  const ClockSet &clks,
                  // Return values.
                  const Corner *&corner1,
                  const RiseFall *&rf1,
                  Slew &slew1,
                  float &limit1,
                  float &slack1) const;
  void checkSlew3(const Vertex *vertex,
                  const Corner *corner1,
                  const RiseFall *rf1,
                  const MinMax *min_max,
                  float limit1,
                  // Return values.
                  const Corner *&corner,
                  const RiseFall *&rf,
                  Slew &slew,
                  float &slack,
                  float &limit) const;
  void findLimit(const Pin *pin,
                 const Corner *corner,
		 const RiseFall *rf,
		 const MinMax *min_max,
                 const ClockSet &clks,
		 // Return values.
		 float &limit,
		 bool &limit_exists) const;
  void checkSlewLimits(const Instance *inst,
                       bool violators,
                       const Corner *corner,
                       const MinMax *min_max,
                       PinSeq &slew_pins,
                       float &min_slack);
  void checkSlewLimits(const Pin *pin,
                       bool violators,
                       const Corner *corner,
                       const MinMax *min_max,
                       PinSeq &slew_pins,
                       float &min_slack);
  ClockSet clockDomains(const Vertex *vertex) const;

  const StaState *sta_;
};

} // namespace
