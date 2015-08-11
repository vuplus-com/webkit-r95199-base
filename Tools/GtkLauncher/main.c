/*
 * Copyright (C) 2006, 2007 Apple Inc.
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
 * Copyright (C) 2011 Lukasz Slachciak
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <errno.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <webkit/webkit.h>
#include <signal.h>
#include <execinfo.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/types.h>

#define LIBSOUP_USE_UNSTABLE_REQUEST_API
#include <libsoup/soup-cache.h>

static gint width;
static gint height;
static gchar * proxy;
static bool enableBenchmarking;
static gint windowCount = 0;
static GtkWidget* inspector_window;

static GtkWidget* createWindow(WebKitWebView** outWebView);

static void activateUriEntryCb(GtkWidget* entry, gpointer data)
{
    WebKitWebView *webView = g_object_get_data(G_OBJECT(entry), "web-view");
    const gchar* uri = gtk_entry_get_text(GTK_ENTRY(entry));
    g_assert(uri);
    webkit_web_view_load_uri(webView, uri);
}

#ifndef WEBKIT2
static void updateTitle(GtkWindow* window, WebKitWebView* webView)
{
    GString *string = g_string_new(webkit_web_view_get_title(webView));
    gdouble loadProgress = webkit_web_view_get_progress(webView) * 100;
    g_string_append(string, " - WebKit Launcher");
    if (loadProgress < 100)
        g_string_append_printf(string, " (%f%%)", loadProgress);
    gchar *title = g_string_free(string, FALSE);
    gtk_window_set_title(window, title);
    g_free(title);
}

static void linkHoverCb(WebKitWebView* page, const gchar* title, const gchar* link, GtkStatusbar* statusbar)
{
    guint statusContextId =
      GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(statusbar), "link-hover-context"));
    /* underflow is allowed */
    gtk_statusbar_pop(statusbar, statusContextId);
    if (link)
        gtk_statusbar_push(statusbar, statusContextId, link);
}

static void notifyTitleCb(WebKitWebView* webView, GParamSpec* pspec, GtkWidget* window)
{
    updateTitle(GTK_WINDOW(window), webView);
}

static void notifyLoadStatusCb(WebKitWebView* webView, GParamSpec* pspec, GtkWidget* uriEntry)
{
    if (webkit_web_view_get_load_status(webView) == WEBKIT_LOAD_COMMITTED) {
        WebKitWebFrame *frame = webkit_web_view_get_main_frame(webView);
        const gchar *uri = webkit_web_frame_get_uri(frame);
        if (uri)
            gtk_entry_set_text(GTK_ENTRY(uriEntry), uri);
    }
}

static void notifyProgressCb(WebKitWebView* webView, GParamSpec* pspec, GtkWidget* window)
{
    updateTitle(GTK_WINDOW(window), webView);
}
#endif

static void destroyCb(GtkWidget* widget, GtkWidget* window)
{
    if (g_atomic_int_dec_and_test(&windowCount))
      gtk_main_quit();
}

static void goBackCb(GtkWidget* widget,  WebKitWebView* webView)
{
    webkit_web_view_go_back(webView);
}

static void goForwardCb(GtkWidget* widget, WebKitWebView* webView)
{
    webkit_web_view_go_forward(webView);
}

#ifndef WEBKIT2
static WebKitWebView*
createWebViewCb(WebKitWebView* webView, WebKitWebFrame* web_frame, GtkWidget* window)
{
    WebKitWebView *newWebView;
    createWindow(&newWebView);
    webkit_web_view_set_settings(newWebView, webkit_web_view_get_settings(webView));
    return newWebView;
}

static gboolean webViewReadyCb(WebKitWebView* webView, GtkWidget* window)
{
    gtk_widget_grab_focus(GTK_WIDGET(webView));
    gtk_widget_show_all(window);
    return FALSE;
}

static gboolean closeWebViewCb(WebKitWebView* webView, GtkWidget* window)
{
    gtk_widget_destroy(window);
    return TRUE;
}
#endif


 static void
hide_window_cb(GtkWidget *widget, gpointer data)
{
    gtk_widget_hide(widget);
}

static WebKitWebView*
create_inspector_cb (WebKitWebInspector* web_inspector, WebKitWebView* page, gpointer data)
{
    GtkWidget* scrolled_window;
    GtkWidget* new_web_view;

    inspector_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(G_OBJECT(inspector_window), "delete-event",
                     G_CALLBACK(hide_window_cb), NULL);

    gtk_window_set_title(GTK_WINDOW(inspector_window), "Inspector");
    gtk_window_set_default_size(GTK_WINDOW(inspector_window), 400, 300);
    gtk_widget_show(inspector_window);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(inspector_window), scrolled_window);
    gtk_widget_show(scrolled_window);

    new_web_view = webkit_web_view_new();
    gtk_container_add(GTK_CONTAINER(scrolled_window), new_web_view);

    return WEBKIT_WEB_VIEW(new_web_view);
}

static gboolean
inspector_show_window_cb (WebKitWebInspector* inspector)
{
    gtk_widget_show(inspector_window);
    g_message("show_window: %p", inspector);
    return TRUE;
}

static gboolean
inspector_close_window_cb (WebKitWebInspector* inspector)
{
    g_message("close_window");
    return TRUE;
}

static gboolean
inspector_attach_window_cb (WebKitWebInspector* inspector)
{
    g_message("attach_window");
    return FALSE;
}

static gboolean
inspector_dettach_window_cb (WebKitWebInspector* inspector)
{
    g_message("dettach_window");
    return FALSE;
}

static gboolean
inspector_uri_changed_cb (WebKitWebInspector* inspector)
{
    g_message("uri_changed: %s", webkit_web_inspector_get_inspected_uri(inspector));
    return FALSE;
}

static gboolean
inspector_inspector_destroyed_cb (WebKitWebInspector* inspector)
{
    g_message("destroy");
    return FALSE;
}


