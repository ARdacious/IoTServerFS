#!/usr/bin/python3

# WS client example

from ws4py.client.threadedclient import WebSocketClient

import requests
from urllib.request import urlopen
from json import loads, dumps
from os import system
from time import sleep
from time import time
import threading
import sys

tempErrors = 0

def getValues(server, verbose, page="temp"):
    _s = time()
    url = "http://%s/%s" % (server, page)
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
    if verbose:
        print(temps)
        print("temp took", _dt)
    return temps, _dt

serverp1 = sys.argv[1]

def getTemp(verbose=False):
    temps, _ = getValues(serverp1, verbose)
    t = temps[0]['temp0']
    if verbose: print("temp", t)
    return t

getTemp(verbose=True)
getValues(serverp1, True, "all")
"""
#url = 'https://docs.google.com/forms/d/e/1FAIpQLSeI3-jkg0oedRXECibaxbGZiFyOHGYjvNiOXcADBK9Qq__gUg/formResponse'
url = 'https://docs.google.com/forms/d/e/1FAIpQLSeA62KILXqeqjV2NJi7Oi7p4mg7UESI4SFGQmliKpmgKAeR4Q/formResponse'
temp = "%s" % getTemp()
form_data = {
    "entry.1711528084":temp,
    "entry.1248773206":'beneden voor',
    "fvv":1,"draftResponse":'[]',"pageHistory":0,"fbzx":-6718008993703688486}
result = requests.post(url, data=form_data)  # response [200]
print(result)
"""


class SousVideClient(WebSocketClient):
    def testing(self):
        # do the thing
        print("starting test")
        ws.send("o")
        ws.send("c")
        for temp in [55,]:
            ws.send("TOOT%s" % temp)
            sleep(15 * 60)
        ws.send("TOOT20")
        ws.send("o")

    def opened(self):
        print("Websocket opened")
        try:
            print("trying to start thread")
            x = threading.Thread(target=SousVideClient.testing, args=(self,))
            x.start()
            print("thread started", x)
        except Exception as e:
            print(e)
    def closed(self, code, reason=None):
        print("Connexion closed down", code, reason)
    def received_message(self, m):
        analog = None
        try :
            m = str(m)
            m = loads(m)
            try:
                # get vallue for analog pin
                analog = getValues(serverp1, False, "all")[0]['analog']
                m.append({"name":"Analog", "value": analog})
                #ws.send('?')
            except Exception as e:
                import traceback
                print(e)
                traceback.print_exc()
            print(m)
            with open("templog.json", "a+") as log:
                log.write(dumps(m)+'\n')
        except Exception as e:
            print("other message:", m, e)
            with open("templog.log", "a+") as log:
                log.write(m+'\n')

if __name__ == '__main__':
    try:
        ws = SousVideClient("ws://sousvide.local:81")
        ws.connect()
        while True:
            sleep(5)
    except:
        print("problem occured")
        ws.close()
