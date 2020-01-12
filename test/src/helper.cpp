#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string>

int dir_count(std::string path)
{
    DIR *dp;
    int i = 0;
    struct dirent *ep;     
    dp = opendir (path.c_str());

    if (dp != NULL)
    {
        while ( (ep = readdir (dp)) )
            i++;

        (void) closedir (dp);
    }
    else
        perror ("Couldn't open the directory");
    
    return i-2;
}