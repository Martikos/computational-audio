all:
	gcc sf2float.c portsf.c ieee80.c -o audiodize -lm
