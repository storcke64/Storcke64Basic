/***************************************************************************

  c_websettings.cpp

  (c) Benoît Minisini <g4mba5@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#define __C_WEBSETTINGS_CPP

#include <errno.h>
#include <unistd.h>

#include "c_webview.h"
#include "c_websettings.h"

//static QNetworkDiskCache *_cache = 0;
/*static bool _cache_enabled = false;
static char *_cache_path = 0;

static void set_cache(bool on)
{
	QNetworkDiskCache *cache;
	
	if (!_cache_path)
		return;
	
	_cache_enabled = on;
	
	if (on)
	{
		cache = new QNetworkDiskCache(0);
		cache->setCacheDirectory(TO_QSTRING(_cache_path));
		WEBVIEW_get_network_manager()->setCache(cache);
	}
	else
		WEBVIEW_get_network_manager()->setCache(0);
}*/


static QWebSettings *get_settings(void *_object)
{
	if (GB.Is(_object, GB.FindClass("WebSettings")))
		return QWebSettings::globalSettings();
	else
		return WEBVIEW->settings();
}

//-------------------------------------------------------------------------

static void handle_font_family(QWebSettings::FontFamily font, void *_object, void *_param)
{
	if (READ_PROPERTY)
		RETURN_NEW_STRING(get_settings(_object)->fontFamily(font));
	else
		get_settings(_object)->setFontFamily(font, QSTRING_PROP());
}

BEGIN_PROPERTY(WebSettingsFonts_Default)

	handle_font_family(QWebSettings::StandardFont, _object, _param);

END_METHOD

BEGIN_PROPERTY(WebSettingsFonts_Fixed)

	handle_font_family(QWebSettings::FixedFont, _object, _param);

END_METHOD

BEGIN_PROPERTY(WebSettingsFonts_Serif)

	handle_font_family(QWebSettings::SerifFont, _object, _param);

END_METHOD

BEGIN_PROPERTY(WebSettingsFonts_SansSerif)

	handle_font_family(QWebSettings::SansSerifFont, _object, _param);

END_METHOD

BEGIN_PROPERTY(WebSettingsFonts_Cursive)

	handle_font_family(QWebSettings::CursiveFont, _object, _param);

END_METHOD

BEGIN_PROPERTY(WebSettingsFonts_Fantasy)

	handle_font_family(QWebSettings::FantasyFont, _object, _param);

END_METHOD

BEGIN_PROPERTY(WebSettingsFonts_Pictograph)

	handle_font_family(QWebSettings::FantasyFont, _object, _param);

END_METHOD

static void handle_font_size(QWebSettings::FontSize size, void *_object, void *_param)
{
	if (READ_PROPERTY)
		GB.ReturnInteger(get_settings(_object)->fontSize(size) * 76 / 92);
	else
		get_settings(_object)->setFontSize(size, VPROP(GB_INTEGER) * 92 / 76);
}

BEGIN_PROPERTY(WebSettingsFonts_DefaultSize)

	handle_font_size(QWebSettings::DefaultFontSize, _object, _param);

END_PROPERTY

BEGIN_PROPERTY(WebSettingsFonts_DefaultFixedSize)

	handle_font_size(QWebSettings::DefaultFixedFontSize, _object, _param);

END_PROPERTY

BEGIN_PROPERTY(WebSettingsFonts_MinimumSize)

	handle_font_size(QWebSettings::MinimumFontSize, _object, _param);

END_PROPERTY

/*BEGIN_PROPERTY(WebSettingsFonts_MinimumLogicalFontSize)

	handle_font_size(QWebSettings::MinimumLogicalFontSize, _object, _param);

END_PROPERTY*/

#if 0
/***************************************************************************/

BEGIN_METHOD_VOID(WebSettingsIconDatabase_Clear)

	QWebSettings::clearIconDatabase();

END_METHOD

BEGIN_PROPERTY(WebSettingsIconDatabase_Path)

	if (READ_PROPERTY)
		RETURN_NEW_STRING(QWebSettings::iconDatabasePath());
	else
		QWebSettings::setIconDatabasePath(QSTRING_PROP());

