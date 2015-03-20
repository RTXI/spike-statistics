
PLUGIN_NAME = SpikeStats

HEADERS = SpikeStats.h\
    include/runningstat.h

SOURCES = SpikeStats.cpp\
    moc_SpikeStats.cpp\
    include/runningstat.cpp
		
LIBS = 

### Do not edit below this line ###

include $(shell rtxi_plugin_config --pkgdata-dir)/Makefile.plugin_compile
