# movistarEPG
movistartEPG is a Movistar+ Electronic Programming Guide written in C. It downloads EPG from official [Movistar+](https://comunicacion.movistarplus.es/wp-admin/admin-post.php) website.

## Dependencies ##

[inih ](https://github.com/benhoyt/inih#compile-time-options)


COMPILE:

COMPILE INIH:
g++ -DINI_MAX_LINE=2000 -c ini.h
g++ -DINI_MAX_LINE=2000 -c ini.c -o ini.o


COMPILAR EN DREAMBOX:
g++ -Wall  -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/libxml2  -I/usr/include/curl  -I/media/hdd/desarrollo/inih /media/hdd/desarrollo/inih/inih/ini.o  -lcurl -lxml2 -lglib-2.0 movistarEPG.h movistarEPG.c  -o movistarEPG 

COMPILAR UBUNTU:
g++ -Wall movistarEPG.c movistarEPG.h -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/libxml2 ./inih-master/ini.o   -lcurl -lxml2 -lglib-2.0 -o movistarEPG

COMPILAR ASUS ROUTER:
g++ -Wl,--dynamic-linker=/opt/lib/ld-uClibc.so.1 -Wl,-rpath=/opt/lib -L/opt/lib ./inih-master/ini.o -I/mnt/sda1/optware-ng.arm/include/libxml2 -I/mnt/sda1/optware-ng.arm/lib/glib-2.0/include/  -I/mnt/sda1/optware-ng.arm/include/glib-2.0/ -lcurl -lxml2 -lglib-2.0  movistarEPG.c movistarEPG.h -o movistarEPG

g++ -Wl,--dynamic-linker=/opt/lib/ld-uClibc.so.1 -Wl,-rpath=/opt/lib -L/opt/lib ./inih-master/ini.o -I/mnt/DISCO/optware/include/libxml2 -I/mnt/DISCO/optware/lib/glib-2.0/include/  -I/mnt/DISCO/optware/include/glib-2.0/ -lcurl -lxml2 -lglib-2.0  movistarEPG.c movistarEPG.h -o movistarEPG

COMPILE INIH:
g++ -Wl,--dynamic-linker=/opt/lib/ld-uClibc.so.1 -Wl,-rpath=/opt/lib -L/opt/lib -DINI_MAX_LINE=2000 -c ini.h
g++ -Wl,--dynamic-linker=/opt/lib/ld-uClibc.so.1 -Wl,-rpath=/opt/lib -L/opt/lib -DINI_MAX_LINE=2000 -c ini.c -o ini.o



