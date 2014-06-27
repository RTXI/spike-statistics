PLUGIN_NAME = spike_stats

HEADERS = spike-stats.h

SOURCES = spike-stats.cpp \
    moc_spike-stats.cpp
		
LIBS = 

### Do not edit below this line ###

include $(shell rtxi_plugin_config --pkgdata-dir)/Makefile.plugin_compile
