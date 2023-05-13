// open, creat, read, write, close, stat, mkdir, opendir, closedir, readdir, strerror, errno, exit

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define BLOCK_SIZE 4096

char buffer[BLOCK_SIZE];

int filecopy(const char *source_file, const char *target_file)
{
    int bytes = -1;
    int fsrc = open(source_file, O_RDONLY);
    if (fsrc < 0)
    {
        fprintf(stderr, "filecopy: Unable to open %s: %s\n", source_file, strerror(errno));
        return bytes;
    }

    if (!access(target_file, F_OK))
    {
        fprintf(stderr, "filecopy: couldn`t create file %s: File exists\n", target_file);
        goto close_src;
    }

    int ftar = open(target_file, O_WRONLY | O_CREAT, S_IRUSR);
    if (ftar < 0)
    {
        fprintf(stderr, "filecopy: Unable to create %s: %s\n", target_file, strerror(errno));
        goto close_src;
    }

    bytes = 0;
    int nr, nw;
    while (1)
    {
        nr = read(fsrc, buffer, BLOCK_SIZE);
        if (nr < 0)
        {
            fprintf(stderr, "filecopy: fail to read file %s: %s\n", source_file, strerror(errno));
            bytes = -1;
            goto close_tar;
        }
        else if (nr == 0)
            break;
        else
        {
            bytes += nr;
            char *ptr = buffer;
            while (nr)
            {
                nw = write(ftar, ptr, nr);
                if (nw < 0)
                {
                    fprintf(stderr, "filecopy: fail to write file %s: %s\n", target_file, strerror(errno));
                    bytes = -1;
                    goto close_tar;
                }
                nr -= nw;
                ptr += nw;
            }
        }
    }

close_tar:
    close(ftar);
close_src:
    close(fsrc);

    return bytes;
}

int main(int argc, char **argv)
{
    // check arguments
    if (argc != 3)
    {
        if (argc < 3)
            printf("filecopy: Too fews arguments!\n");
        else
            printf("filecopy: Too many arguments!\n");
        printf("usage: filecopy <sourcefile> <targetfile>\n");
        return 0;
    }
    int bytes;
    if ((bytes = filecopy(argv[1], argv[2])) != -1)
    {
        printf("treecopy: copied %d %s from %s to %s\n", 
            bytes, bytes > 1 ? "bytes" : "byte", argv[1], argv[2]);
        return 0;
    }

    return 1;
}