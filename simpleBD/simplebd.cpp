// Simple TCP client
#include <unistd.h>
int main()
{
    execlp("nc"," nc", "-l", "6969", NULL);
}