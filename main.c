#include <stdio.h>
#include "StringCJJ.h"
#include "FtpClientCJJ.h"
#include "FtpClientMenu.h"

int main(int argc, char const *argv[])
{
    const char *url = (argc > 1) ? argv[1] : "ftp://";
    app(url);
    return 0;
}