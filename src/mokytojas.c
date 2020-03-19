#include <gtk/gtk.h>
#include "kikaitachi.h"
#include "resources.h"
#include "telemetry.h"

static GtkWindow *window_telemetry;

static GtkTreeStore *telemetry_tree;
static GtkWidget *tree_view;

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

static GtkWindow *create_window(GtkApplication* app, GtkWidget *content) {
	GtkWindow *window = GTK_WINDOW(gtk_application_window_new(app));
	gtk_window_set_titlebar(window, create_header_bar());

	// id, name, type, value, key to press, key to release
	telemetry_tree = gtk_tree_store_new(6, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT);

	tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(telemetry_tree));

	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();

	GtkTreeViewColumn *column_sensor_or_actuator = gtk_tree_view_column_new_with_attributes(
		"Sensor or actuator", renderer, "text", 1, NULL);

	GtkTreeViewColumn *column_value = gtk_tree_view_column_new_with_attributes(
		"Value or state", renderer, "text", 3, NULL);

	GtkTreeViewColumn *column_key_press = gtk_tree_view_column_new_with_attributes(
		"On key press", renderer, "text", 4, NULL);

	GtkTreeViewColumn *column_key_release = gtk_tree_view_column_new_with_attributes(
		"On key release", renderer, "text", 5, NULL);

	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column_sensor_or_actuator);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column_value);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column_key_press);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column_key_release);

	//g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_widget_destroy), window);
	gtk_container_add (GTK_CONTAINER (window), tree_view);

	//GdkPixbuf * icon_title = gdk_pixbuf_new_from_inline(-1, icon, FALSE, NULL);
	//gtk_window_set_icon(window, icon_title);
	// TODO: implement proper solution
	//gtk_window_set_icon_from_file(GTK_WINDOW (window), "resource:///com/kikaitachi/mokytojas/icon.svg", NULL);
	//gtk_window_set_icon(GTK_WINDOW (window), gdk_pixbuf_new_from_resource("com/kikaitachi/mokytojas/icon.svg", NULL));
	gtk_window_set_icon(window, gdk_pixbuf_new_from_stream(
		g_resource_open_stream(mokytojas_get_resource(), "/com/kikaitachi/mokytojas/icon.svg", G_RESOURCE_LOOKUP_FLAGS_NONE, NULL), NULL, NULL));
	// TODO: Free the returned object with g_object_unref()
	gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

	/*GtkTreeIter iter, parent;
	gtk_tree_store_append(tree_store, &iter, NULL);
	gtk_tree_store_set(tree_store, &iter, 0, "Temperature", -1);
	gtk_tree_store_append(tree_store, &iter, NULL);
	gtk_tree_store_set(tree_store, &iter, 0, "Voltage", -1);
	gtk_tree_store_append(tree_store, &parent, NULL);
	gtk_tree_store_set(tree_store, &parent, 0, "Dynamixel XM430-W350-T", -1);
	gtk_tree_store_append(tree_store, &iter, &parent);
	gtk_tree_store_set(tree_store, &iter, 0, "Temperature 1", -1);
	gtk_tree_store_append(tree_store, &iter, &parent);
	gtk_tree_store_set(tree_store, &iter, 0, "Voltage 1", -1);*/

	//gtk_widget_add_events(GTK_WIDGET(window), GDK_KEY_RELEASE_MASK | GDK_KEY_PRESS_MASK);
	g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(on_key_pressed), NULL);
	g_signal_connect(G_OBJECT(window), "key-release-event", G_CALLBACK(on_key_released), NULL);

	return window;
}

void handle_telemetry_definition_message(void *buf_ptr, int buf_len) {
	GtkTreeIter iter, parent;
	int id;
	while (kt_msg_read_int(&buf_ptr, &buf_len, &id) != -1) {
		int parent_id;
		kt_msg_read_int(&buf_ptr, &buf_len, &parent_id);

		int name_len;
		kt_msg_read_int(&buf_ptr, &buf_len, &name_len);

		//char name[name_len + 1];
		char *name = malloc(name_len + 1);
		kt_msg_read(&buf_ptr, &buf_len, name, name_len);
		name[name_len] = 0;

		int type;
		kt_msg_read_int(&buf_ptr, &buf_len, &type);

		kt_log_debug("id: %d, parent id: %d, name_len: %d, name: %s, type: %d",
			  id, parent_id, name_len, name, type);

		if (!find_tree_item_by_id(GTK_TREE_MODEL(telemetry_tree), &iter, id)) {
			if (find_tree_item_by_id(GTK_TREE_MODEL(telemetry_tree), &parent, parent_id)) {
				gtk_tree_store_append(telemetry_tree, &iter, &parent);
				gtk_tree_store_set(telemetry_tree, &iter, 0, id, 1, name, 2, type, -1);
			} else {
				gtk_tree_store_append(telemetry_tree, &iter, NULL);
				gtk_tree_store_set(telemetry_tree, &iter, 0, id, 1, name, 2, type, -1);
			}
			gtk_tree_view_expand_all(GTK_TREE_VIEW(tree_view));
		}
		break;
	}
}