END_PROPERTY

BEGIN_METHOD(WebSettingsIconDatabase_get, GB_STRING url)

	QIcon icon;
	QSize size;
	QSize max_size;
	
	icon = QWebSettings::iconForUrl(QSTRING_ARG(url));
	if (icon.isNull())
	{
		GB.ReturnNull();
		return;
	}
	
	foreach(size, icon.availableSizes())
	{
		if ((size.width() * size.height()) > (max_size.width() * max_size.height()))
			max_size = size;
	}
	
	GB.ReturnObject(QT.CreatePicture(icon.pixmap(max_size)));

END_METHOD


/***************************************************************************/

BEGIN_PROPERTY(WebSettingsCache_Path)

	if (READ_PROPERTY)
		GB.ReturnString(_cache_path);
	else
	{
		char *path = GB.FileName(PSTRING(), PLENGTH());
		QString qpath = QString(path);
		QString root = QString(GB.System.Home());

		if (root.at(root.length() - 1) != '/')
			root += '/';
		root += ".cache/";
		if (!qpath.startsWith(root))
		{
			GB.Error("Cache directory must be located inside ~/.cache");
			return;
		}

		GB.FreeString(&_cache_path);
		_cache_path = GB.NewZeroString(path);
		set_cache(_cache_enabled);
	}

END_PROPERTY

BEGIN_PROPERTY(WebSettingsCache_Enabled)

	if (READ_PROPERTY)
		GB.ReturnBoolean(_cache_enabled);
	else
		set_cache(VPROP(GB_BOOLEAN));

END_PROPERTY

static int _clear_error;
static char *_clear_path;

static void remove_file(const char *path)
{
	if (rmdir(path) == 0)
		return;

	if (errno == ENOTDIR)
	{
		if (unlink(path) == 0)
			return;
	}

	if (_clear_error == 0)
	{
		_clear_error = errno;
		_clear_path = GB.NewZeroString(path);
	}
}

BEGIN_METHOD_VOID(WebSettingsCache_Clear)

	if (!_cache_path || !*_cache_path)
		return;

	_clear_error = 0;
	GB.BrowseDirectory(_cache_path, NULL, remove_file);

	if (_clear_error)
	{
		GB.Error("Unable to remove '&1': &2", _clear_path, strerror(_clear_error));
		GB.FreeString(&_clear_path);
	}

	set_cache(_cache_enabled);

END_METHOD
#endif

//-------------------------------------------------------------------------

BEGIN_METHOD(WebSettings_get, GB_INTEGER flag)

	QWebSettings *settings = get_settings(_object);
	int flag = VARG(flag);
	
	if (flag >= 0)
		GB.ReturnBoolean(settings->testAttribute((QWebSettings::WebAttribute)flag));
	else
		GB.ReturnBoolean(FALSE);

END_METHOD

BEGIN_METHOD(WebSettings_put, GB_BOOLEAN value; GB_INTEGER flag)

	QWebSettings *settings = get_settings(_object);
	int flag = VARG(flag);

	if (flag >= 0)
		settings->setAttribute((QWebSettings::WebAttribute)flag, VARG(value));

END_METHOD

/*BEGIN_METHOD(WebSettings_Reset, GB_INTEGER flag)

	QWebSettings *settings = get_settings(_object);
	int flag = VARG(flag);

	if (flag >= 0)
		settings->resetAttribute((QWebSettings::WebAttribute)flag);

END_METHOD*/

#if 0
/***************************************************************************/

BEGIN_METHOD_VOID(WebSettings_exit)

	GB.FreeString(&_cache_path);

END_METHOD

/***************************************************************************/

BEGIN_PROPERTY(WebSettingsProxy_Host)

	QNetworkAccessManager *nam = WEBVIEW_get_network_manager();
	QNetworkProxy proxy = nam->proxy();
	
	if (READ_PROPERTY)
		RETURN_NEW_STRING(proxy.hostName());
	else
	{
		proxy.setHostName(QSTRING_PROP());
		nam->setProxy(proxy);
	}

