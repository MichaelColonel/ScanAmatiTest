libscanner_a_SOURCES = defines.hpp \
	acquisition.hpp \
	acquisition.cpp \
	adc_count.hpp \
	adc_count.cpp \
	assemble.hpp \
	assemble.cpp \
	builtin_chip_capacities.hpp \
	builtin_chip_capacities.cpp \
	commands.hpp \
	commands.cpp \
	data.hpp \
	data.cpp \
	image_calibration.hpp \
	image_calibration.cpp \
	image_data.hpp \
	image_data.cpp \
	manager_device.cpp \
	manager.hpp \
	manager.cpp \
	manager_state.hpp \
	manager_state.cpp \
	movement.hpp \
	movement.cpp \
	run_arguments.hpp \
	run_arguments.cpp \
	state.hpp \
	state.cpp \
	temperature_regulator.hpp \
	temperature_regulator.cpp \
	x-ray.hpp \
	x-ray.cpp

AM_CXXFLAGS = $(GLIBMM_CFLAGS) $(MAGICK_CFLAGS) \
	-I$(top_srcdir)/src

noinst_LIBRARIES = libscanner.a
