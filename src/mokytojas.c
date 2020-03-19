#include <gtk/gtk.h>
#include "kikaitachi.h"
#include "resources.h"

static GtkWidget *window_telemetry;

static int client_socket;

static GtkWidget *create_header_bar() {
	GtkWidget *header_bar = gtk_header_bar_new();

	/*GMenu *menu_model = g_menu_new();
	g_menu_append(menu_model, "Telemetry", NULL);
	g_menu_append(menu_model, "Control", NULL);
	g_menu_append(menu_model, "SLAM", NULL);
	g_menu_append(menu_model, "Video", NULL);

	GtkWidget *menu = gtk_menu_new_from_model((GMenuModel *)menu_model);

	GtkWidget *menu_button = gtk_menu_button_new();
	gtk_menu_button_set_popup(GTK_MENU_BUTTON (menu_button), menu);*/

	gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "Telemetry");
	gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header_bar), "Disconnected");
	gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);
	//gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), menu_button);

	return header_bar;
}

gboolean on_key_pressed(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    if (event->keyval == GDK_KEY_a || event->keyval == GDK_KEY_d) {
        kt_log_info ("Key pressed: %d", event->keyval);
        return TRUE;
    }
	kt_log_info ("Key pressed but not handled: %d, %s", event->keyval, gdk_keyval_name(event->keyval));
    return FALSE;
}

gboolean on_key_released(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    /*if (event->keyval == GDK_KEY_space) {
        printf("SPACE KEY PRESSED!");
        return TRUE;
    }*/
	kt_log_info ("Key released: %d, %s", event->keyval, gdk_keyval_name(event->keyval));
    return FALSE;
}

static GtkWidget *create_window(GtkApplication* app, GtkWidget *content) {
	GtkWidget *window = gtk_application_window_new (app);
	gtk_window_set_titlebar(GTK_WINDOW(window), create_header_bar());

	GtkTreeStore *tree_store = gtk_tree_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	GtkWidget *tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(tree_store));

	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();

	GtkTreeViewColumn *column_sensor_or_actuator = gtk_tree_view_column_new_with_attributes(
		"Sensor or actuator", renderer, "text", 0, NULL);

	GtkTreeViewColumn *column_value = gtk_tree_view_column_new_with_attributes(
		"Value or state", renderer, "text", 1, NULL);

	GtkTreeViewColumn *column_actions = gtk_tree_view_column_new_with_attributes(
		"Actions", renderer, "text", 2, NULL);

	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column_sensor_or_actuator);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column_value);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column_actions);

	//g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_widget_destroy), window);
	gtk_container_add (GTK_CONTAINER (window), tree_view);

	//GdkPixbuf * icon_title = gdk_pixbuf_new_from_inline(-1, icon, FALSE, NULL);
	//gtk_window_set_icon(window, icon_title);
	// TODO: implement proper solution
	//gtk_window_set_icon_from_file(GTK_WINDOW (window), "resource:///com/kikaitachi/mokytojas/icon.svg", NULL);
	//gtk_window_set_icon(GTK_WINDOW (window), gdk_pixbuf_new_from_resource("com/kikaitachi/mokytojas/icon.svg", NULL));
	gtk_window_set_icon(GTK_WINDOW(window), gdk_pixbuf_new_from_stream(
		g_resource_open_stream(mokytojas_get_resource(), "/com/kikaitachi/mokytojas/icon.svg", G_RESOURCE_LOOKUP_FLAGS_NONE, NULL), NULL, NULL));
	// TODO: Free the returned object with g_object_unref()
	gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

	GtkTreeIter iter, parent;
	gtk_tree_store_append(tree_store, &iter, NULL);
	gtk_tree_store_set(tree_store, &iter, 0, "Temperature", -1);
	gtk_tree_store_append(tree_store, &iter, NULL);
	gtk_tree_store_set(tree_store, &iter, 0, "Voltage", -1);
	gtk_tree_store_append(tree_store, &parent, NULL);
	gtk_tree_store_set(tree_store, &parent, 0, "Dynamixel XM430-W350-T", -1);
	gtk_tree_store_append(tree_store, &iter, &parent);
	gtk_tree_store_set(tree_store, &iter, 0, "Temperature 1", -1);
	gtk_tree_store_append(tree_store, &iter, &parent);
	gtk_tree_store_set(tree_store, &iter, 0, "Voltage 1", -1);

	//gtk_widget_add_events(GTK_WIDGET(window), GDK_KEY_RELEASE_MASK | GDK_KEY_PRESS_MASK);
	g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(on_key_pressed), NULL);
	g_signal_connect(G_OBJECT(window), "key-release-event", G_CALLBACK(on_key_released), NULL);

	return window;
}

static void activate(GtkApplication* app, gpointer user_data) {
	/*if (windows[0] == NULL) {
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
	}*/
	window_telemetry = create_window(app, NULL);
	gtk_widget_show_all(window_telemetry);
}

gboolean network_data_available_callback(GIOChannel *source, GIOCondition condition, gpointer data) {
	int fd = g_io_channel_unix_get_fd(source);
	char buf[KT_MAX_MSG_SIZE];
	ssize_t result = recv(fd, buf, sizeof(buf), 0);
	if (result == -1) {
		printf("Failed to receive packet\n");
	} else {
		printf("Received packet of size %zd:", result);
		for (int i = 0; i < result; i++) {
			printf(" %d", buf[i]);
		}
		printf("\n");
		char *buf_ptr = &buf;
		int buf_len = result;
		enum KT_MESSAGE msg_type;
		int telemetry_id;
		float battery_voltage;
		kt_msg_read_int(&buf_ptr, &buf_len, &msg_type);
		kt_msg_read_int(&buf_ptr, &buf_len, &telemetry_id);
		kt_msg_read_float(&buf_ptr, &buf_len, &battery_voltage);
		/*char string[str_len + 1];
		string[str_len] = 0;
		kt_msg_read(&buf_ptr, &buf_len, string, str_len);*/
		printf("Type: %d, id: %d, battery: %f\n", msg_type, telemetry_id, battery_voltage);
	}
	return TRUE;
}

int main (int argc, char **argv) {
	int server_socket = kt_udp_bind(argv[1]);
	if (server_socket == -1) {
		kt_log_last("Failed to create server socket");
		return -1;
	}
	client_socket = kt_udp_connect(argv[2], argv[3]);
	if (client_socket == -1) {
		kt_log_last("Failed to create client socket");
		return -1;
	}
	g_io_add_watch(g_io_channel_unix_new(server_socket), G_IO_IN, &network_data_available_callback, NULL);
	GtkApplication *app = gtk_application_new("com.kikaitachi.mokytojas", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	int status = g_application_run(G_APPLICATION (app), 1, argv);
	g_object_unref(app);
	return status;
}

