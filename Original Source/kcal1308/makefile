wc = wcl386
flags = /c /cc /ms /W3 /3 /fpi /omax /zp1 /bt=NT

kcal.exe: k1.obj eclipse.obj sunmon.obj jd.obj bcd.obj
    wlink system nt name kcal file k1, eclipse, sunmon, jd, bcd

k1.obj: k1.c
        $(wc) $(flags) k1.c

eclipse.obj: eclipse.c
        $(wc) $(flags) eclipse.c

sunmon.obj: sunmon.c
        $(wc) $(flags) sunmon.c

jd.obj: jd.c jd.h bcd.h
        $(wc) $(flags) jd.c

bcd.obj: bcd.c bcd.h
        $(wc) $(flags) bcd.c


