#include "piano_window.h"
#include "audio_engine.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>


Display *dpy;
Window w;
GC gc;
pthread_t w_thread;

int is_window_open_ = 0;

void breakerbreaker(){}

int is_window_open(){
  return is_window_open_;

}

void get_string(char* number){ //read the keyboard for a string
    XEvent e;
    char buf[2];
    KeySym ks = 0;
    int count = 0;
    do{
        if(XPending(dpy) > 0){
            XNextEvent(dpy, &e);
            if(e.type == KeyPress){
                XLookupString(&e.xkey, buf, 1, &ks, NULL);
                if(ks == 0xFF0D){
                    number[count] = 0;
                    return;
                }
                number[count] = buf[0];
                XDrawString(dpy, w, gc, (14+count)*8, 100, buf, 1);
                XFlush(dpy);
                count++;
            }
        }
    } while(1);
}


/*
 * Special Cases required for sharps and flats, as well as notes above and below staff. :(
 */
void draw_note_treble(unsigned midi_pitch_num, unsigned pos){
  XSetForeground(dpy, gc, 0x0000FF);
  unsigned note_pos_x = SCREEN_WIDTH - 32 - pos*32; //Move note sideways
  unsigned middle_c_y = -2  + (SCREEN_HEIGHT/2) - 200 + 40;

  //                      0    1    2  3   4  5   6  7  8  9  10 11  12  13 14  15   16 17   18  19 20  21  22  23  24  25  26  27 28   29  30  31
  //                      F    F#   G  Ab  A  Bb  B  C  C# D  Eb E   F   F# G   Ab   A  Bb   B   C  C#   D  Eb   E   F  F#   G  Ab  A   Bb  B   C
  int note_map_y[32] = {-16, -16, -12, -8,-8, -4,-4, 0, 0, 4, 8, 8, 12, 12, 16, 20, 20, 24, 24, 28, 28, 32, 36, 36, 40, 40, 44, 48, 48, 52, 52, 56};
  int accidental[32] = {0,     1,   0, -1, 0, -1, 0, 0, 1, 0,-1, 0,  0,  1, 0,  -1,  0, -1,  0,  0, 1,   0, -1,  0,  0,  1,  0, -1, 0,  -1, 0,  0};
  int bar_lines[32] =  {-3,   -3,  -2, -2,-2, -1,-1,-1,-1, 0, 0, 0,  0,  0, 0,   0,  0,  0,  0,  0, 0,   0,  0,  0,  0,  0,  0,  1, 1,   1, 1,  2};
  
  unsigned note_idx = midi_pitch_num-60+7;
  unsigned note_pos_y = middle_c_y - note_map_y[note_idx];
  
  
  
  //Draw Sharp or flat
  if(accidental[note_idx] == 1){ //sharp
    XDrawString(dpy, w, gc, note_pos_x-8, note_pos_y + 6, "#",1);
  }
  else if(accidental[note_idx] == -1){
    XDrawString(dpy, w, gc, note_pos_x-8, note_pos_y + 6, "b",1);
  }

  //draw extra bar lines
  switch(bar_lines[note_idx]){
  case -3:
    XDrawLine(dpy, w, gc, note_pos_x-4, middle_c_y+19, note_pos_x+9, middle_c_y+19);
    XDrawLine(dpy, w, gc, note_pos_x-4, middle_c_y+11, note_pos_x+9, middle_c_y+11);
    XDrawLine(dpy, w, gc, note_pos_x-4, middle_c_y+3, note_pos_x+9, middle_c_y+3);
    break;
  case -2:
    XDrawLine(dpy, w, gc, note_pos_x-4, middle_c_y+11, note_pos_x+9, middle_c_y+11);
    XDrawLine(dpy, w, gc, note_pos_x-4, middle_c_y+3, note_pos_x+9, middle_c_y+3);
    break;
  case -1:
    XDrawLine(dpy, w, gc, note_pos_x-4, middle_c_y+3, note_pos_x+9, middle_c_y+3);
    break;
  case 0:
    break;
  case 1:
    XDrawLine(dpy, w, gc, note_pos_x-4, middle_c_y-45, note_pos_x+9, middle_c_y-45);
    break;
  case 2:
    XDrawLine(dpy, w, gc, note_pos_x-4, middle_c_y-45, note_pos_x+9, middle_c_y-45);
    XDrawLine(dpy, w, gc, note_pos_x-4, middle_c_y-53, note_pos_x+9, middle_c_y-53);
    break;
  }
  
  //Draw Actual Note and stem
  XDrawLine(dpy, w, gc, note_pos_x+5, note_pos_y, note_pos_x+5, note_pos_y-10);
  XFillArc(dpy, w, gc, note_pos_x, note_pos_y, 6, 6, 0, 360*64);
}

