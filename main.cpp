#include "audio_engine.h"
#include "controller.h"
#include "piano_window.h"


int main(int argc, char *argv[]){
  init_window();
  init_midi(argc, argv);
  window_thread(NULL);
  del_window();
  del_midi();
}
