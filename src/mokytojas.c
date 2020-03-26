#include <arpa/inet.h>
#include <gtk/gtk.h>
#include "kikaitachi.h"
#include "resources.h"
#include "telemetry.h"

static GtkWindow *window_telemetry;

static GtkTreeStore *telemetry_tree;
static GtkWidget *tree_view;
static GtkWidget *header_bar;

static int client_socket;
static struct sockaddr_in broadcast_addr;
static socklen_t broadcast_addr_len = sizeof(broadcast_addr);

static struct sockaddr_in client_addr;
static socklen_t client_addr_len = sizeof(client_addr);

static gboolean is_connected = FALSE;
static gboolean is_packet_received = FALSE;

GHashTable *key_is_pressed, *key_down_to_action, *key_up_to_action;

static GtkWidget *create_header_bar() {
	header_bar = gtk_header_bar_new();

	/*GMenu *menu_model = g_menu_new();
	g_menu_append(menu_model, "Telemetry", NULL);
	g_menu_append(menu_model, "Control", NULL);
	g_menu_append(menu_model, "SLAM", NULL);
	g_menu_append(menu_model, "Video", NULL);

	GtkWidget *menu = gtk_menu_new_from_model((GMenuModel *)menu_model);

	GtkWidget *menu_button = gtk_menu_button_new();
	gtk_menu_button_set_popup(GTK_MENU_BUTTON (menu_button), menu);*/

	gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "Telemetry");
	gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);
	//gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), menu_button);

	return header_bar;
}

gboolean on_key_pressed(GtkWidget *widget, GdkEventKey *event, gpointer data) {
	gpointer value = g_hash_table_lookup(key_down_to_action, (gconstpointer)&event->keyval);
	if (value != NULL) {
		if (!g_hash_table_contains(key_is_pressed, (gconstpointer)&event->keyval)) {
			int id = *((int *)value);
			char buf[KT_MAX_MSG_SIZE];
			int buf_len = KT_MAX_MSG_SIZE;
			void *buf_ptr = &buf;
			kt_msg_write_int(&buf_ptr, &buf_len, KT_MSG_TELEMETRY);
			kt_msg_write_int(&buf_ptr, &buf_len, id);
			kt_udp_send(client_socket, buf, KT_MAX_MSG_SIZE - buf_len);
			int *key = malloc(sizeof(int));
			*key = event->keyval;
			g_hash_table_add(key_is_pressed, key);
		}
		return TRUE;
	}
	return FALSE;
}

gboolean on_key_released(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    /*if (event->keyval == GDK_KEY_space) {
        printf("SPACE KEY PRESSED!");
        return TRUE;
    }*/
	g_hash_table_remove(key_is_pressed, &event->keyval);
	kt_log_info ("Key released: %d, %s", event->keyval, gdk_keyval_name(event->keyval));
    return FALSE;
}

static GtkWindow *create_window(GtkApplication* app, GtkWidget *content) {
	GtkWindow *window = GTK_WINDOW(gtk_application_window_new(app));
	gtk_window_set_titlebar(window, create_header_bar());

	// id, name, type, value, key to press, key to release
	telemetry_tree = gtk_tree_store_new(6, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(telemetry_tree));
	gtk_tree_view_set_grid_lines(tree_view, GTK_TREE_VIEW_GRID_LINES_BOTH);

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

	gtk_window_set_icon(window, gdk_pixbuf_new_from_stream(
		g_resource_open_stream(mokytojas_get_resource(), "/com/kikaitachi/mokytojas/icon.svg", G_RESOURCE_LOOKUP_FLAGS_NONE, NULL), NULL, NULL));
	// TODO: Free the returned object with g_object_unref()
	gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

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
			} else {
				gtk_tree_store_append(telemetry_tree, &iter, NULL);
			}
			int key_down, key_up;
			if (type == KT_TELEMETRY_TYPE_ACTION) {
				kt_msg_read_int(&buf_ptr, &buf_len, &key_down);
				if (key_down != 0) {
					int *key = malloc(sizeof(int));
					int *value = malloc(sizeof(int));
					*key = key_down;
					*value = id;
					g_hash_table_insert(key_down_to_action, key, value);
				}
				kt_msg_read_int(&buf_ptr, &buf_len, &key_up);
				gtk_tree_store_set(telemetry_tree, &iter, 0, id, 1, name, 2, type, 4, gdk_keyval_name(key_down), 5, gdk_keyval_name(key_up), -1);
			} else {
				gtk_tree_store_set(telemetry_tree, &iter, 0, id, 1, name, 2, type, -1);
			}
			gtk_tree_view_expand_all(GTK_TREE_VIEW(tree_view));
		}
		break;
	}
}