static GtkWidget* createBrowser(GtkWidget* window, GtkWidget* uriEntry, GtkWidget* statusbar, WebKitWebView* webView)
{
   WebKitWebSettings *settings = webkit_web_settings_new();
   WebKitWebInspector *inspector;	
   
    GtkWidget *scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledWindow), GTK_POLICY_NEVER, GTK_POLICY_NEVER);

    gtk_container_add(GTK_CONTAINER(scrolledWindow), GTK_WIDGET(webView));

/*    g_object_set(G_OBJECT(settings), "enable-developer-extras", TRUE, NULL);
    webkit_web_view_set_settings(WEBKIT_WEB_VIEW(webView), settings); */


#ifndef WEBKIT2
/*    g_signal_connect(webView, "notify::title", G_CALLBACK(notifyTitleCb), window);
    g_signal_connect(webView, "notify::load-status", G_CALLBACK(notifyLoadStatusCb), uriEntry);
    g_signal_connect(webView, "notify::progress", G_CALLBACK(notifyProgressCb), window);
    g_signal_connect(webView, "hovering-over-link", G_CALLBACK(linkHoverCb), statusbar); */
    g_signal_connect(webView, "create-web-view", G_CALLBACK(createWebViewCb), window);
    g_signal_connect(webView, "web-view-ready", G_CALLBACK(webViewReadyCb), window);
    g_signal_connect(webView, "close-web-view", G_CALLBACK(closeWebViewCb), window);
#endif

		 inspector = webkit_web_view_get_inspector(webView);
		 g_signal_connect (G_OBJECT (inspector), "inspect-web-view", G_CALLBACK (create_inspector_cb), NULL);
		 g_signal_connect (G_OBJECT (inspector), "show-window", G_CALLBACK (inspector_show_window_cb), NULL);
		 g_signal_connect (G_OBJECT (inspector), "close-window", G_CALLBACK (inspector_close_window_cb), NULL);
		 g_signal_connect (G_OBJECT (inspector), "attach-window", G_CALLBACK (inspector_attach_window_cb), NULL);
		 g_signal_connect (G_OBJECT (inspector), "dettach-window", G_CALLBACK (inspector_dettach_window_cb), NULL);
		 g_signal_connect (G_OBJECT (inspector), "destroy", G_CALLBACK (inspector_inspector_destroyed_cb), NULL);
	
		 g_signal_connect (G_OBJECT (inspector), "notify::inspected-uri", G_CALLBACK (inspector_uri_changed_cb), NULL);
	

    return scrolledWindow;
}

static GtkWidget* createStatusbar()
{
    GtkStatusbar *statusbar = GTK_STATUSBAR(gtk_statusbar_new());
    guint statusContextId = gtk_statusbar_get_context_id(statusbar, "Link Hover");
    g_object_set_data(G_OBJECT(statusbar), "link-hover-context",
        GUINT_TO_POINTER(statusContextId));

    return GTK_WIDGET(statusbar);
}

static GtkWidget* createToolbar(GtkWidget* uriEntry, WebKitWebView* webView)
{
    GtkWidget *toolbar = gtk_toolbar_new();

#if GTK_CHECK_VERSION(2, 15, 0)
    gtk_orientable_set_orientation(GTK_ORIENTABLE(toolbar), GTK_ORIENTATION_HORIZONTAL);
#else
    gtk_toolbar_set_orientation(GTK_TOOLBAR(toolbar), GTK_ORIENTATION_HORIZONTAL);
#endif
    gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_BOTH_HORIZ);

    GtkToolItem *item;

    /* the back button */
    item = gtk_tool_button_new_from_stock(GTK_STOCK_GO_BACK);
    g_signal_connect(G_OBJECT(item), "clicked", G_CALLBACK(goBackCb), webView);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);

    /* The forward button */
    item = gtk_tool_button_new_from_stock(GTK_STOCK_GO_FORWARD);
    g_signal_connect(G_OBJECT(item), "clicked", G_CALLBACK(goForwardCb), webView);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);

    /* The URL entry */
    item = gtk_tool_item_new();
    gtk_tool_item_set_expand(item, TRUE);
    gtk_container_add(GTK_CONTAINER(item), uriEntry);
    g_signal_connect(G_OBJECT(uriEntry), "activate", G_CALLBACK(activateUriEntryCb), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);

    /* The go button */
    g_object_set_data(G_OBJECT(uriEntry), "web-view", webView);
    item = gtk_tool_button_new_from_stock(GTK_STOCK_OK);
    g_signal_connect_swapped(G_OBJECT(item), "clicked", G_CALLBACK(activateUriEntryCb), (gpointer)uriEntry);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);

    return toolbar;
}

static WebKitWebView* _webView;

static gboolean expose(GtkWidget *widget, GdkEventExpose *event, gpointer userdata)
{
   cairo_t *cr = gdk_cairo_create(widget->window);

       cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.0); /* transparent */

    /* draw the background */
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);

    cairo_destroy(cr);

    return FALSE;
}

static GtkWidget* createWindow(WebKitWebView** outWebView)
{
    WebKitWebView *webView;
    GtkWidget *vbox;
    GtkWidget *window;
    GtkWidget *uriEntry;
    GtkWidget *statusbar;

    g_atomic_int_inc(&windowCount);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), width ? width : 1280, height ? height : 720);
    gtk_widget_set_name(window, "GtkLauncher");
	gtk_window_set_decorated( window, FALSE );

	gtk_widget_set_app_paintable(window, TRUE);
	
	g_signal_connect(G_OBJECT(window), "expose-event", G_CALLBACK(expose), NULL);

    webView = WEBKIT_WEB_VIEW(webkit_web_view_new());
    uriEntry = gtk_entry_new();

#ifdef GTK_API_VERSION_2
    vbox = gtk_vbox_new(FALSE, 0);
#else
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#endif
/*    statusbar = createStatusbar(webView); */
/*    gtk_box_pack_start(GTK_BOX(vbox), createToolbar(uriEntry, webView), FALSE, FALSE, 0); */
      gtk_box_pack_start(GTK_BOX(vbox), createBrowser(window, uriEntry, statusbar, webView), TRUE, TRUE, 0); 
