COMPONENT = gb.gtk
include $(top_srcdir)/component.am

SUBDIRS = . @GTKOPENGL_DIR@

gblib_LTLIBRARIES = gb.gtk.la

gb_gtk_la_LIBADD = @THREAD_LIB@ @GTK_LIB@
gb_gtk_la_LDFLAGS = -module @LD_FLAGS@ @GTK_LDFLAGS@
gb_gtk_la_CPPFLAGS = @THREAD_INC@ @GTK_INC@ -I$(top_srcdir)/share

gb_gtk_la_SOURCES = \
	gtag.h ggambastag.h gshare.h \
	gb.gtk.h widgets.h font-parser.h font-parser.cpp \
	gtools.cpp gcolor.h \
	gfont.cpp gpicture.cpp \
	gdesktop.cpp gdialog.cpp \
	gcontrol.cpp gcontainer.cpp \
	gbutton.cpp gslider.cpp gsignals.cpp \
	gtextbox.cpp gtextarea.cpp \
	gpanel.cpp gtabstrip.cpp \
	gmenu.cpp gtrayicon.cpp gmainwindow.cpp \
	gtree.h gtree.cpp \
	watcher.h watcher.cpp \
	CConst.h CConst.cpp CColor.h CColor.cpp \
	CFont.h CFont.cpp \
	CKey.h CKey.cpp \
	CScreen.h CScreen.cpp \
	CStyle.h CStyle.cpp \
	CDialog.h CDialog.cpp \
	CDraw.h CDraw.cpp \
	cpaint_impl.h cpaint_impl.cpp \
	CImage.h CImage.cpp \
	CPicture.h CPicture.cpp \
	canimation.h canimation.cpp \
	CClipboard.h CClipboard.cpp \
	CMouse.h CMouse.cpp \
	CWatcher.h CWatcher.cpp \
	CWidget.h CWidget.cpp CContainer.h  CContainer.cpp \
	CDrawingArea.h CDrawingArea.cpp \
	CSlider.h CSlider.cpp \
	CButton.h CButton.cpp \
	CTextBox.h CTextBox.cpp \
	CTextArea.h CTextArea.cpp \
	CTabStrip.h CTabStrip.cpp \
	CPanel.h CPanel.cpp \
	CMenu.h CMenu.cpp CTrayIcon.h CTrayIcon.cpp CWindow.h CWindow.cpp \
	cprinter.h cprinter.cpp \
	csvgimage.h csvgimage.cpp \
	main.h main.cpp \
	gkey.h gkey.cpp \
	gcursor.h gcursor.cpp \
	gmouse.h gmouse.cpp \
	gdesktop.h \
	gpicture.h \
	gfont.h \
	gdialog.h \
	gcontrol.h \
	gseparator.h \
	gtrayicon.h \
	gplugin.h \
	gbutton.h \
	gtextbox.h \
	gtextarea.h \
	gslider.h \
	gscrollbar.h \
	gcontainer.h \
	gdrawingarea.h gdrawingarea.cpp \
	gpanel.h gtabstrip.h \
	gmenu.h \
	gmainwindow.h \
	gapplication.h gapplication.cpp \
	gclipboard.h \
	gdrag.h gdrag.cpp \
	gtools.h kentities.h \
	gprinter.h gprinter.cpp \
	gglarea.h gglarea.cpp \
	x11.h x11.c \
	sm/bonobo-macros.h \
	sm/gnome-macros.h \
	sm/gnome-uidefs.h \
	sm/libgnomeui.h \
	sm/libgnomeuiP.h \
	sm/gnome-client.h \
	sm/gnome-client.c \
	sm/gnome-ice.h \
	sm/gnome-ice.c \
	sm/gnome-marshal.h \
	sm/gnome-marshal.c \
	sm/gnometypebuiltins.h \
	sm/gnometypebuiltins.c \
	sm/sm.h