unsigned generate_next_note(){
  unsigned low_f = 53;
  return low_f + floorf(32.0f*rand()/RAND_MAX); //generate all notes from low F, to high C
}


void draw_level(unsigned level){
    char number[10];
    sprintf(number, "Level: %d", level);
    XDrawString(dpy, w, gc, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 64, number, strlen(number));  
  
}

void draw_treble_clef(){
  XSegment segs[5];

  int staff_line_separation = 8;

  for(int i = 0; i < 5; i++){
    segs[i].x1 = 0;
    segs[i].x2 = SCREEN_WIDTH;
    segs[i].y1 = (SCREEN_HEIGHT/2)-200 + staff_line_separation*i;
    segs[i].y2 = (SCREEN_HEIGHT/2)-200 + staff_line_separation*i;
  }
  
  XSetForeground(dpy, gc, 0xFF0000);  
  XDrawSegments(dpy, w, gc, segs, 5);
}

void* window_thread(void*){
  int should_clear = 1;
  
  unsigned max_notes = 100;
  unsigned note_list[max_notes];
  char in_string[100];
  
  memset(note_list, 0,  sizeof(unsigned)*max_notes);
    
  XSetBackground(dpy, gc, 0);
  while(1){
    printf("Beginning Round\n");
    XSetForeground(dpy, gc, 0x0000FF);
    XDrawString(dpy, w, gc, SCREEN_WIDTH/2, SCREEN_HEIGHT/2, "Ready?", 6);
    XFlush(dpy);
    get_string(in_string);
    
    int has_wrong_note = 0;
    int num_notes = 0;
    float counter = 0;
    float step = .01; //update every 10 seconds.
    unsigned correct_notes = 0;
    unsigned level = 0;
    unsigned char *notes_pressed = get_notes_pressed();
    
    for(unsigned i = 0; i < 0xFF; i++){
      notes_pressed[i] = 0;
    }
    
    while(!has_wrong_note){
      XClearWindow(dpy, w);
      draw_treble_clef();
      
      draw_level(level);


      step = .01*(1 + .1*level);
      counter += step;
      if(counter >= 1){
        counter = 0;
        
        note_list[num_notes] = generate_next_note();
        num_notes++;
      }
      
      for(unsigned i = 0; i < num_notes; i++){
        unsigned idx = num_notes - i;
        draw_note_treble(note_list[i], counter+idx);
      }

      notes_pressed = get_notes_pressed();
      if(num_notes > 0){
        if(notes_pressed[note_list[0]]){
          notes_pressed[note_list[0]] = 0;

          correct_notes++;
          if(correct_notes == 10){
            correct_notes = 0;
            level++;
          }
          
          for(int i = 0; i < (num_notes-1); i++){
            note_list[i] = note_list[i+1];
          }
          
          num_notes--;
        }
        else{
          for(unsigned i = 0; i < 0xFF; i++){
            if(notes_pressed[i]){
              XDrawString(dpy, w, gc, SCREEN_WIDTH/2, SCREEN_HEIGHT/2+ 16, "Wrong Note", 10);
              has_wrong_note = 1;
              break;
            }
          }
        }
      }

      if(num_notes > 50){
        XDrawString(dpy, w, gc, SCREEN_WIDTH/2, SCREEN_HEIGHT/2+ 32, "Too Slow", 8);
        break;
      }
      
      XFlush(dpy);
      usleep(10000); //update 100 times a second.
    }
  }
  return 0;
}


void init_window(){  
    dpy = XOpenDisplay(0);
    w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0);

    Atom wm_state   = XInternAtom (dpy, "_NET_WM_STATE", true );
    Atom wm_fullscreen = XInternAtom (dpy, "_NET_WM_STATE_FULLSCREEN", true );
    XChangeProperty(dpy, w, wm_state, XA_ATOM, 32, PropModeReplace, (unsigned char *)&wm_fullscreen, 1);
    
    XSelectInput(dpy, w, StructureNotifyMask | ExposureMask | KeyPressMask | ButtonPressMask);
    XClearWindow(dpy, w);
    XMapWindow(dpy, w);
    gc = XCreateGC(dpy, w, 0, 0);
    
    XEvent e;
    do{
        XNextEvent(dpy, &e);        
    } while(e.type != MapNotify);
    
    is_window_open_ = 1;
    //    pthread_create(&w_thread, NULL, &window_thread, NULL);    
}



void del_window(){
  //pthread_join(w_thread, 0);
    XDestroyWindow(dpy, w);
    XCloseDisplay(dpy);
    is_window_open_ = 0;
    return;
}

