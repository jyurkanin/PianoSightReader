#include "piano_window.h"
#include "audio_engine.h"
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <vector>
#include <queue>
#include <sys/ioctl.h>

pthread_t m_thread;
pthread_t piano_midi_thread;
int MidiFD;
unsigned char midiNotesPressed[0xFF]; /* this records all the notes jus pressed by pitch maximum notes on is KEYS*/
unsigned char midiNotesReleased[0xFF]; //records the notes just released
unsigned char midiNotesSustained[0xFF];
int sustain = 0;


unsigned char* get_notes_pressed(){
  return midiNotesPressed;
}

void breakOnMe(){
  //break me on, Break on meeeeeee
}


int init_midi(int argc, char *argv[]){
  char *MIDI_DEVICE = argv[1];
  int flags;
  
  MidiFD = open(MIDI_DEVICE, O_RDONLY);
  
  flags = fcntl(MidiFD, F_GETFL, 0);
  if(fcntl(MidiFD, F_SETFL, flags | O_NONBLOCK)){
    printf("A real bad ERror %d\n", errno);
  }
  
  if(MidiFD < 0){
    printf("Error: Could not open device %s\n", MIDI_DEVICE);
    exit(-1);
  }
  
  pthread_create(&piano_midi_thread, NULL, &midi_loop, NULL);
  //pthread_create(&m_thread, NULL, &audio_thread, NULL);
  
  return 1;//Controller::init_controller(argc, argv);
}


int del_midi(){
  pthread_join(piano_midi_thread, NULL);
  close(MidiFD);

  pthread_join(m_thread, NULL);
  return Controller::exit_controller();
}

void *midi_loop(void *ignoreme){
  int bend = 64;
  unsigned char packet[4];
  std::queue<unsigned char> incoming;
  unsigned char temp;
  unsigned char last_status_byte;
  while(is_window_open()){
    if(read(MidiFD, &temp, sizeof(temp)) <= 0){
      usleep(10);
      continue;
    }
    else{
      if(incoming.size() == 0 && !(temp & 0b10000000)){ //so if the first byte in the sequence is not a status byte, use the last status byte.
          incoming.push(last_status_byte);
      }
      incoming.push(temp);
    }
    
    if(incoming.size() >= 3){
      packet[0] = incoming.front(); incoming.pop();
      packet[1] = incoming.front(); incoming.pop();
      packet[2] = incoming.front(); incoming.pop();
    }
    else continue;
    
//    printf("keyboard %d %d %d\n", packet[0], packet[1], packet[2]);
    
    last_status_byte = packet[0];
    
    switch(packet[0] & 0b11110000){
    case(MIDI_NOTE_OFF):
      if(sustain < 64){	
	midiNotesReleased[packet[1]] = 1;
      }
      else{
	midiNotesSustained[packet[1]] = 1;
      }
      break;
    case(MIDI_NOTE_ON):
      midiNotesPressed[packet[1]] = packet[2];            
      break;
    case(PEDAL):
        if(sustain >= 64 && packet[2] < 64){ //if the sustain is released.
	  for(int i = 0; i < 128; i++){ //releases all the notes and sets the sustained notes to 0.
	    midiNotesReleased[i] |= midiNotesSustained[i];
	    midiNotesSustained[i] = 0;
	  }
	}
	if(sustain < 64 && packet[2] >= 64){
	  //scanner->strike();
	}
	sustain = packet[2];
	break;
    case(PITCH_BEND):
      bend = packet[2];
      break;
    default:
      lseek(MidiFD, 0, SEEK_END);
      break;
    }
    
  }
  printf("Piano Thread is DEADBEEF\n");
  return 0;
}
