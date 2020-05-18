#ifndef _SHORTCUT_
#define _SHORTCUT_

/**
 * Find value by shortcut.
 * Returns -1 if not found.
 */
int find_shortcut(int modifiers, int key_down, int key_up);

/**
 * Associate value with given shortcut.
 * Return 1 if value was added, 0 if value existed and was updated, -1 when too many shortcuts are defined.
 */
int add_shortcut(int modifiers, int key_down, int key_up, int value);

#endif