END_PROPERTY

BEGIN_PROPERTY(WebSettingsProxy_User)

	QNetworkAccessManager *nam = WEBVIEW_get_network_manager();
	QNetworkProxy proxy = nam->proxy();
	
	if (READ_PROPERTY)
		RETURN_NEW_STRING(proxy.user());
	else
	{
		proxy.setUser(QSTRING_PROP());
		nam->setProxy(proxy);
	}

END_PROPERTY

BEGIN_PROPERTY(WebSettingsProxy_Password)

	QNetworkAccessManager *nam = WEBVIEW_get_network_manager();
	QNetworkProxy proxy = nam->proxy();
	
	if (READ_PROPERTY)
		RETURN_NEW_STRING(proxy.password());
	else
	{
		proxy.setPassword(QSTRING_PROP());
		nam->setProxy(proxy);
	}

END_PROPERTY

BEGIN_PROPERTY(WebSettingsProxy_Port)

	QNetworkAccessManager *nam = WEBVIEW_get_network_manager();
	QNetworkProxy proxy = nam->proxy();
	
	if (READ_PROPERTY)
		GB.ReturnInteger(proxy.port());
	else
	{
		proxy.setPort(VPROP(GB_INTEGER));
		nam->setProxy(proxy);
	}

END_PROPERTY

BEGIN_PROPERTY(WebSettingsProxy_Type)

	QNetworkAccessManager *nam = WEBVIEW_get_network_manager();
	QNetworkProxy proxy = nam->proxy();
	
	if (READ_PROPERTY)
		GB.ReturnInteger(proxy.type());
	else
	{
		int type = VPROP(GB_INTEGER);
		if (type == QNetworkProxy::DefaultProxy || type == QNetworkProxy::NoProxy || type == QNetworkProxy::Socks5Proxy || type == QNetworkProxy::HttpProxy)
		{
			proxy.setType((QNetworkProxy::ProxyType)type);
			nam->setProxy(proxy);
		}
	}

END_PROPERTY
#endif

/***************************************************************************/

#if 0
GB_DESC WebSettingsIconDatabaseDesc[] =
{
  GB_DECLARE(".WebSettings.IconDatabase", 0), GB_VIRTUAL_CLASS(),
	
	GB_STATIC_METHOD("Clear", NULL, WebSettingsIconDatabase_Clear, NULL),
	GB_STATIC_PROPERTY("Path", "s", WebSettingsIconDatabase_Path),
	GB_STATIC_METHOD("_get", "Picture", WebSettingsIconDatabase_get, "(Url)s"),
	
	GB_END_DECLARE
};

GB_DESC WebSettingsCacheDesc[] =
{
  GB_DECLARE(".WebSettings.Cache", 0), GB_VIRTUAL_CLASS(),
	
	GB_STATIC_PROPERTY("Enabled", "b", WebSettingsCache_Enabled),
	GB_STATIC_PROPERTY("Path", "s", WebSettingsCache_Path),
	GB_STATIC_METHOD("Clear", NULL, WebSettingsCache_Clear, NULL),
	
	GB_END_DECLARE
};

GB_DESC WebSettingsProxyDesc[] =
{
  GB_DECLARE(".WebSettings.Proxy", 0), GB_VIRTUAL_CLASS(),
	
	GB_STATIC_PROPERTY("Type", "i", WebSettingsProxy_Type),
	GB_STATIC_PROPERTY("Host", "s", WebSettingsProxy_Host),
	GB_STATIC_PROPERTY("Port", "i", WebSettingsProxy_Port),
	GB_STATIC_PROPERTY("User", "s", WebSettingsProxy_User),
	GB_STATIC_PROPERTY("Password", "s", WebSettingsProxy_Password),
	
	GB_END_DECLARE
};
#endif

