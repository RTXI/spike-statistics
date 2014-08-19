PLUGIN_NAME = spike_statistics

HEADERS = spike-statistics.h

SOURCES = spike-statistics.cpp \
          moc_spike-statistics.cpp
		
LIBS = 

### Do not edit below this line ###

include $(shell rtxi_plugin_config --pkgdata-dir)/Makefile.plugin_compile