/*    gtk_box_pack_start(GTK_BOX(vbox), statusbar, FALSE, FALSE, 0); */

    gtk_container_add(GTK_CONTAINER(window), vbox);

    g_signal_connect(window, "destroy", G_CALLBACK(destroyCb), NULL);

	_webView = webView;

    if (outWebView)
        *outWebView = webView;

    return window;
}

static gchar* filenameToURL(const char* filename)
{
    if (!g_file_test(filename, G_FILE_TEST_EXISTS))
        return 0;

    GFile *gfile = g_file_new_for_path(filename);
    gchar *fileURL = g_file_get_uri(gfile);
    g_object_unref(gfile);

    return fileURL;
}

#ifndef WEBKIT2
static gboolean parseOptionEntryCallback(const gchar *optionNameFull, const gchar *value, WebKitWebSettings *webSettings, GError **error)
{
    if (strlen(optionNameFull) <= 2) {
        g_set_error(error, G_OPTION_ERROR, G_OPTION_ERROR_FAILED, "Invalid option %s", optionNameFull);
        return FALSE;
    }

    /* We have two -- in option name so remove them. */
    const gchar *optionName = optionNameFull + 2;
    GParamSpec *spec = g_object_class_find_property(G_OBJECT_GET_CLASS(webSettings), optionName);
    if (!spec) {
        g_set_error(error, G_OPTION_ERROR, G_OPTION_ERROR_FAILED, "Cannot find web settings for option %s", optionNameFull);
        return FALSE;
    }

    switch (G_PARAM_SPEC_VALUE_TYPE(spec)) {
    case G_TYPE_BOOLEAN: {
        gboolean propertyValue = TRUE;
        if (value && g_ascii_strcasecmp(value, "true") && strcmp(value, "1"))
            propertyValue = FALSE;
        g_object_set(G_OBJECT(webSettings), optionName, propertyValue, NULL);
        break;
    }
    case G_TYPE_STRING:
        g_object_set(G_OBJECT(webSettings), optionName, value, NULL);
        break;
    case G_TYPE_INT: {
        glong propertyValue;
        gchar *end;

        errno = 0;
        propertyValue = g_ascii_strtoll(value, &end, 0);
        if (errno == ERANGE || propertyValue > G_MAXINT || propertyValue < G_MININT) {
            g_set_error(error, G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE, "Integer value '%s' for %s out of range", value, optionNameFull);
            return FALSE;
        }
        if (errno || value == end) {
            g_set_error(error, G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE, "Cannot parse integer value '%s' for %s", value, optionNameFull);
            return FALSE;
        }
        g_object_set(G_OBJECT(webSettings), optionName, propertyValue, NULL);
        break;
    }
    case G_TYPE_FLOAT: {
        gdouble propertyValue;
        gchar *end;

        errno = 0;
        propertyValue = g_ascii_strtod(value, &end);
        if (errno == ERANGE || propertyValue > G_MAXFLOAT || propertyValue < G_MINFLOAT) {
            g_set_error(error, G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE, "Float value '%s' for %s out of range", value, optionNameFull);
            return FALSE;
        }
        if (errno || value == end) {
            g_set_error(error, G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE, "Cannot parse float value '%s' for %s", value, optionNameFull);
            return FALSE;
        }
        g_object_set(G_OBJECT(webSettings), optionName, propertyValue, NULL);
        break;
    }
    default:
        g_assert_not_reached();
    }

    return TRUE;
}

static GOptionEntry* getOptionEntriesFromWebKitWebSettings(WebKitWebSettings *webSettings)
{
    GParamSpec **propertySpecs;
    GOptionEntry *optionEntries;
    guint numProperties, numEntries, i;

    propertySpecs = g_object_class_list_properties(G_OBJECT_GET_CLASS(webSettings), &numProperties);
    if (!propertySpecs)
        return 0;

    optionEntries = g_new0(GOptionEntry, numProperties + 1);
    numEntries = 0;
    for (i = 0; i < numProperties; i++) {
        GParamSpec *param = propertySpecs[i];

        /* Fill in structures only for writable properties. */
        if (!param || !(param->flags & G_PARAM_WRITABLE))
            continue;

        GType gParamType = G_PARAM_SPEC_VALUE_TYPE(param);
        if (gParamType == G_TYPE_BOOLEAN || gParamType == G_TYPE_STRING || gParamType == G_TYPE_INT
            || gParamType == G_TYPE_FLOAT) {
            GOptionEntry *optionEntry = &optionEntries[numEntries++];
            optionEntry->long_name = g_param_spec_get_name(param);

            /* There is no easy way to figure our short name for generated option entries.
               optionEntry.short_name=*/
            /* For bool arguments "enable" type make option argument not required. */
            if (gParamType == G_TYPE_BOOLEAN && (strstr(optionEntry->long_name, "enable")))
                optionEntry->flags = G_OPTION_FLAG_OPTIONAL_ARG;
            optionEntry->arg = G_OPTION_ARG_CALLBACK;
            optionEntry->arg_data = parseOptionEntryCallback;
            optionEntry->description = g_param_spec_get_blurb(param);
            optionEntry->arg_description = g_type_name(gParamType);
        }
    }
    g_free(propertySpecs);

    return optionEntries;
}

static gboolean addWebSettingsGroupToContext(GOptionContext *context, WebKitWebSettings* webkitSettings)
{
    GOptionEntry *optionEntries = getOptionEntriesFromWebKitWebSettings(webkitSettings);
    if (!optionEntries)
        return FALSE;

    GOptionGroup *webSettingsGroup = g_option_group_new("websettings",
                                                        "WebKitWebSettings writable properties for default WebKitWebView",
                                                        "WebKitWebSettings properties",
                                                        webkitSettings,
                                                        NULL);
    g_option_group_add_entries(webSettingsGroup, optionEntries);
    g_free(optionEntries);

    /* Option context takes ownership of the group. */
    g_option_context_add_group(context, webSettingsGroup);

    return TRUE;
}
#endif

