#pragma once

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <complex.h>
#include <sys/time.h>
#include "controller.h"


#define MIDI_NOTE_ON 0x90
#define MIDI_NOTE_OFF 0x80
#define PEDAL 176
#define PITCH_BEND 224
#define KEYS 88
#define LOWEST_KEY 21
#define SAMPLE_LEN 4410
#define NUM_CHANNELS 2



int init_midi(int argc, char *argv[]);
int del_midi();
void *midi_loop(void *arg);
unsigned char* get_notes_pressed();
