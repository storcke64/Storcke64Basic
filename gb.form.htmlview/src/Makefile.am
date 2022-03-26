COMPONENT = gb.form.htmlview
include $(top_srcdir)/component.am

EXTRA_DIST=gumbo/LICENSE litehtml/README.md litehtml/LICENSE

noinst_LTLIBRARIES = libgumbo.la
gblib_LTLIBRARIES = gb.form.htmlview.la

libgumbo_la_LIBADD =
libgumbo_la_LDFLAGS = -module @LD_FLAGS@
libgumbo_la_CFLAGS = -std=c99 -I$(top_srcdir)/share $(AM_CFLAGS_OPT)

libgumbo_la_SOURCES = \
	gumbo/attribute.c \
	gumbo/attribute.h \
	gumbo/char_ref.c \
	gumbo/char_ref.h \
	gumbo/char_ref.rl \
	gumbo/error.c \
	gumbo/error.h \
	gumbo/gumbo.h \
	gumbo/insertion_mode.h \
	gumbo/parser.c \
	gumbo/parser.h \
	gumbo/string_buffer.c \
	gumbo/string_buffer.h \
	gumbo/string_piece.c \
	gumbo/string_piece.h \
	gumbo/tag.c \
	gumbo/tag_enum.h \
	gumbo/tag_gperf.h \
	gumbo/tag_sizes.h \
	gumbo/tag_strings.h \
	gumbo/tokenizer.c \
	gumbo/tokenizer.h \
	gumbo/tokenizer_states.h \
	gumbo/token_type.h \
	gumbo/utf8.c \
	gumbo/utf8.h \
	gumbo/util.c \
	gumbo/util.h \
	gumbo/vector.c \
	gumbo/vector.h

gb_form_htmlview_la_LIBADD = libgumbo.la
gb_form_htmlview_la_LDFLAGS = -module @LD_FLAGS@
gb_form_htmlview_la_CXXFLAGS = -std=c++11 -I$(top_srcdir)/share -I$(srcdir)/gumbo $(AM_CXX_FLAGS)
gb_form_htmlview_la_CPPFLAGS =

gb_form_htmlview_la_SOURCES = \
	main.cpp main.h \
	c_htmldocument.cpp c_htmldocument.h \
	\
	litehtml/attributes.h \
	litehtml/background.cpp \
	litehtml/background.h \
	litehtml/borders.h \
	litehtml/box.cpp \
	litehtml/box.h \
	litehtml/codepoint.cpp \
	litehtml/codepoint.h \
	litehtml/context.cpp \
	litehtml/context.h \
	litehtml/css_length.cpp \
	litehtml/css_length.h \
	litehtml/css_margins.h \
	litehtml/css_offsets.h \
	litehtml/css_position.h \
	litehtml/css_selector.cpp \
	litehtml/css_selector.h \
	litehtml/document.cpp \
	litehtml/document.h \
	litehtml/el_anchor.cpp \
	litehtml/el_anchor.h \
	litehtml/el_base.cpp \
	litehtml/el_base.h \
	litehtml/el_before_after.cpp \
	litehtml/el_before_after.h \
	litehtml/el_body.cpp \
	litehtml/el_body.h \
	litehtml/el_break.cpp \
	litehtml/el_break.h \
	litehtml/el_cdata.cpp \
	litehtml/el_cdata.h \
	litehtml/el_comment.cpp \
	litehtml/el_comment.h \
	litehtml/el_div.cpp \
	litehtml/el_div.h \
	litehtml/element.cpp \
	litehtml/element.h \
	litehtml/el_font.cpp \
	litehtml/el_font.h \
	litehtml/el_image.cpp \
	litehtml/el_image.h \
	litehtml/el_li.cpp \
	litehtml/el_li.h \
	litehtml/el_link.cpp \
	litehtml/el_link.h \
	litehtml/el_para.cpp \
	litehtml/el_para.h \
	litehtml/el_script.cpp \
	litehtml/el_script.h \
	litehtml/el_space.cpp \
	litehtml/el_space.h \
	litehtml/el_style.cpp \
	litehtml/el_style.h \
	litehtml/el_table.cpp \
	litehtml/el_table.h \
	litehtml/el_td.cpp \
	litehtml/el_td.h \
	litehtml/el_text.cpp \
	litehtml/el_text.h \
	litehtml/el_title.cpp \
	litehtml/el_title.h \
	litehtml/el_tr.cpp \
	litehtml/el_tr.h \
	litehtml/html.cpp \
	litehtml/html.h \
	litehtml/html_tag.cpp \
	litehtml/html_tag.h \
	litehtml/iterators.cpp \
	litehtml/iterators.h \
	litehtml/media_query.cpp \
	litehtml/media_query.h \
	litehtml/num_cvt.cpp \
	litehtml/num_cvt.h \
	litehtml/os_types.h \
	litehtml/strtod.cpp \
	litehtml/src.txt \
	litehtml/style.cpp \
	litehtml/style.h \
	litehtml/stylesheet.cpp \
	litehtml/stylesheet.h \
	litehtml/table.cpp \
	litehtml/table.h \
	litehtml/tstring_view.cpp \
	litehtml/tstring_view.h \
	litehtml/types.h \
	litehtml/url.cpp \
	litehtml/url.h \
	litehtml/url_path.cpp \
	litehtml/url_path.h \
	litehtml/utf8_strings.cpp \
	litehtml/utf8_strings.h \
	litehtml/web_color.cpp \
	litehtml/web_color.h
