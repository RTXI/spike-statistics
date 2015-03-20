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

/*
 * This module watches the input and detects a positive threshold crossing.
 * It computes the average ISI and the running CV.
 */

#include <SpikeStats.h>

#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qscrollview.h>
#include <qtooltip.h>
#include <qvalidator.h>
#include <qwhatsthis.h>

extern "C" Plugin::Object *
createRTXIPlugin(void)
{
  return new SpikeStats();
}

// inputs, outputs, parameters
static DefaultGUIModel::variable_t
    vars[] =
      {
            { "Vm", "Membrane Voltage (in mV)", DefaultGUIModel::INPUT, },
            { "ISI", "ISI (ms)", DefaultGUIModel::OUTPUT, },
            { "Threshold (mV)", "Threshold (mV) at which to detect a spike",
                DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
            {
                "Min Interval (ms)",
                "Minimum interval (refractory period) that must pass before another spike is detected",
                DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
            { "Average ISI (ms)", "Average ISI (ms)", DefaultGUIModel::STATE, },
            { "CV", "Coefficient of Variation", DefaultGUIModel::STATE, },
            { "# Spikes", "# Spikes", DefaultGUIModel::STATE, },
            { "Time (s)", "Time (s)", DefaultGUIModel::STATE, }, };

// some necessary variable
static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);

// constructor 
// provides default values for paramters, calls update(INIT)
SpikeStats::SpikeStats(void) :
  DefaultGUIModel("Spike Statistics", ::vars, ::num_vars), thresh(-0.02),
      min_int(5e-3), spktime(0), prevspktime(0), state(0)
{
  QWhatsThis::add(
      this,
      "<p><b>Spike Statistics:</b><br>This module watches the voltage and detects the onset and offsets of spikes "
        "using a positive threshold crossings. It tracks the running average ISI and CV, which can be reset.</p>");
  initParameters();
  createGUI(vars, num_vars);
  update( INIT);
  refresh();
}

SpikeStats::~SpikeStats(void)
{
}

// execute is run every time step
void
SpikeStats::execute(void)
{
  systime = RT::OS::getTime();

  switch (state)
    {
  case 0:
    if (input(0) > thresh) // spike happened
      {
        state = 1;
      }
    break;
  case 1:
    state = 2;
    break;
  case 2:
    if (input(0) > thresh && (systime - spktime) > 100)
      {
        state = 4;
      }
    else if (input(0) < thresh)
      {
        state = 3;
      }
    break;
  case 3:
    state = -1;
    break;
  case 4:
    if (input(0) < thresh)
      {
        state = -1;
      }
    break;
  case -1:
    if (systime - spktime > min_int)
      {
        state = 0;
      }
    break;
  default:
    break;
    }

  if (state == 1)
    countspikes();
  if (spikecount > 2)
    { // ignore first period
      runningPeriod.push(ISI);
      ISImean = runningPeriod.mean();
      ISIstd = runningPeriod.std();
      CV = runningPeriod.std() / runningPeriod.mean();
      printf("%f %f\n", ISI, ISImean);
    }

  output(0) = ISI; // send state information as output
}

void
SpikeStats::countspikes()
{
  prevspktime = spktime;
  spktime = systime;
  ISI = (spktime - prevspktime) * 1e-6;
  spikecount++;
}

void
SpikeStats::update(DefaultGUIModel::update_flags_t flag)
{
  switch (flag)
    {
  case INIT:
    setParameter("Threshold (mV)", QString::number(thresh * 1000.0)); // in V, display in mV
    setParameter("Min Interval (ms)", QString::number(min_int * 1000)); // in s, display in ms
    setState("Average ISI (ms)", ISImean);
    setState("CV", CV);
    setState("# Spikes", spikecount);
    break;
  case MODIFY:
    thresh = getParameter("Threshold (mV)").toDouble() / 1000.; // displayed in mV, convert to V
    min_int = getParameter("Min Interval (ms)").toDouble() / 1000; // displayed in ms, convert to s
    break;
  case PAUSE:
    break;
  case UNPAUSE:
    reset();
    break;
  case PERIOD:
    dt = RT::System::getInstance()->getPeriod() * 1e-9; // s
  default:
    break;
    }
}

void
SpikeStats::reset()
{
  runningPeriod.clear();
  CV = 0;
  systime = 0;
  state = 0;
  ISI = 0;
  ISImean = 0;
  ISIstd = 0;
  CV = 0;
  spikecount = 0;
  spktime = 0;
  prevspktime = 0;
}

void
SpikeStats::initParameters()
{
  dt = RT::System::getInstance()->getPeriod() * 1e-9; // s
  reset();
}

//create the GUI components
void
SpikeStats::createGUI(DefaultGUIModel::variable_t *var, int size)
{
  QBoxLayout *layout = new QHBoxLayout(this); // overall GUI layout

  // Left side GUI
  QBoxLayout *leftlayout = new QVBoxLayout();
  QPushButton *resetButton = new QPushButton("Reset Statistics", this);
  leftlayout->addWidget(resetButton);

  QObject::connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));

  // Add custom left side GUI components to layout above default_gui_model components
  //    leftlayout->addLayout(optionLayout);

  // Create default_gui_model GUI DO NOT EDIT
  QScrollView *sv = new QScrollView(this);
  sv->setResizePolicy(QScrollView::AutoOneFit);
  sv->setHScrollBarMode(QScrollView::AlwaysOff);
  leftlayout->addWidget(sv);

  QWidget *viewport = new QWidget(sv->viewport());
  sv->addChild(viewport);
  QGridLayout *scrollLayout = new QGridLayout(viewport, 1, 2);
  //loop through to create GUI buttons/text boxes
  size_t nstate = 0, nparam = 0, nevent = 0, ncomment = 0;
  for (size_t i = 0; i < num_vars; i++)
    {
      if (vars[i].flags & (PARAMETER | STATE | EVENT | COMMENT))
        {
          param_t param;

          param.label = new QLabel(vars[i].name, viewport);
          scrollLayout->addWidget(param.label, parameter.size(), 0);
          param.edit = new DefaultGUILineEdit(viewport);
          scrollLayout->addWidget(param.edit, parameter.size(), 1);

          QToolTip::add(param.label, vars[i].description);
          QToolTip::add(param.edit, vars[i].description);

          if (vars[i].flags & PARAMETER)
            {
              if (vars[i].flags & DOUBLE)
                {
                  param.edit->setValidator(new QDoubleValidator(param.edit));
                  param.type = PARAMETER | DOUBLE;
                }
              else if (vars[i].flags & UINTEGER)
                {
                  QIntValidator *validator = new QIntValidator(param.edit);
                  param.edit->setValidator(validator);
                  validator->setBottom(0);
                  param.type = PARAMETER | UINTEGER;
                }
              else if (vars[i].flags & INTEGER)
                {
                  param.edit->setValidator(new QIntValidator(param.edit));
                  param.type = PARAMETER | INTEGER;
                }
              else
                param.type = PARAMETER;
              param.index = nparam++;
              param.str_value = new QString;
            }
          else if (vars[i].flags & STATE)
            {
              param.edit->setReadOnly(true);
              param.type = STATE;
              param.index = nstate++;
            }
          else if (vars[i].flags & EVENT)
            {
              param.edit->setReadOnly(true);
              param.type = EVENT;
              param.index = nevent++;
            }
          else if (vars[i].flags & COMMENT)
            {
              param.type = COMMENT;
              param.index = ncomment++;
            }

          parameter[vars[i].name] = param;
        }
    }

  QHBox *utilityBox = new QHBox(this);
  pauseButton = new QPushButton("Pause", utilityBox);
  pauseButton->setToggleButton(true);
  QObject::connect(pauseButton, SIGNAL(toggled(bool)), this, SLOT(pause(bool)));
  QPushButton *modifyButton = new QPushButton("Modify", utilityBox);
  QObject::connect(modifyButton, SIGNAL(clicked(void)), this, SLOT(modify(void)));
  QPushButton *unloadButton = new QPushButton("Unload", utilityBox);
  QObject::connect(unloadButton, SIGNAL(clicked(void)), this, SLOT(exit(void)));
  QObject::connect(pauseButton, SIGNAL(toggled(bool)), modifyButton, SLOT(setEnabled(bool)));
  QToolTip::add(pauseButton, "Start/Stop");
  QToolTip::add(modifyButton, "Commit changes to parameter values");
  QToolTip::add(unloadButton, "Close plug-in");

  // add custom components to layout below default_gui_model components
  leftlayout->addWidget(utilityBox);
  // Add left and right side layouts to the overall layout
  layout->addLayout(leftlayout);
  //    layout->setResizeMode(QLayout::Fixed);

  show();

}
