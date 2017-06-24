#include "common.h"
#include "x86.h"
#include "scan_code.h"

#define NR_KEYS 37

enum {KEY_STATE_EMPTY, KEY_STATE_WAIT_RELEASE, KEY_STATE_RELEASE, KEY_STATE_PRESS};

/* Only the following keys are used in NEMU-PAL. */
static const int keycode_array[] = {
	K_UP, K_DOWN, K_LEFT, K_RIGHT, K_ENTER, K_SPACE, K_UP_MINUS, K_UP_PLUS, K_COMMA, K_DOT, K_SLASH, K_A, K_B, K_C, K_D, K_E, K_F, K_G, K_H, K_I, K_J, K_K, K_L, K_M, K_N, K_O, K_P, K_Q, K_R, K_S, K_T, K_U, K_V, K_W, K_X, K_Y, K_Z
};



static int key_state[NR_KEYS];

void keyboard_event() {
	int key_code = in_byte(0x60); 
	//printk("%x\n", key_code);
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
	//cli();
	int i;
	for(i = 0; i<NR_KEYS; ++i) {
		if(key_state[i] == KEY_STATE_PRESS) {
			key_state[i] = KEY_STATE_WAIT_RELEASE;
			//sti();           /* WARNING: DON'T allow interruptions during the process!*/
								/* That's because we are ring3 which dived into kernel! */
			//printk("%d\n", keycode_array[i]);
			return keycode_array[i];
		}
		else if(key_state[i] == KEY_STATE_RELEASE) {
			key_state[i] = KEY_STATE_EMPTY;
			//sti(); 
			//printk("%d\n", keycode_array[i]);
			return keycode_array[i] + 0x80;
		}
	}
	//sti();
	return 0xff;
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
