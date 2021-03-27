#pragma once

#include <X11/keysymdef.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <deque>
#ifdef Success //this is stupid
  #undef Success
#endif

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1024
#define EPS 1e-7


void init_window();
void del_window();
void* window_thread(void*);

int is_window_open();


void get_string(char* number);
