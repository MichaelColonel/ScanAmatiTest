noinst_LIBRARIES = libdicom.a

libdicom_a_SOURCES = \
	conquest.hpp \
	conquest.cpp \
	typedefs.hpp \
	tag_string_map.hpp \
	information.hpp \
	patient_age.hpp \
	patient_age.cpp \
	patient_information.hpp \
	study_information.hpp \
	summary_information.hpp \
	summary_information.cpp \
	short_information.hpp \
	short_information.cpp \
	server.hpp \
	server.cpp \
	user_commands.hpp \
	user_commands.cpp \
	utils.hpp \
	utils.cpp \
	xmedcon_wrapper.h \
	xmedcon_wrapper.c

AM_CXXFLAGS = $(XMEDCON_CFLAGS) $(GLIBMM_CFLAGS) $(DCMTK_CFLAGS) \
	-I$(top_srcdir)/src
