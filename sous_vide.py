#!/usr/bin/python3

from urllib.request import urlopen
from json import loads
from os import system
from time import sleep
from time import time
tempErrors = 0
def getTemps(server):
    _s = time()
    url = "http://%s/temp" % server
    try:
        r = urlopen(url)
        l = int(r.headers['Content-Length'].split(',')[0])
        content = r.read(l)
    except:
        global tempErrors
        tempErrors += 1
        sleep(3)
        r = urlopen(url)
        l = int(r.headers['Content-Length'].split(',')[0])
        content = r.read(l)
    temps = loads(content)
    _dt = time()-_s
    #print("temp took", _dt)
    return temps, _dt

def getTemp(verbose=False):
    temps, _ = getTemps('192.168.1.9')
    t = temps[0]['temp0']
    if verbose: print("temp", t)
    return t

def neededWattsH(dT, vol):
    # 1,160 W per celcius per liter
    return vol * 4 * dT / 3.412

def neededTime(watthrs, p):
    return watthrs/p*3600

def Off():
    _s = time()
    system('~/git/pi/lights-off "Stekker B" > /dev/null')
    _dt = time()-_s
    print("off took", _dt)
    return time()-_s
def On():
    _s = time()
    system('~/git/pi/lights-on "Stekker B" > /dev/null')
    _dt = time()-_s
    print("on took", _dt)
    return time()-_s

def waitForPeak():
    _s = time()
    start = getTemp()
    t = start
    print('wait rise')
    while t <= (start + 0.13):
        t = getTemp()
        print (t, start-t)
    print('rise start took:', time()-_s)
    _s = time()
    print('wait peak')
    maxT = t
    while t >= (maxT - 0.13):
        maxT = max(t, maxT)
        t = getTemp()
        print (t, maxT-t)
    print('peak took: ', time()-_s)
    # return peak temperature
    return maxT
    
Off();

volume = 0.75
targetT = 65
tAct = targetT
while True:
    t = getTemp(verbose=False)
    watthrs = neededWattsH(targetT - t, volume)
    dT = neededTime(watthrs, 1800)
    print("temp:", t, "time needed:", dT, "error:", tAct-targetT)
    if dT > 0.6:
        print(watthrs, dT)
        On()
        sleep(dT)
        Off()
        Off()
        tAct = waitForPeak()

exit()

print('start')
sleep(1)
print('ready')
server = '192.168.1.9'
on = False
ts = time()
maxdt = 60*3
targetT = 50
minoff = 10
system('~/git/pi/lights-off "Stekker B" > /dev/null')
system('~/git/pi/lights-off "Stekker B" > /dev/null')
try:
    while True:
        temps, deltaT = getTemps(server)
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
            Off()
            Off()
            on = False
        elif t < targetT and not on and dt > minoff:
            ts = now
            print("turn on", t, dt, maxdt)
            On()
            On()
            on = True
        else:
            print("heating", on, t, dt, maxdt, minoff, tempErrors)
        sleep(0.3)
except:
    system('~/git/pi/lights-off "Stekker B" > /dev/null')
    system('~/git/pi/lights-off "Stekker B" > /dev/null')
    system('~/git/pi/lights-off "Stekker B" > /dev/null')
    raise
