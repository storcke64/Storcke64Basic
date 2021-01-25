/***************************************************************************

  c_webview.c

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

#define __C_WEBVIEW_C

#include "c_websettings.h"
#include "c_webview.h"

#define HISTORY (webkit_web_view_get_back_forward_list(WIDGET))

static bool _init = FALSE;

//---------------------------------------------------------------------------

static void set_link(void *_object, const char *link, int len)
{
	GB.FreeString(&THIS->link);
	if (len == 0 || !link)
		return;
	if (len < 0) len = strlen(link);
	THIS->link = GB.NewString(link, len);
}

//---------------------------------------------------------------------------

DECLARE_EVENT(EVENT_TITLE);
DECLARE_EVENT(EVENT_URL);
DECLARE_EVENT(EVENT_ICON);
DECLARE_EVENT(EVENT_START);
DECLARE_EVENT(EVENT_PROGRESS);
DECLARE_EVENT(EVENT_FINISH);
DECLARE_EVENT(EVENT_ERROR);
DECLARE_EVENT(EVENT_LINK);
DECLARE_EVENT(EVENT_NEW_VIEW);

static void cb_title(WebKitWebView *widget, GParamSpec *pspec, CWEBVIEW *_object)
{
	GB.Raise(THIS, EVENT_TITLE, 0);
}

static void cb_url(WebKitWebView *widget, GParamSpec *pspec, CWEBVIEW *_object)
{
	//fprintf(stderr, "cb_url: %s\n", webkit_web_view_get_uri(WIDGET));
	GB.Raise(THIS, EVENT_URL, 0);
	if (!THIS->got_load_event)
		GB.Raise(THIS, EVENT_FINISH, 0);
}

static void cb_icon(WebKitWebView *widget, GParamSpec *pspec, CWEBVIEW *_object)
{
	GB.Unref(POINTER(&THIS->icon));
	THIS->icon = NULL;
	GB.Raise(THIS, EVENT_ICON, 0);
}

static void cb_load_changed(WebKitWebView *widget, WebKitLoadEvent load_event, CWEBVIEW *_object)
{
	//fprintf(stderr, "cb_load_changed: %d\n", load_event);
	
	switch (load_event)
	{
		case WEBKIT_LOAD_STARTED:
			THIS->got_load_event = TRUE;
			break;
			
		case WEBKIT_LOAD_FINISHED:
			if (!THIS->error)
				GB.Raise(THIS, EVENT_FINISH, 0);
			GB.FreeString(&THIS->link);
			break;
			
		default:
			break;
	}
}

static gboolean cb_load_failed(WebKitWebView *widget, WebKitLoadEvent load_event, gchar *failing_uri, GError *error, CWEBVIEW *_object)
{
	THIS->error = TRUE;
	GB.Raise(THIS, EVENT_ERROR, 0);
	return FALSE;
}

static void cb_progress(WebKitWebView *widget, GParamSpec *pspec, CWEBVIEW *_object)
{
	//fprintf(stderr, "cb_progress: %f\n", webkit_web_view_get_estimated_load_progress(WIDGET));
	if (!THIS->error)
	{
		GB.Raise(THIS, EVENT_PROGRESS, 0);
		if (webkit_web_view_get_estimated_load_progress(WIDGET) == 1.0)
			GB.Raise(THIS, EVENT_FINISH, 0);
	}
}

static void cb_link(WebKitWebView *widget, WebKitHitTestResult *hit_test_result, guint modifiers, CWEBVIEW *_object)
{
	const char *uri = webkit_hit_test_result_get_link_uri(hit_test_result);
	set_link(THIS, uri, -1);
	GB.Raise(THIS, EVENT_LINK, 0);
}

static GtkWidget *cb_create(WebKitWebView *widget, WebKitNavigationAction *navigation_action, CWEBVIEW *_object)
{
	GtkWidget *new_view;
	
	if (GB.Raise(THIS, EVENT_NEW_VIEW, 0))
		return NULL;
	
	if (!THIS->new_view)
		return NULL;
	
	new_view = ((CWEBVIEW *)THIS->new_view)->widget;
	GB.Unref(POINTER(&THIS->new_view));
	THIS->new_view = NULL;
	return new_view;
}

static gboolean cb_decide_policy(WebKitWebView *widget, WebKitPolicyDecision *decision, WebKitPolicyDecisionType type, CWEBVIEW *_object)
{
	if (type == WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION)
	{
		/*const char *uri = webkit_uri_request_get_uri(
			webkit_navigation_action_get_request(
				webkit_navigation_policy_decision_get_navigation_action(WEBKIT_NAVIGATION_POLICY_DECISION(decision))));

		fprintf(stderr, "cb_decide_policy: %s\n", uri);*/
		
		if (THIS->accept_next)
		{
			THIS->accept_next = FALSE;
			return FALSE;
		}
		
		THIS->error = FALSE;
		THIS->got_load_event = FALSE;
		if (GB.Raise(THIS, EVENT_START, 0))
			webkit_policy_decision_ignore(decision);
		
		/*if (THIS->cancel)
		{
			fprintf(stderr, "cb_decide_policy: IGNORE !\n");
			THIS->cancel = FALSE;
			webkit_policy_decision_ignore(decision);
		}*/
		
		/*GB.FreeString(&THIS->link);
		THIS->link = GB.NewZeroString(uri);
		cancel = GB.Raise(THIS, EVENT_NAVIGATE, 0);
		if (cancel)
			webkit_policy_decision_ignore(decision);*/
	}
	
	return FALSE;
}