void handle_telemetry_message(void *buf_ptr, int buf_len) {
	int id;
	float value_float;
	int value_int;
	char value_string[KT_MAX_MSG_SIZE];
	while (kt_msg_read_int(&buf_ptr, &buf_len, &id) != -1) {
		GtkTreeIter iter;
		if (find_tree_item_by_id(GTK_TREE_MODEL(telemetry_tree), &iter, id)) {
			GValue value = { 0, };
			gtk_tree_model_get_value(GTK_TREE_MODEL(telemetry_tree), &iter, 2, &value);
			int type = g_value_get_int(&value);
			g_value_unset(&value);
			switch (type) {
				case KT_TELEMETRY_TYPE_FLOAT:
					kt_msg_read_float(&buf_ptr, &buf_len, &value_float);
					/*GValue value = { 0, };
					gtk_tree_model_get_value(GTK_TREE_MODEL(telemetry_tree), &iter, 3, &value);
					g_value_set_string(&value, "test");
					g_value_unset(&value);*/
					int len = snprintf(NULL, 0, "%f", value_float);
					char *result = (char *)malloc(len + 1);
					snprintf(result, len + 1, "%f", value_float);
					gtk_tree_store_set(telemetry_tree, &iter, 3, result, -1);
					break;
				case KT_TELEMETRY_TYPE_STRING:
					kt_msg_read_int(&buf_ptr, &buf_len, &value_int);
					kt_msg_read(&buf_ptr, &buf_len, value_string, value_int);
					value_string[value_int] = 0;
					gtk_tree_store_set(telemetry_tree, &iter, 3, value_string, -1);
					break;
				default:
					kt_log_error("Received telemetry message with id %d and unknown type %d", id, type);
					return;
			}
		} else {
			kt_log_info("Received telemetry message with unknown id %d, requesting definitions", id);
			char buf[KT_MAX_MSG_SIZE];
			int buf_len = KT_MAX_MSG_SIZE;
			void *buf_ptr = &buf;
			kt_msg_write_int(&buf_ptr, &buf_len, KT_MSG_TELEMETRY_DEFINITION);
			kt_udp_send(client_socket, buf, KT_MAX_MSG_SIZE - buf_len);
			return;
		}
	}
	/*char string[str_len + 1];
	string[str_len] = 0;
	kt_msg_read(&buf_ptr, &buf_len, string, str_len);*/
}

gboolean on_new_message(GIOChannel *source, GIOCondition condition, gpointer data) {
	int fd = g_io_channel_unix_get_fd(source);
	char buf[KT_MAX_MSG_SIZE];
	ssize_t result = recv(fd, buf, sizeof(buf), 0);
	if (result == -1) {
		kt_log_last("Failed to receive UDP packet");
	} else {
		kt_log_debug("Received packet of %zd bytes", result);
		void *buf_ptr = &buf;
		int buf_len = result;
		int msg_type;
		kt_msg_read_int(&buf_ptr, &buf_len, &msg_type);
		switch (msg_type) {
			case KT_MSG_TELEMETRY:
				handle_telemetry_message(buf_ptr, buf_len);
				break;
			case KT_MSG_TELEMETRY_DEFINITION:
				handle_telemetry_definition_message(buf_ptr, buf_len);
				break;
			default:
				kt_log_error ("Received message of unknown type: %d", msg_type);
				break;
		}
	}
	return TRUE;
}

void on_activate(GtkApplication* app, gpointer user_data) {
	if (window_telemetry == NULL) {
		int server_socket = kt_udp_bind(((char **)user_data)[1]);
		if (server_socket == -1) {
			kt_log_last("Failed to create server socket");
			exit(-1);
		}
		client_socket = kt_udp_connect(((char **)user_data)[2], ((char **)user_data)[3]);
		if (client_socket == -1) {
			kt_log_last("Failed to create client socket");
			exit(-1);
		}
		g_io_add_watch(g_io_channel_unix_new(server_socket), G_IO_IN, &on_new_message, NULL);
		window_telemetry = create_window(app, NULL);
		gtk_window_set_position(window_telemetry, GTK_WIN_POS_CENTER_ALWAYS);
		gtk_widget_show_all(GTK_WIDGET(window_telemetry));
	} else {
		gtk_window_present(window_telemetry);
	}
}

int main (int argc, char **argv) {
	GtkApplication *app = gtk_application_new("com.kikaitachi.mokytojas", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(on_activate), argv);
	int status = g_application_run(G_APPLICATION (app), 1, argv);
	g_object_unref(app);
	return status;
}

