#include "common.h"
#include "x86.h"
#include "device/scan_code.h"

#define NR_KEYS 7

enum {KEY_STATE_EMPTY, KEY_STATE_WAIT_RELEASE, KEY_STATE_RELEASE, KEY_STATE_PRESS};

/* Only the following keys are used in NEMU-PAL. */
static const int keycode_array[] = {
	K_UP, K_DOWN, K_LEFT, K_RIGHT, K_ENTER, K_SPACE, K_Q
};



static int key_state[NR_KEYS];

void keyboard_event(void) {
	int key_code = inb(0x60); 
	int i;
	for (i = 0; i < NR_KEYS; i++){
		if(key_code == keycode_array[i]) {
			switch(key_state[i]) {
				case KEY_STATE_EMPTY: 
				case KEY_STATE_PRESS: 
				case KEY_STATE_WAIT_RELEASE: key_state[i] = KEY_STATE_PRESS; break;
				case KEY_STATE_RELEASE: break;
				default: panic("keyboard error!\n");break;
			}
			break;
		}
		else if(key_code == keycode_array[i] + 0x80) {
			switch(key_state[i]) {
				case KEY_STATE_EMPTY: break;
				case KEY_STATE_PRESS: 
				case KEY_STATE_WAIT_RELEASE: 
				case KEY_STATE_RELEASE: key_state[i] = KEY_STATE_RELEASE; break;
				default: panic("keyboard error!\n");break;
			}
			break;
		}
	}
}

int handle_keys() {
	cli();
	int i;
	for(i = 0; i<NR_KEYS; ++i) {
		if(key_state[i] == KEY_STATE_PRESS) {
			key_state[i] = KEY_STATE_WAIT_RELEASE;
			sti(); 
			//printk("%d\n", keycode_array[i]);
			return keycode_array[i];
		}
		else if(key_state[i] == KEY_STATE_RELEASE) {
			key_state[i] = KEY_STATE_EMPTY;
			sti(); 
			//printk("%d\n", keycode_array[i]);
			return keycode_array[i] + 0x80;
		}
	}
	sti(); return 0xff;
}




static inline int
get_keycode(int index) {
	assert(index >= 0 && index < NR_KEYS);
	return keycode_array[index];
}

static inline int
query_key(int index) {
	assert(index >= 0 && index < NR_KEYS);
	return key_state[index];
}

static inline void
release_key(int index) {
	assert(index >= 0 && index < NR_KEYS);
	key_state[index] = KEY_STATE_WAIT_RELEASE;
}

static inline void
clear_key(int index) {
	assert(index >= 0 && index < NR_KEYS);
	key_state[index] = KEY_STATE_EMPTY;
}
