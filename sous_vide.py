#!/usr/bin/python3

from urllib.request import urlopen
from json import loads
from os import system
from time import sleep
from time import time
tempErrors = 0
def getTemps(server):
        url = "http://%s/temp" % server
        try:
            r = urlopen(url)
            l = int(r.headers['Content-Length'].split(',')[0])
            content = r.read(l)
        except:
            tempErrors += 1
            sleep(3)
            r = urlopen(url)
            l = int(r.headers['Content-Length'].split(',')[0])
            content = r.read(l)
        temps = loads(content)
        return temps

print('start')
sleep(1)
print('ready')
server = '192.168.1.19'
on = False
ts = time()
maxdt = 60*3
targetT = 50
minoff = 10
system('~/git/pi/lights-off "Stekker B" > /dev/null')
system('~/git/pi/lights-off "Stekker B" > /dev/null')
try:
    while True:
        temps = getTemps(server)
        now = time()
        dt = now - ts
        #print("heating:", on)
        t = temps[0]['temp0']
        if abs(t - targetT) < 3:
            maxdt = 1
            minoff = 20
        if (t > targetT):
            maxdt = 1
        if (t < (targetT -3)) and (t > (targetT -10)):
            maxdt = 5
            minoff = 10
        if t < targetT - 10:
            maxdt = 60
            minoff = 10
        if (on and dt > maxdt) or (t > targetT and on):
            ts = now
            maxdt = 1
            print("turn off", t, dt, maxdt)
            system('~/git/pi/lights-off "Stekker B" > /dev/null')
            system('~/git/pi/lights-off "Stekker B" > /dev/null')
            on = False
        elif t < targetT and not on and dt > minoff:
            ts = now
            print("turn on", t, dt, maxdt)
            system('~/git/pi/lights-on "Stekker B" > /dev/null')
            system('~/git/pi/lights-on "Stekker B" > /dev/null')
            on = True
        else:
            print("heating", on, t, dt, maxdt, minoff, tempErrors)
        sleep(0.3)
except:
    system('~/git/pi/lights-off "Stekker B" > /dev/null')
    system('~/git/pi/lights-off "Stekker B" > /dev/null')
    system('~/git/pi/lights-off "Stekker B" > /dev/null')
    raise
