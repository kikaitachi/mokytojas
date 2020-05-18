#include "shortcuts.h"

typedef struct {
	int modifiers;
	int key_down;
	int key_up;
	int value;
} shortcut;

#define MAX_SHORTCUTS 100

static shortcut shortcuts[MAX_SHORTCUTS];
static int shortcut_count = 0;

int find_shortcut(int modifiers, int key_down, int key_up) {
	for (int i = 0; i < shortcut_count; i++) {
		if (shortcuts[i].modifiers == modifiers && shortcuts[i].key_down == key_down && shortcuts[i].key_up == key_up) {
			return shortcuts[i].value;
		}
	}
	return -1;
}

int add_shortcut(int modifiers, int key_down, int key_up, int value) {
	for (int i = 0; i < shortcut_count; i++) {
		if (shortcuts[i].modifiers == modifiers && shortcuts[i].key_down == key_down && shortcuts[i].key_up == key_up) {
			shortcuts[i].value = value;
			return 0;
		}
	}
	if (shortcut_count == MAX_SHORTCUTS) {
		return -1;
	}
	shortcuts[shortcut_count].modifiers = modifiers;
	shortcuts[shortcut_count].key_down = key_down;
	shortcuts[shortcut_count].key_up = key_up;
	shortcuts[shortcut_count].value = value;
	shortcut_count++;
	return 1;
}

