#ifndef CONNECTIVITY_H
#define CONNECTIVITY_H

#include <WebServer.h>

// Make the server object accessible to the Main file
extern WebServer server;

void initWiFi();
void initBLE();

#endif