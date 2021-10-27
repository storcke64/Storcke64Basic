COMPONENT = gb.form.htmlview
include $(top_srcdir)/component.am

EXTRA_DIST = master.css master.css.h

gblib_LTLIBRARIES = gb.form.htmlview.la

gb_form_htmlview_la_LIBADD = @FORM_HTMLVIEW_LIB@
gb_form_htmlview_la_LDFLAGS = -module @LD_FLAGS@ @FORM_HTMLVIEW_LDFLAGS@
gb_form_htmlview_la_CXXFLAGS = $(AM_CXX_FLAGS)
gb_form_htmlview_la_CPPFLAGS = @FORM_HTMLVIEW_INC@

gb_form_htmlview_la_SOURCES = \
	main.cpp main.h \
	c_htmldocument.cpp c_htmldocument.h
