#ifndef __CONFIG_H__
#define __CONFIG_H__

#define MDNS_HOST_NAME "p1"


struct config {
  char m1[255] = "http://192.168.1.4:8888/test1/{val}";
  char m2[255] = "http://192.168.1.4:8888/test2/{val}";
};

extern struct config cfg;

#endif
