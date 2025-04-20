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

#include "VerilogWriter.hh"

#include <cstdlib>
#include <algorithm>

#include "Error.hh"
#include "Liberty.hh"
#include "PortDirection.hh"
#include "Network.hh"
#include "NetworkCmp.hh"
#include "VerilogNamespace.hh"
#include "ParseBus.hh"

namespace sta {

using std::min;
using std::max;
using std::string;

class VerilogWriter
{
public:
  VerilogWriter(const char *filename,
		bool sort,
		bool include_pwr_gnd,
		CellSeq *remove_cells,
		FILE *stream,
		Network *network);
  void writeModules();

protected:
  void writeModule(const Instance *inst);
  InstanceSeq findHierChildren();
  void findHierChildren(const Instance *inst,
                        InstanceSeq &children,
                        CellSet &cells);
  void writePorts(const Cell *cell);
  void writePortDcls(const Cell *cell);
  void writeWireDcls(const Instance *inst);
  const char *verilogPortDir(PortDirection *dir);
  void writeChildren(const Instance *inst);
  void writeChild(const Instance *child);
  void writeInstPin(const Instance *inst,
		    const Port *port,
		    bool &first_port);
  void writeInstBusPin(const Instance *inst,
		       const Port *port,
		       bool &first_port);
  void writeInstBusPinBit(const Instance *inst,
			  const Port *port,
			  bool &first_member);
  void writeAssigns(const Instance *inst);

  int findUnconnectedNetCount(const Instance *inst);
  int findChildNCcount(const Instance *child);
  int findPortNCcount(const Instance *inst,
                      const Port *port);

