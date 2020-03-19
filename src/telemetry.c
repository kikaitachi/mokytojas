#include "telemetry.h"

static gboolean tree_item_by_id(GtkTreeModel *model, GtkTreeIter *node, GtkTreeIter *result, int id) {
	do {
		GValue value = { 0, };
		gtk_tree_model_get_value(model, node, 0, &value);
		gint curr_id = g_value_get_int(&value);
		g_value_unset(&value);
		if (id == curr_id) {
			*result = *node;
			return TRUE;
		}
		GtkTreeIter child;
		if (gtk_tree_model_iter_children(model, &child, node)) {
			if (tree_item_by_id(model, &child, result, id)) {
				return TRUE;
			}
		}
	} while (gtk_tree_model_iter_next(model, node));
	return FALSE;
}

gboolean find_tree_item_by_id(GtkTreeModel *model, GtkTreeIter *iter, int id) {
	GtkTreeIter node;
	if (gtk_tree_model_get_iter_first(model, &node) ) {
		return tree_item_by_id(model, &node, iter, id);
	}
	return FALSE;
}

