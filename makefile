LDFLAGS = -lGL -lGLU -lglut


a4 : a4.c graphics.c visible.c graphics.h
	gcc a4.c graphics.c visible.c -o a4 $(LDFLAGS) -lm