void handle_telemetry_message(void *buf_ptr, int buf_len) {
	int id;
	int len;
	char *result;
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
				case KT_TELEMETRY_TYPE_INT:
					kt_msg_read_int(&buf_ptr, &buf_len, &value_int);
					len = snprintf(NULL, 0, "%d", value_int);
					result = (char *)malloc(len + 1);
					snprintf(result, len + 1, "%d", value_int);
					gtk_tree_store_set(telemetry_tree, &iter, 3, result, -1);
					break;
				case KT_TELEMETRY_TYPE_FLOAT:
					kt_msg_read_float(&buf_ptr, &buf_len, &value_float);
					len = snprintf(NULL, 0, "%f", value_float);
					result = (char *)malloc(len + 1);
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
}

gboolean on_new_message(GIOChannel *source, GIOCondition condition, gpointer data) {
	int fd = g_io_channel_unix_get_fd(source);
	char buf[KT_MAX_MSG_SIZE];
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);
	ssize_t result = recvfrom(fd, buf, sizeof(buf), 0, &addr, &addr_len);
	/*struct ifaddrs *current_network_interface = nextwork_interfaces;
	while (current_network_interface != NULL) {
		printf("Network Interface Name :- %s\n",current_network_interface->ifa_name);
		printf("Network Address of %s :- %d\n",current_network_interface->ifa_name,current_network_interface->ifa_addr);
		printf("Network Data :- %d \n",current_network_interface->ifa_data);
		printf("Socket Data : -%c\n",current_network_interface->ifa_addr->sa_data);
		if (current_network_interface->ifa_addr->sa_family == AF_INET) {
			kt_log_debug("AF_INET");
			//if (((struct sockaddr_in *)current_network_interface->ifa_addr)->sin_addr.s_addr == addr.sin_addr.s_addr) {
			//	kt_log_debug ("Ignore message from its own address %s", inet_ntoa(addr.sin_addr.s_addr));
			//	return TRUE;
			//}
		}
		current_network_interface = current_network_interface->ifa_next;
	}*/
	if (result == -1) {
		kt_log_last("Failed to receive UDP packet");
	} else {
		kt_log_debug("Received packet of %d bytes from addr len %d, family %d", result, addr_len, addr.sin_family);
		void *buf_ptr = &buf;
		int buf_len = result;
		int msg_type;
		kt_msg_read_int(&buf_ptr, &buf_len, &msg_type);
		switch (msg_type) {
			case KT_MSG_DISCOVER:
				if (result > 1) { // If not own broadcasted message
					is_connected = TRUE;
					memcpy(&client_addr, &addr, addr_len);
					client_addr.sin_port = broadcast_addr.sin_port;
					if (connect(client_socket, (const struct sockaddr *)&client_addr, client_addr_len) < 0) {
						kt_log_last ("Failed to connect socket %d to %s", client_socket, inet_ntoa(client_addr.sin_addr));
					}
					char subtitle[256];
					snprintf(subtitle, sizeof(subtitle), "Connected to %s", inet_ntoa(client_addr.sin_addr));
					gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header_bar), subtitle);
					is_packet_received = TRUE;
				}
				break;
			case KT_MSG_TELEMETRY:
				handle_telemetry_message(buf_ptr, buf_len);
				is_packet_received = TRUE;
				break;
			case KT_MSG_TELEMETRY_DEFINITION:
				handle_telemetry_definition_message(buf_ptr, buf_len);
				is_packet_received = TRUE;
				break;
			default:
				kt_log_error("Received message of unknown type: %d", msg_type);
				break;
		}
	}
	return TRUE;
}

gint on_connection_timeout(gpointer data) {
	if (is_packet_received) {
		is_packet_received = FALSE;
	} else {
		is_connected = FALSE;
	}
	if (!is_connected) {
		gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header_bar), "Disconnected");
		uint8_t msg_type = KT_MSG_DISCOVER;
		if (sendto(client_socket, &msg_type, 1, 0, (struct sockaddr *)&broadcast_addr, broadcast_addr_len) == -1) {
			kt_log_last("Failed to send DISCOVER message");
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
		client_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (client_socket == -1) {
			kt_log_last("Failed to create client socket");
			exit(-1);
		}
		int broadcastEnable = 1;
		if (setsockopt(client_socket, SOL_SOCKET, SO_BROADCAST, (void *)&broadcastEnable, sizeof(broadcastEnable)) < 0) {
			kt_log_last("Failed to enable broadcast for socket");
		}

		g_io_add_watch(g_io_channel_unix_new(server_socket), G_IO_IN, &on_new_message, NULL);
		window_telemetry = create_window(app, NULL);
		gtk_window_set_position(window_telemetry, GTK_WIN_POS_CENTER_ALWAYS);
		gtk_widget_show_all(GTK_WIDGET(window_telemetry));

		memset(&broadcast_addr, 0, broadcast_addr_len);
		broadcast_addr.sin_family = AF_INET;
		broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
		broadcast_addr.sin_port = htons(atoi(((char **)user_data)[2]));

		key_is_pressed = g_hash_table_new_full(g_int_hash, g_int_equal, free, NULL);
		key_down_to_action = g_hash_table_new_full(g_int_hash, g_int_equal, free, free);
		key_up_to_action = g_hash_table_new_full(g_int_hash, g_int_equal, free, free);

		g_timeout_add(3000, on_connection_timeout, NULL);
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

