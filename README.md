# movistarEPG
movistartEPG is a Movistar+ Electronic Programming Guide written in C. It downloads EPG from official [Movistar+](https://www.movistarplus.es/programacion-tv) website. With the dowloaded data it generates the EPG data in xmltv format. THis data can be easily integrated as a rytec source.  There are some example rytec configurations in the ./data folder.

## Dependencies ##

[inih ](https://github.com/benhoyt/inih#compile-time-options)


## How to compile ##

### Dreambox ###
To install gcc in a dreambox read  [this](https://dreambox.de/board/index.php?thread/21525-dm900-build-essential/). Later needed dependencies (libxml2, glib2.0 and libcurl) are needed

INIH:
g++ -DINI_MAX_LINE=2000 -c ini.h
g++ -DINI_MAX_LINE=2000 -c ini.c -o ini.o

MOVISTAR EPG:
g++ -Wall  -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/libxml2  -I/usr/include/curl  -I/media/hdd/desarrollo/inih /media/hdd/desarrollo/inih/inih/ini.o  -lcurl -lxml2 -lglib-2.0 movistarEPG.h movistarEPG.c  -o movistarEPG 

### Ubuntu ###
g++ -Wall movistarEPG.c movistarEPG.h -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/libxml2 ./inih-master/ini.o   -lcurl -lxml2 -lglib-2.0 -o movistarEPG

### Asuswrt with Optware ###
INIH:
g++ -Wl,--dynamic-linker=/opt/lib/ld-uClibc.so.1 -Wl,-rpath=/opt/lib -L/opt/lib -DINI_MAX_LINE=2000 -c ini.h
g++ -Wl,--dynamic-linker=/opt/lib/ld-uClibc.so.1 -Wl,-rpath=/opt/lib -L/opt/lib -DINI_MAX_LINE=2000 -c ini.c -o ini.o


MOVISTAR EPG:
g++ -Wl,--dynamic-linker=/opt/lib/ld-uClibc.so.1 -Wl,-rpath=/opt/lib -L/opt/lib ./inih-master/ini.o -I/mnt/sda1/optware-ng.arm/include/libxml2 -I/mnt/sda1/optware-ng.arm/lib/glib-2.0/include/  -I/mnt/sda1/optware-ng.arm/include/glib-2.0/ -lcurl -lxml2 -lglib-2.0  movistarEPG.c movistarEPG.h -o movistarEPG




