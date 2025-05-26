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

#include "Zlib.hh"
#include "StringSeq.hh"
#include "NetworkClass.hh"
#include "ParasiticsClass.hh"
#include "StaState.hh"

namespace sta {

class Report;
class MinMaxAll;
class SpefRspfPi;
class SpefTriple;
class Corner;
class SpefScanner;

typedef std::map<int, std::string> SpefNameMap;

class SpefReader : public StaState
{
public:
  SpefReader(const char *filename,
	     Instance *instance,
	     ParasiticAnalysisPt *ap,
	     bool pin_cap_included,
	     bool keep_coupling_caps,
	     float coupling_cap_factor,
	     bool reduce,
	     const Corner *corner,
	     const MinMaxAll *min_max,
             StaState *sta);
  virtual ~SpefReader();
  bool read();
  char divider() const { return divider_; }
  void setDivider(char divider);
  char delimiter() const { return delimiter_; }
  void setDelimiter(char delimiter);
  const char *filename() const { return filename_; }
  // Translate from spf/spef namespace to sta namespace.
  char *translated(const char *token);
  void warn(int id,
            const char *fmt,
            ...)
    __attribute__((format (printf, 3, 4)));
  void setBusBrackets(char left,
		      char right);
  void setTimeScale(float scale,
		    const char *units);
  void setCapScale(float scale,
		   const char *units);
  void setResScale(float scale,
		   const char *units);
  void setInductScale(float scale,
		      const char *units);
  void makeNameMapEntry(const char *index,
			const char *name);
  const char *nameMapLookup(const char *index);
  void setDesignFlow(StringSeq *flow_keys);
  Pin *findPin(char *name);
  Net *findNet(const char *name);
  void rspfBegin(Net *net,
		 SpefTriple *total_cap);
  void rspfFinish();
  void rspfDrvrBegin(Pin *drvr_pin,
		     SpefRspfPi *pi);
  void rspfLoad(Pin *load_pin,
		SpefTriple *rc);
  void rspfDrvrFinish();
  void dspfBegin(Net *net,
		 SpefTriple *total_cap);
  void dspfFinish();
  void makeCapacitor(int id,
		     char *node_name,
		     SpefTriple *cap);
  void makeCapacitor(int id,
		     char *node_name1,
		     char *node_name2,
		     SpefTriple *cap);
  void makeResistor(int id,
		    char *node_name1,
		    char *node_name2,
		    SpefTriple *res);
  PortDirection *portDirection(char *spef_dir);

private:
  Pin *findPinRelative(const char *name);
  Pin *findPortPinRelative(const char *name);
  Net *findNetRelative(const char *name);
  Instance *findInstanceRelative(const char *name);
  ParasiticNode *findParasiticNode(char *name,
                                   bool local_only);

  const char *filename_;
  SpefScanner *scanner_;
  Instance *instance_;
  const ParasiticAnalysisPt *ap_;
  bool pin_cap_included_;
  bool keep_coupling_caps_;
  bool reduce_;
  const Corner *corner_;
  const MinMaxAll *min_max_;
  // Normally no need to keep device names.
  char divider_;
  char delimiter_;
  char bus_brkt_left_;
  char bus_brkt_right_;
  Net *net_;

  int triple_index_;
  float time_scale_;
  float cap_scale_;
  float res_scale_;
  float induct_scale_;
  SpefNameMap name_map_;
  StringSeq *design_flow_;
  Parasitic *parasitic_;
};

class SpefTriple
{
public:
  SpefTriple(float value);
  SpefTriple(float value1,
	     float value2,
	     float value3);
  float value(int index) const;
  bool isTriple() const { return is_triple_; }

private:
  float values_[3];
  bool is_triple_;
};

class SpefRspfPi
{
public:
  SpefRspfPi(SpefTriple *c2, SpefTriple *r1, SpefTriple *c1);
  ~SpefRspfPi();
  SpefTriple *c2() { return c2_; }
  SpefTriple *r1() { return r1_; }
  SpefTriple *c1() { return c1_; }

private:
  SpefTriple *c2_;
  SpefTriple *r1_;
  SpefTriple *c1_;
};

extern SpefReader *spef_reader;

} // namespace
