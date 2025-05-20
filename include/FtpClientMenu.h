#ifndef FTP_MENU_H
#define FTP_MENU_H

#include "FtpClientCJJ.h"

bool readYN();

void printMenu(FTPClient *client);
void configure(FTPClient *client);
bool handleMenu(FTPClient *client);
void app(const char *URL);

#endif