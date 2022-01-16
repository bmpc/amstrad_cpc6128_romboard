10 MEMORY &7FFF
20 addr = &8000
30 INPUT "Select ROM [14, 15]";n
40 POKE addr, n
50 addr=&8010:FOR i=0 to 34:READ a$:POKE addr+i,VAL("&"+a$):NEXT
60 CALL &8010
70 DATA 21,00,80,4e,cd,0f,b9,cd,00,b9,0e,ff,21,00,c0,7e,c5,e5,26,00,6f,11,76,80,cd,43,80,21,76,80,cd,6c,80,e1,c1,3e,2c,cd,5a,bb,23,0d,79,fe,00,20,e0,cd,03,b9,c9,01,f0,d8,cd,5b,80,01,18,fc,cd,5b,80,01,9c,ff,cd,5b,80,0e,f6,cd,5b,80,48,3e,2f,3c,09,38,fc,ed,42,fe,30,20,02,3e,20,12,13,c9,7e,fe,ff,c8,23,cd,5a,bb,18,f6,30,30,30,30,30,ff