static void cb_javascript_finished(WebKitWebView *widget, GAsyncResult *result, void *_object)
{
	WebKitJavascriptResult *js_result;
	JSCValue *value;
	GError *error = NULL;
	JSCException *exception;
	char *json;

	js_result = webkit_web_view_run_javascript_finish(widget, result, &error);
	if (!js_result)
	{
		THIS->js_result = GB.NewZeroString(error->message);
		g_error_free(error);
		THIS->js_error = TRUE;
	}
	else
	{
		value = webkit_javascript_result_get_js_value(js_result);
		json = jsc_value_to_json(value, 0);
		
		exception = jsc_context_get_exception(jsc_value_get_context (value));
		if (exception)
		{
			THIS->js_result = GB.NewZeroString(jsc_exception_get_message(exception));
			THIS->js_error = TRUE;
		}
		else
		{
			THIS->js_result = GB.NewZeroString(json);
		}
		
		g_free(json);
		webkit_javascript_result_unref(js_result);
	}
	
	GB.Unref(POINTER(&_object));
	THIS->js_running = FALSE;
}

//---------------------------------------------------------------------------

#define must_patch(_widget) (true)
#include "../gb.gtk.patch.h"

PATCH_DECLARE(WEBKIT_TYPE_WEB_VIEW)

static void create_widget(void *_object, void *parent)
{
	THIS->widget = webkit_web_view_new();
	
	GTK.CreateControl(THIS, parent, THIS->widget);
	
	PATCH_CLASS(THIS->widget, WEBKIT_TYPE_WEB_VIEW)
	
	if (!_init)
	{
		//webkit_settings_set_load_icons_ignoring_image_load_setting(WEBVIEW_default_settings, TRUE);
		//webkit_web_context_set_use_system_appearance_for_scrollbars(webkit_web_context_get_default(), TRUE);
		webkit_web_context_set_favicon_database_directory(webkit_web_context_get_default(), NULL);
		_init = TRUE;
	}
	
	g_signal_connect(G_OBJECT(WIDGET), "notify::title", G_CALLBACK(cb_title), (gpointer)THIS);
	g_signal_connect(G_OBJECT(WIDGET), "notify::uri", G_CALLBACK(cb_url), (gpointer)THIS);
	g_signal_connect(G_OBJECT(WIDGET), "notify::favicon", G_CALLBACK(cb_icon), (gpointer)THIS);
	g_signal_connect(G_OBJECT(WIDGET), "notify::estimated-load-progress", G_CALLBACK(cb_progress), (gpointer)THIS);
	g_signal_connect(G_OBJECT(WIDGET), "load-changed", G_CALLBACK(cb_load_changed), (gpointer)THIS);
	g_signal_connect(G_OBJECT(WIDGET), "load-failed", G_CALLBACK(cb_load_failed), (gpointer)THIS);
	g_signal_connect(G_OBJECT(WIDGET), "mouse-target-changed", G_CALLBACK(cb_link), (gpointer)THIS);
	g_signal_connect(G_OBJECT(WIDGET), "create", G_CALLBACK(cb_create), (gpointer)THIS);
	g_signal_connect(G_OBJECT(WIDGET), "decide-policy", G_CALLBACK(cb_decide_policy), (gpointer)THIS);
	
	WEBVIEW_init_settings(THIS);
}

//---------------------------------------------------------------------------

BEGIN_METHOD(WebView_new, GB_OBJECT parent)

	create_widget(THIS, VARG(parent));

