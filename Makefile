all:
	gcc --share interface.c sim.c mul.c pic.c cJSON.c memory.c utils.c -o libinterface.so -lm 
