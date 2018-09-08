CC:=gcc
CFLAGS:=-Wall
TARGET:=hugepage

.PHONY : all clean

all : $(TARGET)

$(TARGET) : hugepage.c
	$(CC) $(CFLAGS) $< -o $@ -lhugetlbfs

clean :
	-rm $(TARGET)

