#include <gtk/gtk.h>

static void
activate (GtkApplication* app,
          gpointer        user_data)
{
  GtkWidget *window;

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), (char*)user_data);
  gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);
  gtk_widget_show (window);

  //GdkPixbuf * icon_title = gdk_pixbuf_new_from_inline(-1, icon, FALSE, NULL);
  //gtk_window_set_icon(window, icon_title);
  // TODO: implement proper solution
  gtk_window_set_icon_from_file(window, "/home/mechanikas/Archyvas/projects/mokytojas/icon.svg", NULL);
}

int
main (int    argc,
      char **argv)
{
  GtkApplication *app;
  int status;
  guint8 a;

  app = gtk_application_new ("com.kikaitachi.mokytojas", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), "Window 1");
  g_signal_connect (app, "activate", G_CALLBACK (activate), "Window 2");
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}

