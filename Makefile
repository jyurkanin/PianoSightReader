


all:
	g++ -o pv piano_window.cpp main.cpp audio_engine.cpp controller.cpp -lX11 -lm -lpthread