void init_remote();
void show_stackframe( int sig );
extern GTlsDatabase*                g_tls_file_database_gnutls_new                   (const gchar *anchor_file);
static GtkWidget *main_window ;

static void
request_started (SoupSession *session, SoupMessage *msg,
		 SoupSocket *socket, gpointer user_data)
{
	char* uri = NULL;
	if( msg )
	{
		SoupURI* soup_uri = soup_message_get_uri( msg );
		if( soup_uri )
		{
			uri = soup_uri_to_string( soup_uri, 0 );
		}
	}

	if( uri )
	{
		fprintf( stderr, "SOUP - Request Started.. %s\n", uri );
	}
	else
	{
		fprintf( stderr, "SOUP - Request Started..\n" );
	}
}

static void
ea_connection_created (SoupSession *session, GObject *conn, gpointer user_data)
{
	fprintf( stderr, "SOUP - Connection Created..\n" );
}


static void request_queued   (SoupSession *session,
                                                        SoupMessage *msg,
                                                        gpointer     user_data)
{
	char* uri = NULL;
	if( msg )
	{
		SoupURI* soup_uri = soup_message_get_uri( msg );
		if( soup_uri )
		{
			uri = soup_uri_to_string( soup_uri, 0 );
		}
	}

	if( uri )
	{
		fprintf( stderr, "SOUP - Request Queued.. %s\n", uri );
	}
	else
	{
		fprintf( stderr, "SOUP - Request Queued\n" );
	}
}

static void request_unqueued   (SoupSession *session,
                                                        SoupMessage *msg,
                                                        gpointer     user_data)
{
	char* uri = NULL;
	if( msg )
	{
		SoupURI* soup_uri = soup_message_get_uri( msg );
		if( soup_uri )
		{
			uri = soup_uri_to_string( soup_uri, 0 );
		}
	}

	if( uri )
	{
		fprintf( stderr, "SOUP - Request UnQueued.. %s\n", uri );
	}
	else
	{	
		fprintf( stderr, "SOUP - Request UnQueued\n" );
	}
}

int main(int argc, char* argv[])
{
#ifndef WEBKIT2
    WebKitWebSettings *webkitSettings = 0;
#endif
    const gchar **uriArguments = 0;
    const GOptionEntry commandLineOptions[] =
    {
        { "proxy", 'p', 0, G_OPTION_ARG_STRING, &proxy, "proxy", "proxy" },
        { "width", 'w', 0, G_OPTION_ARG_INT, &width, "width", "width" },
        { "height", 'h', 0, G_OPTION_ARG_INT, &height, "height", "height" },
        { "benchmark", 'b', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_NONE, &enableBenchmarking, "benchmark the load+render time", "benchmark" },
        { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &uriArguments, 0, "[URL]" },
        { 0, 0, 0, 0, 0, 0, 0 }
    };

    gtk_init(&argc, &argv);
    if (!g_thread_supported())
        g_thread_init(NULL);

    GOptionContext *context = g_option_context_new(0);
    g_option_context_add_main_entries(context, commandLineOptions, 0);
    g_option_context_add_group(context, gtk_get_option_group(TRUE));
#ifndef WEBKIT2
    webkitSettings = webkit_web_settings_new();
    if (!addWebSettingsGroupToContext(context, webkitSettings)) {
        g_object_unref(webkitSettings);
        webkitSettings = 0;
    }
#endif

    GError *error = 0;
    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_printerr("Cannot parse arguments: %s\n", error->message);
        g_error_free(error);
        g_option_context_free(context);

        return 1;
    }
    g_option_context_free(context);

#ifndef WEBKIT2
#ifdef SOUP_TYPE_PROXY_RESOLVER_DEFAULT
    soup_session_add_feature_by_type(webkit_get_default_session(), SOUP_TYPE_PROXY_RESOLVER_DEFAULT);
#else
    const char *httpProxy = g_getenv("http_proxy");
    if (httpProxy) {
        SoupURI *proxyUri = soup_uri_new(httpProxy);
        g_object_set(webkit_get_default_session(), SOUP_SESSION_PROXY_URI, proxyUri, NULL);
        soup_uri_free(proxyUri);
    }
#endif
#endif

    WebKitWebView *webView;
    main_window = createWindow(&webView);
	
    GValue value = { 0, { { 0 } } };
    g_value_init( &value, G_TYPE_INT );
    g_value_set_int( &value, (int)1 );		
	g_object_set_property( G_OBJECT( webView ), "transparent", &value ); 

/*	g_object_set (webkit_get_default_session(),
					 SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE, TRUE,
					 SOUP_SESSION_SSL_STRICT, FALSE,
					 NULL);	*/

	g_object_set (webkit_get_default_session(),
						 SOUP_SESSION_IDLE_TIMEOUT, 60,
						 NULL); 

    if (proxy) {
        SoupURI *proxyUri = soup_uri_new(proxy);
        g_object_set(webkit_get_default_session(), SOUP_SESSION_PROXY_URI, proxyUri, NULL);
        soup_uri_free(proxyUri);
    }
