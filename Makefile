all:
	gcc interface.c sim.c cJSON.c memory.c utils.c -o interface -lm 
