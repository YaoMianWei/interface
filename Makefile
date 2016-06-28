all:
	gcc -shared interface.c sim.c mul.c pic.c cJSON.c memory.c utils.c testbase.c -o libMSG.so -lm -ldl -DTEST_DTV_EXPAND -DSO_CNT=1 -lSDL -lSDL_ttf `sdl-config --cflags --libs`