END_METHOD

BEGIN_METHOD_VOID(WebView_free)

	GB.FreeString(&THIS->link);

	GB.Unref(POINTER(&THIS->icon));
	GB.Unref(POINTER(&THIS->new_view));

END_METHOD

BEGIN_PROPERTY(WebView_Url)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(webkit_web_view_get_uri(WIDGET));
	else
	{
		set_link(THIS, PSTRING(), PLENGTH());
		webkit_web_view_load_uri(WIDGET, THIS->link);
	}

END_PROPERTY

BEGIN_PROPERTY(WebView_Title)

	GB.ReturnNewZeroString(webkit_web_view_get_title(WIDGET));

END_PROPERTY

BEGIN_METHOD(WebView_SetHtml, GB_STRING html; GB_STRING root)

	THIS->accept_next = TRUE;
	webkit_web_view_load_html(WIDGET, GB.ToZeroString(ARG(html)), MISSING(root) ? NULL : GB.ToZeroString(ARG(root)));

END_METHOD

BEGIN_METHOD_VOID(WebView_Back)

	webkit_web_view_go_back(WIDGET);

END_METHOD

BEGIN_METHOD_VOID(WebView_Forward)

	webkit_web_view_go_forward(WIDGET);

END_METHOD

BEGIN_METHOD(WebView_Reload, GB_BOOLEAN bypass)

	if (VARGOPT(bypass, FALSE))
		webkit_web_view_reload_bypass_cache(WIDGET);
	else
		webkit_web_view_reload(WIDGET);

END_METHOD

BEGIN_METHOD_VOID(WebView_Stop)

	webkit_web_view_stop_loading(WIDGET);

END_METHOD

BEGIN_PROPERTY(WebView_Zoom)

	if (READ_PROPERTY)
		GB.ReturnFloat(webkit_web_view_get_zoom_level(WIDGET));
	else
		webkit_web_view_set_zoom_level(WIDGET, VPROP(GB_FLOAT));

END_PROPERTY

BEGIN_PROPERTY(WebView_Icon)

	cairo_surface_t *icon;
	
	if (!THIS->icon)
	{
		icon = webkit_web_view_get_favicon(WIDGET);
		
		if (icon)
		{
			int size = GTK.GetDesktopScale() * 2;
			cairo_surface_reference(icon);
			THIS->icon = GTK.CreatePicture(icon, size, size);
			GB.Ref(THIS->icon);
		}
	}

	GB.ReturnObject(THIS->icon);

END_PROPERTY

BEGIN_PROPERTY(WebView_Progress)

	GB.ReturnFloat(webkit_web_view_get_estimated_load_progress(WIDGET));

END_PROPERTY

BEGIN_PROPERTY(WebView_NewView)

	if (READ_PROPERTY)
		GB.ReturnObject(THIS->new_view);
	else
		GB.StoreObject(PROP(GB_OBJECT), &THIS->new_view);

END_PROPERTY

BEGIN_PROPERTY(WebView_Link)

	GB.ReturnString(THIS->link);

END_PROPERTY

BEGIN_METHOD_VOID(WebView_Clear)

	create_widget(THIS, NULL);

END_METHOD

BEGIN_METHOD(WebView_ExecJavascript, GB_STRING script)

	char *script;
	
	if (LENGTH(script) == 0)
		return;
	
	script = GB.ToZeroString(ARG(script));
	
	THIS->js_running = TRUE;
	GB.Ref(THIS);
	webkit_web_view_run_javascript(WIDGET, script, NULL, (GAsyncReadyCallback)cb_javascript_finished, (gpointer)THIS);
	
	while(THIS->js_running)
		GB.Wait(0);
	
	if (THIS->js_error)
	{
		GB.Error("Javascript error: &1", THIS->js_result);
		GB.FreeString(&THIS->js_result);
	}
	else
	{
		GB.ReturnString(GB.FreeStringLater(THIS->js_result));
		THIS->js_result = NULL;
	}
	
	THIS->js_error = FALSE;

END_METHOD

//---------------------------------------------------------------------------

BEGIN_PROPERTY(WebViewHistoryItem_Title)

	GB.ReturnNewZeroString(webkit_back_forward_list_item_get_title(THIS->item));

END_PROPERTY

BEGIN_PROPERTY(WebViewHistoryItem_Url)

	GB.ReturnNewZeroString(webkit_back_forward_list_item_get_uri(THIS->item));

END_PROPERTY