#if 1	
		{
		GTlsBackend *backend;
		GTlsDatabase *database;
		GTlsDatabase *check;
		GTlsCertificate* certificate;
		GTlsCertificateFlags errors;		
		GError *error = NULL;


		backend = g_tls_backend_get_default ();
		g_assert (G_IS_TLS_BACKEND (backend));

	/*	database = g_tls_backend_get_default_database (backend);  */
		database = g_tls_file_database_new( "/etc/pki/tls/cert.pem", &error );  

		g_assert (G_IS_TLS_DATABASE (database));
		fprintf( stderr, "database = %x\n", database );


	/*	g_object_set( database, "anchors", "/home/kdhong/webkit/ST/x86/Webkit-r95199-x86.release/ca.pem", NULL );  */
	/*	certificate = g_tls_certificate_new_from_file( "/home/kdhong/webkit/ST/x86/Webkit-r95199-x86.release/cl_855.pem", &error );
		g_assert_no_error (error);		
		errors = g_tls_database_verify_chain( database, certificate, G_TLS_DATABASE_PURPOSE_AUTHENTICATE_SERVER, 
			NULL, NULL, G_TLS_DATABASE_VERIFY_NONE, NULL, &error );

		fprintf( stderr, "error = %x\n", errors );
		g_assert_no_error (error);		
	    g_assert_cmpuint (errors, ==, 0);			*/
        g_object_set(webkit_get_default_session(), "SSL credentials", database, NULL);		
        g_object_set(webkit_get_default_session(), "ssl-strict", FALSE, NULL);		


		/* Soup Debug 
		SoupSocket *socket = NULL;

		g_signal_connect (webkit_get_default_session(), "request-started",
						   G_CALLBACK (request_started),
						   &socket);

		
		g_signal_connect (webkit_get_default_session(), "connection-created",
				  G_CALLBACK (ea_connection_created), NULL);

		g_signal_connect (webkit_get_default_session(), "request_queued",
				  G_CALLBACK (request_queued), NULL);

		g_signal_connect (webkit_get_default_session(), "request_unqueued",
				  G_CALLBACK (request_unqueued), NULL);

		/* Soup Debug */


		/* Set SOUP Context - libsoup 2.40 */
/*        g_object_set(webkit_get_default_session(), 
					SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_CONTENT_DECODER,
					SOUP_SESSION_USE_THREAD_CONTEXT, TRUE,
					NULL);		*/

		/* Enable disk cache */
		FILE* checkFp = fopen( "/mnt/hdd0/program/cache/cache_enabled", "r" );

		if( checkFp )
		{
			fprintf( stderr, "Setting Cache..\n" );
			fclose( checkFp );			
			SoupCache* cache = soup_cache_new("/mnt/hdd0/program/cache/", SOUP_CACHE_SINGLE_USER);
	    	soup_session_add_feature(webkit_get_default_session(), SOUP_SESSION_FEATURE(cache));
		}
		else
		{
			fprintf( stderr, "No HDD cache available\n" );
		}		
    	}
#endif

#ifndef WEBKIT2
    if (webkitSettings) {
        webkit_web_view_set_settings(WEBKIT_WEB_VIEW(webView), webkitSettings);
        g_object_unref(webkitSettings);
    }
#endif

    webkit_set_cache_model(WEBKIT_CACHE_MODEL_DOCUMENT_BROWSER);

    const gchar *uri = (uriArguments ? uriArguments[0] : "http://www.google.com/");
    gchar *fileURL = filenameToURL(uri);

    webkit_web_view_load_uri(webView, fileURL ? fileURL : uri);
    g_free(fileURL);

	init_remote();

	{
/*	struct sigaction sa;
	sa.sa_handler = show_stackframe;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGBUS, &sa, NULL);
	sigaction(5, &sa, NULL);	
	sigaction(7, &sa, NULL);
	sigaction(SIGILL, &sa, NULL);	
	sigaction(SIGTERM, &sa, NULL);	
	sigaction(SIGABRT, &sa, NULL);	*/

	signal( SIGSEGV, show_stackframe );
	signal( SIGBUS, SIG_IGN );

	signal( SIGPIPE, SIG_IGN );
	}

    gtk_widget_grab_focus(GTK_WIDGET(webView));
    gtk_widget_show_all(main_window);
    gtk_main();

    return 0;
}

#define LOG_ERR( a... ) { fprintf( stderr, "ERR - " ); fprintf( stderr, ##a ); fprintf( stderr, "\n" ); }
#define LOG_TRACE( a... ) { fprintf( stderr, "TRACE - " ); fprintf( stderr, ##a ); fprintf( stderr, "\n" ); }
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <gdk/gdkkeysyms.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

typedef struct 
{

	int mCmdType;
	union
	{
		int mKeyCode;
		char mURL[512];
	}u;
}WebkitCmd;

enum
{
	CMD_KEY = 0,
	CMD_CLOSE,
	CMD_LOAD,
	CMD_REDRAW,
	CMD_SHOW,
	CMD_HIDE,
	CMD_KEYSET
};

enum
{
	KEY_FROM_NONE = 0,
	KEY_FROM_RCU = 1,
	KEY_FROM_TACT,
	KEY_FROM_FRONTWHEEL,

	FLAG_NONE = 0,
	FLAG_REPEATED = 1,
	
	VKEY_NO_KEY = 0,
	VKEY_OK,
	VKEY_UP,
	VKEY_DOWN,
	VKEY_LEFT,
	VKEY_RIGHT,
	VKEY_RED,
	VKEY_GREEN,
	VKEY_YELLOW,
	VKEY_BLUE,
	VKEY_0,	

	VKEY_1,
	VKEY_2,
	VKEY_3,
	VKEY_4,
	VKEY_5,
	VKEY_6,
	VKEY_7,
	VKEY_8,
	VKEY_9,
	VKEY_FF,	

	VKEY_REV,
	VKEY_PLAY,
	VKEY_REC,
	VKEY_PAUSE,
	VKEY_STOP,
	VKEY_SLOW,
	VKEY_MENU,
	VKEY_EPG,
	VKEY_TEXT,
	VKEY_INFO,	

	VKEY_BACK,
	VKEY_EXIT,
	VKEY_POWER,
	VKEY_MUTE,
	VKEY_PROG_UP,
	VKEY_PROG_DOWN,
	VKEY_VOL_UP,
	VKEY_VOL_DOWN,
	VKEY_HELP,
	VKEY_MEDIA,	

	VKEY_ARCHIVE,
	VKEY_PREVCH,
	VKEY_FAVORITE,
	VKEY_OPT,
	VKEY_PIP,
	VKEY_SLEEP,
	VKEY_HISTORY,
	VKEY_ADDBOOKMARK,
	VKEY_BMKWINDOW,
	VKEY_JUMP_FORWARD,	

