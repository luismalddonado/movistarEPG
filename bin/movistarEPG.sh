#!/bin/sh

logger "movistarEPG start..."

./movistarEPG
cd ../data
rm -rf lgarrido_xmltv-movistar-spain-c.xml.gz
gzip lgarrido_xmltv-movistar-spain-c.xml
#cp lgarrido_xmltv-movistar-spain-c.xml.gz /tmp/mnt/DISCO/optware/share/nginx/html/EPG

logger "movistarEPG END!"
