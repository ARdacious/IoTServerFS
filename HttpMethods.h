#ifndef __HTTP_METHODS_H__
#define __HTTP_METHODS_H__

#include <ESP8266WebServer.h>

extern ESP8266WebServer server;

void listFiles();

void httpRequest(char *url);

void httpPost(char *url, char *msg);


void handleFileDelete();

void handleFileCreate();

void handleFileList();

void handleFileUpload();

bool handleFileRead(String path);

String getContentType(String filename);

String formatBytes(size_t bytes);

#endif