  const char *filename_;
  bool sort_;
  bool include_pwr_gnd_;
  CellSet remove_cells_;
  FILE *stream_;
  Network *network_;
  int unconnected_net_index_;
};

void
writeVerilog(const char *filename,
	     bool sort,
	     bool include_pwr_gnd,
	     CellSeq *remove_cells,
	     Network *network)
{
  if (network->topInstance()) {
    FILE *stream = fopen(filename, "w");
    if (stream) {
      VerilogWriter writer(filename, sort, include_pwr_gnd,
			   remove_cells, stream, network);
      writer.writeModules();
      fclose(stream);
    }
    else
      throw FileNotWritable(filename);
  }
}

VerilogWriter::VerilogWriter(const char *filename,
			     bool sort,
			     bool include_pwr_gnd,
			     CellSeq *remove_cells,
			     FILE *stream,
			     Network *network) :
  filename_(filename),
  sort_(sort),
  include_pwr_gnd_(include_pwr_gnd),
  remove_cells_(network),
  stream_(stream),
  network_(network),
  unconnected_net_index_(1)
{
  if (remove_cells) {
    for(Cell *lib_cell : *remove_cells)
      remove_cells_.insert(lib_cell);
  }
}

void
VerilogWriter::writeModules()
{
  // Write the top level modeule first.
  writeModule(network_->topInstance());
  InstanceSeq hier_childrenn = findHierChildren();
  for (const Instance *child : hier_childrenn)
    writeModule(child);
}

InstanceSeq
VerilogWriter::findHierChildren()
{
  InstanceSeq children;
  CellSet cells(network_);
  findHierChildren(network_->topInstance(), children, cells);

  if (sort_)
    sort(children, [this](const Instance *inst1,
                          const Instance *inst2) {
      const char *cell_name1 = network_->cellName(inst1);
      const char *cell_name2 = network_->cellName(inst2);
      return stringLess(cell_name1, cell_name2);
    });

  return children;
}

void
VerilogWriter::findHierChildren(const Instance *inst,
                                InstanceSeq &children,
                                CellSet &cells)
{
  InstanceChildIterator *child_iter = network_->childIterator(inst);
  while (child_iter->hasNext()) {
    const Instance *child = child_iter->next();
    const Cell *cell = network_->cell(child);
    if (network_->isHierarchical(child)
        && !cells.hasKey(cell)) {
      children.push_back(child);
      cells.insert(cell);
      findHierChildren(child, children, cells);
    }
  }
  delete child_iter;
}

void
VerilogWriter::writeModule(const Instance *inst)
{
  Cell *cell = network_->cell(inst);
  std::string cell_vname = cellVerilogName(network_->name(cell));
  fprintf(stream_, "module %s (", cell_vname.c_str());
  writePorts(cell);
  writePortDcls(cell);
  fprintf(stream_, "\n");
  writeWireDcls(inst);
  fprintf(stream_, "\n");
  writeChildren(inst);
  writeAssigns(inst);
  fprintf(stream_, "endmodule\n");
}

void
VerilogWriter::writePorts(const Cell *cell)
{
  bool first = true;
  CellPortIterator *port_iter = network_->portIterator(cell);
  while (port_iter->hasNext()) {
    Port *port = port_iter->next();
    if (include_pwr_gnd_
        || !network_->direction(port)->isPowerGround()) {
      if (!first)
        fprintf(stream_, ",\n    ");
      std::string verilog_name = portVerilogName(network_->name(port));
      fprintf(stream_, "%s", verilog_name.c_str());
      first = false;
    }
  }
  delete port_iter;
  fprintf(stream_, ");\n");
}

void
VerilogWriter::writePortDcls(const Cell *cell)
{
  CellPortIterator *port_iter = network_->portIterator(cell);
  while (port_iter->hasNext()) {
    Port *port = port_iter->next();
    PortDirection *dir = network_->direction(port);
    if (include_pwr_gnd_
        || !network_->direction(port)->isPowerGround()) {
      std::string port_vname = portVerilogName(network_->name(port));
      const char *vtype = verilogPortDir(dir);
      if (vtype) {
        fprintf(stream_, " %s", vtype);
        if (network_->isBus(port))
          fprintf(stream_, " [%d:%d]",
                  network_->fromIndex(port),
                  network_->toIndex(port));
        fprintf(stream_, " %s;\n", port_vname.c_str());
        if (dir->isTristate()) {
          fprintf(stream_, " tri");
          if (network_->isBus(port))
            fprintf(stream_, " [%d:%d]",
                    network_->fromIndex(port),
                    network_->toIndex(port));
          fprintf(stream_, " %s;\n", port_vname.c_str());
        }
      }
    }
  }
  delete port_iter;
}

const char *
VerilogWriter::verilogPortDir(PortDirection *dir)
{
  if (dir == PortDirection::input())
    return "input";
  else if (dir == PortDirection::output())
    return "output";
  else if (dir == PortDirection::tristate())
    return "output";
  else if (dir == PortDirection::bidirect())
    return "inout";
  else if (dir == PortDirection::power())
    return "inout";
  else if (dir == PortDirection::ground())
    return "inout";
  else if (dir == PortDirection::internal()
           || dir == PortDirection::unknown())
    return "inout";
  else {
    criticalError(268, "unknown port direction");
    return nullptr;
  }
}

typedef std::pair<int, int> BusIndexRange;

void
VerilogWriter::writeWireDcls(const Instance *inst)
{
  Cell *cell = network_->cell(inst);
  char escape = network_->pathEscape();
  Map<std::string, BusIndexRange, std::less<std::string>> bus_ranges;
  NetIterator *net_iter = network_->netIterator(inst);
  while (net_iter->hasNext()) {
    Net *net = net_iter->next();
    if (include_pwr_gnd_
        || !(network_->isPower(net) || network_->isGround(net))) {
      const char *net_name = network_->name(net);
      if (network_->findPort(cell, net_name) == nullptr) {
        if (isBusName(net_name, '[', ']', escape)) {
          bool is_bus;
          std::string bus_name;
          int index;
          parseBusName(net_name, '[', ']', escape, is_bus, bus_name, index);
          BusIndexRange &range = bus_ranges[bus_name];
          range.first = max(range.first, index);
          range.second = min(range.second, index);
        }
        else {
          std::string net_vname = netVerilogName(net_name);
          fprintf(stream_, " wire %s;\n", net_vname.c_str());;
        }
      }
    }
  }
  delete net_iter;

  for (const auto& [bus_name1, range] : bus_ranges) {
    const char *bus_name = bus_name1.c_str();
    std::string net_vname = netVerilogName(bus_name);
    fprintf(stream_, " wire [%d:%d] %s;\n",
            range.first,
            range.second,
            net_vname.c_str());;
  }

  // Wire net dcls for writeInstBusPinBit.
  int nc_count = findUnconnectedNetCount(inst);
  for (int i = 1; i < nc_count + 1; i++)
    fprintf(stream_, " wire _NC%d;\n", i);
}

void
VerilogWriter::writeChildren(const Instance *inst)
{
  Vector<Instance*> children;
  InstanceChildIterator *child_iter = network_->childIterator(inst);
  while (child_iter->hasNext()) {
    Instance *child = child_iter->next();
    children.push_back(child);
  }
  delete child_iter;

  if (sort_)
    sort(children, [this](const Instance *inst1,
                          const Instance *inst2) {
      return stringLess(network_->name(inst1), network_->name(inst2));
    });

  for (auto child : children)
    writeChild(child);
}

void
VerilogWriter::writeChild(const Instance *child)
{
  Cell *child_cell = network_->cell(child);
  if (!remove_cells_.hasKey(child_cell)) {
    const char *child_name = network_->name(child);
    string child_vname = instanceVerilogName(child_name);
    string child_cell_vname = cellVerilogName(network_->name(child_cell));
    fprintf(stream_, " %s %s (",
	    child_cell_vname.c_str(),
	    child_vname.c_str());
    bool first_port = true;
    CellPortIterator *port_iter = network_->portIterator(child_cell);
    while (port_iter->hasNext()) {
      Port *port = port_iter->next();
      if (include_pwr_gnd_
	  || !network_->direction(port)->isPowerGround()) {
	if (network_->hasMembers(port))
	  writeInstBusPin(child, port, first_port);
	else
	  writeInstPin(child, port, first_port);
      }
    }
    delete port_iter;
    fprintf(stream_, ");\n");
  }
}

void
VerilogWriter::writeInstPin(const Instance *inst,
			    const Port *port,
			    bool &first_port)
{
  Pin *pin = network_->findPin(inst, port);
  if (pin) {
    Net *net = network_->net(pin);
    if (net) {
      const char *net_name = network_->name(net);
      string net_vname = netVerilogName(net_name);
      if (!first_port)
	fprintf(stream_, ",\n    ");
      string port_vname = portVerilogName(network_->name(port));
      fprintf(stream_, ".%s(%s)",
	      port_vname.c_str(),
	      net_vname.c_str());
      first_port = false;
    }
  }
}

void
VerilogWriter::writeInstBusPin(const Instance *inst,
			       const Port *port,
			       bool &first_port)
{
  if (!first_port)
    fprintf(stream_, ",\n    ");

  fprintf(stream_, ".%s({", network_->name(port));
  first_port = false;
  bool first_member = true;

  // Match the member order of the liberty cell if it exists.
  LibertyPort *lib_port = network_->libertyPort(port);
  if (lib_port) {
    Cell *cell = network_->cell(inst);
    LibertyPortMemberIterator member_iter(lib_port);
    while (member_iter.hasNext()) {
      LibertyPort *lib_member = member_iter.next();
      Port *member = network_->findPort(cell, lib_member->name());
      writeInstBusPinBit(inst, member, first_member);
    }
  }
  else {
    PortMemberIterator *member_iter = network_->memberIterator(port);
    while (member_iter->hasNext()) {
      Port *member = member_iter->next();
      writeInstBusPinBit(inst, member, first_member);
    }
    delete member_iter;
  }
  fprintf(stream_, "})");
}

void
VerilogWriter::writeInstBusPinBit(const Instance *inst,
				  const Port *port,
				  bool &first_member)
{
  Pin *pin = network_->findPin(inst, port);
  Net *net = pin ? network_->net(pin) : nullptr;
  string net_name;
  if (net)
    net_name = network_->name(net);
  else
    // There is no verilog syntax to "skip" a bit in the concatentation.
    stringPrint(net_name, "_NC%d", unconnected_net_index_++);
  string net_vname = netVerilogName(net_name.c_str());
  if (!first_member)
    fprintf(stream_, ",\n    ");
  fprintf(stream_, "%s", net_vname.c_str());
  first_member = false;
}

// Verilog "ports" are not distinct from nets.
// Use an assign statement to alias the net when it is connected to
// multiple output ports.
void
VerilogWriter::writeAssigns(const Instance *inst)
{
  InstancePinIterator *pin_iter = network_->pinIterator(inst);
  while (pin_iter->hasNext()) {
    Pin *pin = pin_iter->next();
    Term *term = network_->term(pin);
    if (term) {
      Net *net = network_->net(term);
      Port *port = network_->port(pin);
      if (port
          && (include_pwr_gnd_
              || !(network_->isPower(net) || network_->isGround(net)))
          && (network_->direction(port)->isAnyOutput()
              || (include_pwr_gnd_ && network_->direction(port)->isPowerGround()))
          && !stringEqual(network_->name(port), network_->name(net))) {
        // Port name is different from net name.
        string port_vname = netVerilogName(network_->name(port));
        string net_vname = netVerilogName(network_->name(net));
        fprintf(stream_, " assign %s = %s;\n",
                port_vname.c_str(),
                net_vname.c_str());
      }
    }
  }
  delete pin_iter;
}

////////////////////////////////////////////////////////////////

int
VerilogWriter::findUnconnectedNetCount(const Instance *inst)
{
  int nc_count = 0;
  InstanceChildIterator *child_iter = network_->childIterator(inst);
  while (child_iter->hasNext()) {
    Instance *child = child_iter->next();
    nc_count += findChildNCcount(child);
  }
  delete child_iter;
  return nc_count;
}

int
VerilogWriter::findChildNCcount(const Instance *child)
{
  int nc_count = 0;
  Cell *child_cell = network_->cell(child);
  if (!remove_cells_.hasKey(child_cell)) {
    CellPortIterator *port_iter = network_->portIterator(child_cell);
    while (port_iter->hasNext()) {
      Port *port = port_iter->next();
      if (network_->hasMembers(port))
        nc_count += findPortNCcount(child, port);
    }
    delete port_iter;
  }
  return nc_count;
}

int
VerilogWriter::findPortNCcount(const Instance *inst,
                               const Port *port)
{
  int nc_count = 0;
  PortMemberIterator *member_iter = network_->memberIterator(port);
  while (member_iter->hasNext()) {
    Port *member = member_iter->next();
    Pin *pin = network_->findPin(inst, member);
    if (pin == nullptr
        || network_->net(pin) == nullptr)
      nc_count++;
  }
  delete member_iter;
  return nc_count;
}

} // namespace