BEGIN_METHOD_VOID(WebViewHistoryItem_GoTo)

	webkit_web_view_go_to_back_forward_list_item(WIDGET, THIS->item);

END_METHOD

BEGIN_METHOD_VOID(WebViewHistory_Clear)

	fprintf(stderr, "gb.gtk3.webview: warning: WebKitGTK does not know how to clear its history at the moment.\n");

END_METHOD

BEGIN_METHOD(WebViewHistory_get, GB_INTEGER index)

	WebKitBackForwardListItem *item = webkit_back_forward_list_get_nth_item(HISTORY, VARG(index));
	
	if (!item)
		GB.ReturnNull();
	else
	{
		THIS->item = item;
		RETURN_SELF();
	}

END_METHOD

BEGIN_PROPERTY(WebViewHistory_CanGoBack)

	GB.ReturnBoolean(webkit_web_view_can_go_back(WIDGET));

END_PROPERTY

BEGIN_PROPERTY(WebViewHistory_CanGoForward)

	GB.ReturnBoolean(webkit_web_view_can_go_forward(WIDGET));

END_PROPERTY

//---------------------------------------------------------------------------

GB_DESC WebViewHistoryItemDesc[] = 
{
	GB_DECLARE_VIRTUAL(".WebView.History.Item"),
	
	GB_PROPERTY_READ("Title", "s", WebViewHistoryItem_Title),
	GB_PROPERTY_READ("Url", "s", WebViewHistoryItem_Url),
	GB_METHOD("GoTo", NULL, WebViewHistoryItem_GoTo, NULL),
	
	GB_END_DECLARE
};

GB_DESC WebViewHistoryDesc[] =
{
	GB_DECLARE_VIRTUAL(".WebView.History"),

	GB_METHOD("Clear", NULL, WebViewHistory_Clear, NULL),
	GB_METHOD("_get", ".WebView.History.Item", WebViewHistory_get, "(Index)i"),
	GB_PROPERTY_READ("CanGoBack", "b", WebViewHistory_CanGoBack),
	GB_PROPERTY_READ("CanGoForward", "b", WebViewHistory_CanGoForward),

	GB_END_DECLARE
};

GB_DESC WebViewDesc[] =
{
  GB_DECLARE("WebView", sizeof(CWEBVIEW)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, WebView_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, WebView_free, NULL),
  
  GB_PROPERTY("Url", "s", WebView_Url),
	GB_PROPERTY("Title", "s", WebView_Title),
	GB_PROPERTY("Zoom", "f", WebView_Zoom),
	GB_PROPERTY_READ("Icon", "Picture", WebView_Icon),
	GB_PROPERTY_READ("Progress", "f", WebView_Progress),
	GB_PROPERTY("NewView", "WebView", WebView_NewView),
	GB_PROPERTY_READ("Link", "s", WebView_Link),

	GB_METHOD("SetHtml", NULL, WebView_SetHtml, "(Html)s[(Root)s]"),
	GB_METHOD("Clear", NULL, WebView_Clear, NULL),

	GB_METHOD("Back", NULL, WebView_Back, NULL),
	GB_METHOD("Forward", NULL, WebView_Forward, NULL),
	GB_METHOD("Reload", NULL, WebView_Reload, "[(BypassCache)b]"),
	GB_METHOD("Stop", NULL, WebView_Stop, NULL),
	
	GB_METHOD("ExecJavascript", "s", WebView_ExecJavascript, "(Javascript)s"),
	
	GB_PROPERTY_SELF("History", ".WebView.History"),
	GB_PROPERTY_SELF("Settings", ".WebView.Settings"),

	GB_CONSTANT("_Properties", "s", "*,Url,Zoom=1"),
  GB_CONSTANT("_Group", "s", "View"),

	GB_EVENT("Title", NULL, NULL, &EVENT_TITLE),
	GB_EVENT("Url", NULL, NULL, &EVENT_URL),
	GB_EVENT("Icon", NULL, NULL, &EVENT_ICON),
	GB_EVENT("Start", NULL, NULL, &EVENT_START),
	GB_EVENT("Progress", NULL, NULL, &EVENT_PROGRESS),
	GB_EVENT("Finish", NULL, NULL, &EVENT_FINISH),
	GB_EVENT("Error", NULL, NULL, &EVENT_ERROR),
	GB_EVENT("Link", NULL, NULL, &EVENT_LINK),
	GB_EVENT("NewView", NULL, NULL, &EVENT_NEW_VIEW),

  GB_END_DECLARE
};
