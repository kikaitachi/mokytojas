#include <gtk/gtk.h>

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
	gtk_window_set_icon_from_file(GTK_WINDOW (window), "/home/mechanikas/Archyvas/projects/mokytojas/icon.svg", NULL);

	gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);

	return window;
}

static void activate(GtkApplication* app, gpointer user_data) {
	if (windows[0] == NULL) {
		windows[0] = create_window(app, "Telemetry", NULL);
		windows[1] = create_window(app, "Control", NULL);
		windows[2] = create_window(app, "SLAM", NULL);
		windows[3] = create_window(app, "Video", NULL);
	}

	for (int i = 0; i < 4; i++) {
		gtk_widget_show_all(windows[i]);
	}
}

int main (int argc, char **argv) {
	GtkApplication *app = gtk_application_new ("com.kikaitachi.mokytojas", G_APPLICATION_FLAGS_NONE);
	g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
	int status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);
	return status;
}