	VKEY_JUMP_BACKWARD,
	VKEY_TV_RADIO,
	/* added by lael98 20090331 */
	VKEY_SUBTITLE,
	VKEY_STAR,
	VKEY_CHECK,			
	VKEY_SEARCH,
	VKEY_EDIT,		
	VKEY_DELETE,
	VKEY_FUNC_A,
	VKEY_FUNC_B,		

	VKEY_VOD_TIMESHIFT,
	VKEY_ADULT,
	VKEY_VOD,
	VKEY_SOURCE,
	VKEY_VFORMAT,
	VKEY_AFORMAT,
	VKEY_WIDE,
	VKEY_LIST,

	VKEY_PORTAL,
	VKEY_PAUSE_PLAY,

	VKEY_F1,

	VKEY_FRONT_MENU = 0x80,
	VKEY_FRONT_EXIT,
	VKEY_FRONT_AUX,
	VKEY_FRONT_TV_R,
	VKEY_FRONT_OK,
	VKEY_FRONT_CCW,
	VKEY_FRONT_CW,

	VKEY_CHANGE_ADDR1 = 0x74,
	VKEY_CHANGE_ADDR2 = 0x75,
	VKEY_CHANGE_ADDR3 = 0x76,
	VKEY_CHANGE_ADDR4 = 0x77,
};

enum
{
	KEYSET_RED = 0x1,
	KEYSET_GREEN = 0x2,
	KEYSET_YELLOW = 0x4,
	KEYSET_BLUE = 0x8,
	KEYSET_NAVIGATION = 0x10,
	KEYSET_VCR = 0x20,
	KEYSET_SCROLL = 0x40,
	KEYSET_INFO = 0x80,
	KEYSET_NUMERIC = 0x100,
	KEYSET_ALPHA = 0x200,
	KEYSET_OTHER = 0x400,
};


static char _url[512];
static unsigned int _keyset = 0;

static int webkit_go( gpointer param )
{
	LOG_TRACE( "1 url = %s", _url );
	webkit_web_view_load_uri( _webView, _url );
	LOG_TRACE( "2 url = %s", _url );
	
	return FALSE;
}

static int webkit_redraw( gpointer param )
{
	LOG_TRACE( " >>>> REDRAW!!!" );
	gtk_widget_queue_draw_area( _webView, 0, 0, 1280, 720 );
	LOG_TRACE( " <<<< REDRAW!!!" );
	return FALSE;
}

static int webkit_show( gpointer param )
{
	int show = (int)param;
	LOG_TRACE( " >>>> Show %d", show );
	if( show )
	{
		gtk_widget_show( _webView ); 
		gtk_widget_grab_focus(GTK_WIDGET(_webView));
	}
	else
	{
		gtk_widget_hide( _webView ); 
		gtk_widget_grab_focus(GTK_WIDGET(_webView));
	}

	return FALSE;
}


