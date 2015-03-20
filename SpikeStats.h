/*
 Copyright (C) 2011 Georgia Institute of Technology

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <default_gui_model.h>
#include "include/runningstat.h"

class SpikeStats : public DefaultGUIModel
{

Q_OBJECT

public:

  SpikeStats(void);
  virtual
  ~SpikeStats(void);

  // the main function runs every time step, contains model logic
  virtual void
  execute(void);
  void
  createGUI(DefaultGUIModel::variable_t *, int);

protected:

  // run each time model parameters are updated
  virtual void
  update(DefaultGUIModel::update_flags_t);

private:

  // parameters
  double thresh;
  double min_int;

  // time of spikes
  double spktime;
  double prevspktime;
  RunningStat runningPeriod;
  double ISI;
  double ISImean;
  double ISIstd;
  double CV;
  double spikecount;

  // the internal state variable, sent as output
  int state;
  double systime;
  double dt; // real-time

  void
  initParameters();
  void
  countspikes();

private slots:

void reset();
};
