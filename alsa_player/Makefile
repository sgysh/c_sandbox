TARGET:=alsa_player
CFLAGS:=-Wall

.PHONY : all clean

all : $(TARGET)

alsa_player : alsa_player.c wave_format.h
	$(CC) $< -lasound -o $@

clean :
	rm -f $(TARGET)

