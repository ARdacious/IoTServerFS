#!/usr/bin/python3

from urllib.request import urlopen
from json import loads
from os import system

def listFiles(server):
        url = "http://%s/list?dir=/" % server
        r = urlopen(url)
        l = int(r.headers['Content-Length'].split(',')[0])
        content = r.read(l)
        files = loads(content)
        for f in files:
            print(f)
        return files
server = 'p1.local'
for f in listFiles(server):
    if f['type'] == 'file':
        system('wget %s/%s -O %s/%s' % (server, f['name'], './data', f['name']))
        