// open, creat, read, write, close, stat, mkdir, opendir, closedir, readdir, strerror, errno, exit

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

#define BLOCK_SIZE 4096

char buffer[BLOCK_SIZE];

int directories, files, bytes;

void filecopy(const char *source_file, const char *target_file)
{
    int fsrc = open(source_file, O_RDONLY);
    if (fsrc < 0)
    {
        fprintf(stderr, "filecopy: Unable to open %s: %s\n", source_file, strerror(errno));
        exit(1);
    }

    if (!access(target_file, F_OK))
    {
        fprintf(stderr, "filecopy: couldn`t create file %s: File exists\n", target_file);
        close(fsrc);
        exit(1);
    }

    int ftar = open(target_file, O_WRONLY | O_CREAT, S_IRUSR);
    if (ftar < 0)
    {
        fprintf(stderr, "filecopy: Unable to create %s: %s\n", target_file, strerror(errno));
        close(fsrc);
        exit(1);
    }

    int nr, nw;
    while (1)
    {
        nr = read(fsrc, buffer, BLOCK_SIZE);
        if (nr < 0)
        {
            fprintf(stderr, "filecopy: fail to read file %s: %s\n", source_file, strerror(errno));
            close(fsrc);
            close(ftar);
            exit(1);
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
                    close(fsrc);
                    close(ftar);
                    exit(1);
                }
                nr -= nw;
                ptr += nw;
            }
        }
    }

    close(ftar);
    close(fsrc);
}

void treecopy(const char *src_dir, const char *dst_dir)
{
    // check src_dir stat
    struct stat st;

    if (stat(src_dir, &st) == -1)
    {
        fprintf(stderr, "treecopy: fail to stat %s: %s\n", src_dir, strerror(errno));
        exit(1);
    }
    if (S_ISREG(st.st_mode))
    {
        filecopy(src_dir, dst_dir);
        ++files;
    }

    // recursive copy directory
    else if (S_ISDIR(st.st_mode))
    {
        DIR *streamp;
        struct dirent *dep;

        // open dir
        if (!(streamp = opendir(src_dir)))
        {
            fprintf(stderr, "treecopy: fail to open dir %s: %s\n", src_dir, strerror(errno));
            exit(1);
        }
        // make new dir
        if (mkdir(dst_dir, S_IRWXU))
        {
            fprintf(stderr, "treecopy: fail to mkdir %s: %s\n", dst_dir, strerror(errno));
            exit(1);
        }

        while ((dep = readdir(streamp)) != NULL)
        {
            if (strcmp(dep->d_name, ".") && strcmp(dep->d_name, ".."))
            {
                char src_name[512], dst_name[512];
                memset(src_name, 0, 512);
                memset(dst_name, 0, 512);
                sprintf(src_name, "%s/%s", src_dir, dep->d_name);
                sprintf(dst_name, "%s/%s", dst_dir, dep->d_name);
                treecopy(src_name, dst_name);
            }
        }

        // close dir
        if (closedir(streamp))
        {
            fprintf(stderr, "treecopy: fail to close dir %s: %s\n", src_dir, strerror(errno));
            exit(1);
        }
        ++directories;
    }
    else
    {
        fprintf(stderr, "treecopy: %s is not either a file or a directory\n", src_dir);
        exit(1);
    }
}

int main(int argc, char **argv)
{
    // check arguments
    if (argc != 3)
    {
        if (argc < 3)
            printf("treecopy: Too fews arguments!\n");
        else
            printf("treecopy: Too many arguments!\n");
        printf("usage: treecopy <sourcefile> <targetfile>\n");
        return 0;
    }
    treecopy(argv[1], argv[2]);
    printf("treecopy: copied %d %s, %d %s, and %d %s from %s to %s\n",
           directories, directories > 1 ? "directories" : "directory",
           files, files > 1 ? "files" : "file", bytes, bytes > 1 ? "bytes" : "byte",
           argv[1], argv[2]);

    return 0;
}