static void _webkitCmdCallback( WebkitCmd* aCmd, int aSockFd )
{
	LOG_TRACE( "Cmd Type = %d", aCmd->mCmdType );

	switch( aCmd->mCmdType )
	{
		
		case CMD_LOAD:
		{
			strcpy( _url, aCmd->u.mURL );
			gdk_threads_add_idle_full( -100, webkit_go, (void*)_url, NULL );
			break;
		}
		case CMD_REDRAW:
		{
			strcpy( _url, aCmd->u.mURL  );
			gdk_threads_add_idle_full( -100, webkit_redraw, (void*)NULL, NULL );
			break;
		}
		case CMD_CLOSE:
		{
			strcpy( _url, "file:///mnt/webkit/bin/none.html" );
			gdk_threads_add_idle_full( -100, webkit_go, (void*)_url, NULL );
			break;
		}
		case CMD_KEY:
		{
			int code = ( aCmd->u.mKeyCode & 0xffff );
			int flag = ( aCmd->u.mKeyCode >> 16 );
			LOG_TRACE( "Code = %x, Flag = %x", code, flag );
			

			switch( aCmd->u.mKeyCode )
			{
				case VKEY_EXIT:
					if( _keyset & KEYSET_NAVIGATION )
					{
						my_webkit_input_key( GDK_F4 );
						aCmd->u.mKeyCode = VKEY_NO_KEY;
					}
					write( aSockFd, aCmd, sizeof( WebkitCmd ) );
					break;						
				case VKEY_UP:
					if( _keyset & KEYSET_NAVIGATION )
					{
						my_webkit_input_key( GDK_Up );
						aCmd->u.mKeyCode = VKEY_NO_KEY;						
					}
					write( aSockFd, aCmd, sizeof( WebkitCmd ) );
					break;
				case VKEY_DOWN:
					if( _keyset & KEYSET_NAVIGATION )
					{
						my_webkit_input_key( GDK_Down );
						aCmd->u.mKeyCode = VKEY_NO_KEY; 					
					}
					write( aSockFd, aCmd, sizeof( WebkitCmd ) );					
					break;
				case VKEY_LEFT:
					if( _keyset & KEYSET_NAVIGATION )
					{					
						my_webkit_input_key( GDK_Left );
						aCmd->u.mKeyCode = VKEY_NO_KEY; 					
					}
					write( aSockFd, aCmd, sizeof( WebkitCmd ) );					
					break;
				case VKEY_RIGHT:
					if( _keyset & KEYSET_NAVIGATION )
					{					
						my_webkit_input_key( GDK_Right );
						aCmd->u.mKeyCode = VKEY_NO_KEY; 					
					}
					write( aSockFd, aCmd, sizeof( WebkitCmd ) );											
					break;
				case VKEY_OK:
					if( _keyset & KEYSET_NAVIGATION )
					{									
						my_webkit_input_key( GDK_Return );
						aCmd->u.mKeyCode = VKEY_NO_KEY;
					}
					write( aSockFd, aCmd, sizeof( WebkitCmd ) );
					break;
				case VKEY_FF:
					if( _keyset & KEYSET_VCR )
					{
						my_webkit_input_key( GDK_F12 );
						aCmd->u.mKeyCode = VKEY_NO_KEY;
					}
					write( aSockFd, aCmd, sizeof( WebkitCmd ) );
					break;
				case VKEY_REV:
					if( _keyset & KEYSET_VCR )
					{
						my_webkit_input_key( GDK_F11 );
						aCmd->u.mKeyCode = VKEY_NO_KEY;
					}
					write( aSockFd, aCmd, sizeof( WebkitCmd ) );						
					break;
				case VKEY_0...VKEY_9:			
					if( _keyset & KEYSET_NUMERIC )
					{					
						my_webkit_input_key( GDK_0 + (aCmd->u.mKeyCode - VKEY_0) );
						aCmd->u.mKeyCode = VKEY_NO_KEY;
					}
					write( aSockFd, aCmd, sizeof( WebkitCmd ) );						
					break;
				case VKEY_RED:
					if( _keyset & KEYSET_RED )
					{					
						my_webkit_input_key( GDK_F5 );
						aCmd->u.mKeyCode = VKEY_NO_KEY;
					}
					write( aSockFd, aCmd, sizeof( WebkitCmd ) );						
					break;
				case VKEY_GREEN:
					if( _keyset & KEYSET_GREEN )
					{					
						my_webkit_input_key( GDK_F6 );
						aCmd->u.mKeyCode = VKEY_NO_KEY;
					}
					write( aSockFd, aCmd, sizeof( WebkitCmd ) );						
					break;
				case VKEY_YELLOW:
					if( _keyset & KEYSET_YELLOW )
					{
						my_webkit_input_key( GDK_F7 );
						aCmd->u.mKeyCode = VKEY_NO_KEY;
					}
					write( aSockFd, aCmd, sizeof( WebkitCmd ) );
					break;
				case VKEY_BLUE:
					if( _keyset & KEYSET_BLUE )
					{
						my_webkit_input_key( GDK_F8 );
						aCmd->u.mKeyCode = VKEY_NO_KEY;
					}
					write( aSockFd, aCmd, sizeof( WebkitCmd ) );										
					break;
				case VKEY_PLAY:
					if( _keyset & KEYSET_VCR )
					{					
						my_webkit_input_key( GDK_F9 );
						aCmd->u.mKeyCode = VKEY_NO_KEY;
					}
					write( aSockFd, aCmd, sizeof( WebkitCmd ) );				
					break;
				case VKEY_STOP:
					if( _keyset & KEYSET_VCR )
					{					
						my_webkit_input_key( GDK_F10 );
						aCmd->u.mKeyCode = VKEY_NO_KEY;
					}
					write( aSockFd, aCmd, sizeof( WebkitCmd ) );										
					break;						
				case VKEY_PAUSE:
					if( _keyset & KEYSET_VCR )
					{					
						my_webkit_input_key( GDK_Pause );
						aCmd->u.mKeyCode = VKEY_NO_KEY;
					}
					write( aSockFd, aCmd, sizeof( WebkitCmd ) );										
					break;
			}
			
			break;
		}
		case CMD_SHOW:			
			gdk_threads_add_idle_full( -100, webkit_show, (void*)1, NULL );
			break;
		case CMD_HIDE:
			gdk_threads_add_idle_full( -100, webkit_show, (void*)0, NULL );			
			break;
		case CMD_KEYSET:
			_keyset = aCmd->u.mKeyCode;
			break;
			
	}
}

static int webkit_input_key( gpointer param )
{
	GdkEvent *event    = gdk_event_new (GDK_KEY_PRESS);
	GdkWindow* window = GTK_WIDGET( main_window )->window;
	GTimeVal tv;
	g_get_current_time (&tv);

	event->any.window     = (GdkWindow*)g_object_ref( window );
	event->any.send_event = FALSE;

	int keyCode = (int)param;

	int code = ( keyCode & 0xffff );
	int flag = ( keyCode >> 16 );

	int hardware = 0;

	switch( code )
	{
		case GDK_Up:
			hardware = 0x67;
			break;
		case GDK_Down:
			hardware = 0x6c;
			break;
		case GDK_Page_Up:
			hardware = 0x68;
			break;
		case GDK_Page_Down:
			hardware = 0x6d;
			break;
		case GDK_Return:
			hardware = 0x1c;
			break;
		case GDK_Tab:
			hardware = 0xf;
			break;
			
			
	}

	LOG_TRACE( "Code = %x Flag = %x Window = %x", code, flag, window );

	event->key.keyval = (int)code;
	event->key.time = (guint32) tv.tv_sec * 1000 + tv.tv_usec / 1000;
	event->key.length = 0;
	event->key.is_modifier = 0;
	event->key.hardware_keycode = hardware;
	event->key.state = flag;
	event->key.group = 0;
	event->key.string = NULL;

	gtk_widget_grab_focus(GTK_WIDGET(_webView));

	gtk_main_do_event( event );
	gdk_event_free( event );

	event    = gdk_event_new (GDK_KEY_RELEASE);
	g_get_current_time (&tv);

	event->any.window     = (GdkWindow*)g_object_ref( window );
	event->any.send_event = FALSE;

	event->key.keyval = (int)code;
	event->key.time = (guint32) tv.tv_sec * 1000 + tv.tv_usec / 1000;
	event->key.length = 0;
	event->key.is_modifier = 0;
	event->key.hardware_keycode = 0;
	event->key.state = flag;
	event->key.group = 0;
	event->key.string = NULL;

	gtk_main_do_event( event );
	gdk_event_free( event );

	return FALSE;
}


int 
my_webkit_input_key( int aKey )
{
    gdk_threads_add_idle_full( -100, webkit_input_key, (void*)aKey, NULL );
    return 0;	
}