GB_DESC WebSettingsFontsDesc[] =
{
	GB_DECLARE_VIRTUAL(".WebSettings.Fonts"),

	GB_PROPERTY("Default", "s", WebSettingsFonts_Default),
	GB_PROPERTY("Fixed", "s", WebSettingsFonts_Fixed),
	GB_PROPERTY("Serif", "s", WebSettingsFonts_Serif),
	GB_PROPERTY("SansSerif", "s", WebSettingsFonts_SansSerif),
	GB_PROPERTY("Cursive", "s", WebSettingsFonts_Cursive),
	GB_PROPERTY("Fantasy", "s", WebSettingsFonts_Fantasy),
	GB_PROPERTY("Pictograph", "s", WebSettingsFonts_Pictograph),

	GB_PROPERTY("MinimumSize", "i", WebSettingsFonts_MinimumSize),
	GB_PROPERTY("DefaultSize", "i", WebSettingsFonts_DefaultSize),
	GB_PROPERTY("DefaultFixedSize", "i", WebSettingsFonts_DefaultFixedSize),
	
	GB_END_DECLARE
};

GB_DESC WebViewSettingsDesc[] =
{
  GB_DECLARE_VIRTUAL(".WebView.Settings"),
	
	GB_METHOD("_get", "b", WebSettings_get, "(Flag)i"),
	GB_METHOD("_put", NULL, WebSettings_put, "(Value)b(Flag)i"),
	
	GB_PROPERTY_SELF("Fonts", ".WebSettings.Fonts"),

	GB_END_DECLARE
};

GB_DESC WebSettingsDesc[] =
{
  GB_DECLARE("WebSettings", sizeof(CWEBSETTINGS)), GB_AUTO_CREATABLE(), GB_NOT_CREATABLE(),
  
	GB_CONSTANT("AutoLoadImages", "i", QWebSettings::AutoLoadImages),
	GB_CONSTANT("Javascript", "i", QWebSettings::JavascriptEnabled),
	GB_CONSTANT("JavascriptCanOpenWindows", "i", QWebSettings::JavascriptCanOpenWindows),
	GB_CONSTANT("JavascriptCanAccessClipboard", "i", QWebSettings::JavascriptCanAccessClipboard),
	GB_CONSTANT("LocalStorage", "i", QWebSettings::LocalStorageEnabled),
	GB_CONSTANT("SpatialNavigation", "i", QWebSettings::SpatialNavigationEnabled),
	GB_CONSTANT("LocalContentCanAccessFileUrls", "i", QWebSettings::LocalContentCanAccessFileUrls),
	GB_CONSTANT("HyperlinkAuditing", "i", QWebSettings::HyperlinkAuditingEnabled),
	GB_CONSTANT("Plugins", "i", QWebSettings::PluginsEnabled),
	GB_CONSTANT("FullScreenSupport", "i", -1),
	GB_CONSTANT("WebGL", "i", -1),
	GB_CONSTANT("Accelerated2dCanvas", "i", -1),
	GB_CONSTANT("PrintElementBackgrounds", "i", -1),
	GB_CONSTANT("PlaybackRequiresUserGesture", "i", -1),
	GB_CONSTANT("DnsPrefetch", "i", QWebSettings::DnsPrefetchEnabled),

	/*GB_CONSTANT("DefaultProxy", "i", QNetworkProxy::DefaultProxy),
	GB_CONSTANT("NoProxy", "i", QNetworkProxy::NoProxy),
	GB_CONSTANT("Socks5Proxy", "i", QNetworkProxy::Socks5Proxy),
	GB_CONSTANT("HttpProxy", "i", QNetworkProxy::HttpProxy),*/
	
	GB_PROPERTY_SELF("Fonts", ".WebSettings.Fonts"),
	/*GB_STATIC_PROPERTY_SELF("IconDatabase", ".WebSettings.IconDatabase"),
	GB_STATIC_PROPERTY_SELF("Cache", ".WebSettings.Cache"),
	GB_STATIC_PROPERTY_SELF("Proxy", ".WebSettings.Proxy"),*/
	
	GB_METHOD("_get", "b", WebSettings_get, "(Flag)i"),
	GB_METHOD("_put", NULL, WebSettings_put, "(Value)b(Flag)i"),
	
	//GB_STATIC_METHOD("_exit", NULL, WebSettings_exit, NULL),

GB_END_DECLARE
};


