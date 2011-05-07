bin_PROGRAMS=hidspx

data_DATA=hidspx.ini fuse.txt

hidspx_SOURCES=avrspx.c hwctrl.c utils.c hidasp.c usbasp.c hwctrl.h avrspx.h usbasp.h hidasp.h

install-exec-hook:
	chmod 4755 $(bindir)/hidspx