static void* ServerThread( void* aParam )
{
	char* browser_ip = "192.168.3.110";
	char* system_ip = "192.168.3.235";
	
	if( getenv( "HBB_BROWSER_IP" ) )
	{
		browser_ip = getenv( "HBB_BROWSER_IP" );
	}
	if( getenv( "HBB_SYSTEM_IP" ) )
	{
		system_ip = getenv( "HBB_SYSTEM_IP" );
	}

	struct sockaddr_in addr;
	addr.sin_family = PF_INET;
	addr.sin_port = htons( 56892+1 );
	addr.sin_addr.s_addr = inet_addr( browser_ip );

	int addrlen = sizeof( addr );

	int newsockfd = 0;
	int fd = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );

	if( fd < 0 )
	{
		LOG_ERR( "Socket Creation Failure" );
		return NULL;
	}

	int ret = 0;

	int flag = 1;

	int ret1 = setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, (char *) &flag, sizeof( int ) );
	int ret2 = setsockopt( fd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof( int ) );	

	ret = bind( fd, (struct sockaddr*)&addr, addrlen );

	if( ret < 0 )
	{
		LOG_ERR( "Socket Binding Failure" );
		return NULL;
	}

	ret = listen( fd, 5 );

	if( ret < 0 )
	{
		LOG_ERR( "Socket Listen Failure" );
		return NULL;
	}

	while( (newsockfd = accept( fd, (struct sockaddr*)&addr, &addrlen )) != -1 )
	{
		while( true )
		{
			fd_set rdfs;
			int read_length = 0;
			int ret;
			
			FD_ZERO( &rdfs );
			FD_SET( newsockfd, &rdfs );

			LOG_TRACE( "" );

			if( ( ret = select( newsockfd + 1, &rdfs, NULL, NULL, NULL ) ) >= 0 )
			{
				LOG_TRACE( "" );					
				WebkitCmd cmd;

				read_length = read( newsockfd, &cmd, sizeof( cmd ) );
				LOG_TRACE( "readlength = %d", read_length );

				if( read_length == sizeof( cmd ) )
				{
					LOG_TRACE( "Socket read successful" );					
					_webkitCmdCallback( &cmd, newsockfd );
				}
				else
				{
					LOG_ERR( "!!!!!!!!ERROR::::READ LENGTH=%d", read_length );
					continue;
				}					
			}
			else
			{
				break;
			}

/*			LOG_TRACE( "Socket Select Returns : %d", ret ); */
		}
	
		close( newsockfd ); 		
	}
}

static void* CmdThread( void* aParam )
{
	struct sockaddr_in addr;
	addr.sin_family = PF_INET;
	addr.sin_port = htons( 56894+1 );
	addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );

	int addrlen = sizeof( addr );

	int newsockfd = 0;
	int fd = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );

	if( fd < 0 )
	{
		LOG_ERR( "Socket Creation Failure" );
		return NULL;
	}

	int ret = 0;
	int flag = 1;

	int ret1 = setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, (char *) &flag, sizeof( int ) );
	int ret2 = setsockopt( fd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof( int ) );	
	
	ret = bind( fd, (struct sockaddr*)&addr, addrlen );

	if( ret < 0 )
	{
		LOG_ERR( "Socket Binding Failure" );
		return NULL;
	}

	ret = listen( fd, 5 );

	if( ret < 0 )
	{
		LOG_ERR( "Socket Listen Failure" );
		return NULL;
	}

	while( (newsockfd = accept( fd, (struct sockaddr*)&addr, &addrlen )) != -1 )
	{
		while( true )
		{
			fd_set rdfs;
			int read_length = 0;
			int ret;
			
			FD_ZERO( &rdfs );
			FD_SET( newsockfd, &rdfs );

			LOG_TRACE( "" );

			if( ( ret = select( newsockfd + 1, &rdfs, NULL, NULL, NULL ) ) >= 0 )
			{
				LOG_TRACE( "" );					
				WebkitCmd cmd;

				read_length = read( newsockfd, &cmd, sizeof( cmd ) );
				LOG_TRACE( "readlength = %d", read_length );

				if( read_length == sizeof( cmd ) )
				{
					LOG_TRACE( "Socket read successful" );					
					_webkitCmdCallback( &cmd, newsockfd );
				}
				else
				{
					LOG_ERR( "!!!!!!!!ERROR::::READ LENGTH=%d", read_length );
					continue;
				}					
			}
			else
			{
				break;
			}

/*			LOG_TRACE( "Socket Select Returns : %d", ret ); */
		}
	
		close( newsockfd ); 		
	}
}



void init_remote()
{
	pthread_t pthreadId;
	pthread_create( &pthreadId, NULL, ServerThread, NULL );

	pthread_t pthreadId2;
	pthread_create( &pthreadId2, NULL, CmdThread, NULL );
}

void show_stackframe( int sig ) {
  void *trace[16];
  char **messages = (char **)NULL;
  int i, j, k, trace_size = 0;
  
  trace_size = backtrace(trace, 16);
  messages = backtrace_symbols(trace, trace_size);
  LOG_TRACE("[bt] Caught %d Execution path: traces %d\n", sig, trace_size );
  bool found = false;
  
  for (i=0; i<trace_size; ++i)
  {
	LOG_TRACE("[bt] %s\n", messages[i]);
	if( found == true )
	{
		int len = strlen( messages[i] );
		char* ptr = messages[i];
		for( j = 0; j < len-1; j++ )
		{
			if( ptr[j] == '[' && ptr[j+1] == '0' )
			{
				char tmp[32];
				memset( tmp, 0, 32 );
				for( k = 0; k < 32; k++ )
				{
					tmp[k] = ptr[j++];
					if( ptr[j] == ']' )
						break;
				}
				LOG_TRACE( "tmp = %s", tmp );
				unsigned long addr = strtoul( tmp + 1, NULL, 16 );
				LOG_TRACE( "addr = %x", addr );
				break;
			}
		}

		found = false;
	}

	char* ptr = messages[i];
	if( ptr[0] == '[' )
		found = true;
	
  }  
}


