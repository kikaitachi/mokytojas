#include <gtk/gtk.h>
#include "resources.h"

static int screen_width = 1500;
static int screen_height = 800;

static GtkWidget *windows[] = { NULL, NULL, NULL, NULL };

static GtkWidget *create_header_bar(char* title) {
	GtkWidget *header_bar = gtk_header_bar_new();

	GMenu *menu_model = g_menu_new();
	g_menu_append(menu_model, "Telemetry", NULL);
	g_menu_append(menu_model, "Control", NULL);
	g_menu_append(menu_model, "SLAM", NULL);
	g_menu_append(menu_model, "Video", NULL);

	GtkWidget *menu = gtk_menu_new_from_model((GMenuModel *)menu_model);

	GtkWidget *menu_button = gtk_menu_button_new();
	gtk_menu_button_set_popup(GTK_MENU_BUTTON (menu_button), menu);


	gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), title);
	gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header_bar), "Disconnected");
	gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);
	gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), menu_button);

	return header_bar;
}

static GtkWidget *create_window(GtkApplication* app, char* title, GtkWidget *content) {
	GtkWidget *window = gtk_application_window_new (app);
	gtk_window_set_titlebar(GTK_WINDOW(window), create_header_bar(title));

	GtkWidget *button = gtk_button_new_with_label ("Hello, World!");

	g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_widget_destroy), window);
	gtk_container_add (GTK_CONTAINER (window), button);

	//GdkPixbuf * icon_title = gdk_pixbuf_new_from_inline(-1, icon, FALSE, NULL);
	//gtk_window_set_icon(window, icon_title);
	// TODO: implement proper solution
	//gtk_window_set_icon_from_file(GTK_WINDOW (window), "resource:///com/kikaitachi/mokytojas/icon.svg", NULL);
	//gtk_window_set_icon(GTK_WINDOW (window), gdk_pixbuf_new_from_resource("com/kikaitachi/mokytojas/icon.svg", NULL));
	gtk_window_set_icon(GTK_WINDOW (window), gdk_pixbuf_new_from_stream(
		g_resource_open_stream(mokytojas_get_resource(), "/com/kikaitachi/mokytojas/icon.svg", G_RESOURCE_LOOKUP_FLAGS_NONE, NULL), NULL, NULL));
	// TODO: Free the returned object with g_object_unref().

	return window;
}

static void activate(GtkApplication* app, gpointer user_data) {
	if (windows[0] == NULL) {
		windows[0] = create_window(app, "Telemetry", NULL);
		//gtk_widget_show_all(windows[0]);
		//gtk_window_get_size (GTK_WINDOW (windows[0]), &screen_width, &screen_height);
		//gtk_window_unmaximize(windows[0]);
		GdkScreen *screen = gtk_window_get_screen(GTK_WINDOW(windows[0]));
		printf ("W: %u x H:%u\n", gdk_screen_get_width(screen), gdk_screen_get_height(screen));
		GdkDisplay *display = gdk_screen_get_display (screen);
		GdkMonitor *monitor = gdk_display_get_primary_monitor (display);
		GdkRectangle workarea;
		gdk_monitor_get_workarea(monitor, &workarea);
		printf ("X: %u, Y: %u, W: %u x H:%u\n", workarea.x, workarea.y, workarea.width, workarea.height);
		gtk_window_move(GTK_WINDOW(windows[0]), workarea.x, workarea.y);
		windows[1] = create_window(app, "Control", NULL);
		gtk_window_set_default_size(GTK_WINDOW(windows[0]), workarea.x + workarea.width / 2, workarea.height / 2);
		gtk_window_move(GTK_WINDOW(windows[1]), workarea.width / 2, workarea.y);
		windows[2] = create_window(app, "SLAM", NULL);
		gtk_window_move(GTK_WINDOW(windows[2]), workarea.x, workarea.y + workarea.height / 2);
		windows[3] = create_window(app, "Video", NULL);
		gtk_window_move(GTK_WINDOW(windows[3]), workarea.x + workarea.width / 2, workarea.y + workarea.height / 2);
	}

	for (int i = 0; i < 4; i++) {
		gtk_widget_show_all(windows[i]);
	}
}

int main (int argc, char **argv) {
	//g_resources_register(mokytojas_get_resource());
	//gdk_pixbuf_new_from_file("resource:///com/kikaitachi/mokytojas/icon.svg", NULL);
	//printf("Resource: %d", g_resource_get_info(mokytojas_get_resource(), "/com/kikaitachi/mokytojas/icon.svg",
	//	G_RESOURCE_LOOKUP_FLAGS_NONE, NULL, NULL, NULL));
	GtkApplication *app = gtk_application_new ("com.kikaitachi.mokytojas", G_APPLICATION_FLAGS_NONE);
	//printf("Is null: %d?", mokytojas_get_resource() == NULL);
	g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
	int status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);
	return status;
}

