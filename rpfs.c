#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/md5.h>
#include <errno.h>
#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "uthash.h"
#include "python_caller.h"

struct photo {
    char md5string[MD5_DIGEST_LENGTH*2+1];
    unsigned long long id;
    UT_hash_handle hh;
};

struct photoBackup 
{
    unsigned long long id;
};

struct photo* photos = NULL;

static const char *master_path = "/master.node";
int nodesAlive[10];

static int createBackup()
{
    int i;
    char * name = "/tmp/backup/backup_";
    for(i = 0; i < 10; i++)
    {
        char filename[100];
        sprintf(filename, "%s%d", name, i);
        FILE *f = fopen(filename, "w");
        fclose(f);
        nodesAlive[i] = 1;
    }
    return 0;
}

static int copy_file(char *old_filename, char  *new_filename)
    {
        FILE  *ptr_old, *ptr_new;
        int err = 0, err1 = 0;
        int  a;

        ptr_old = fopen(old_filename, "rb");
        ptr_new = fopen(new_filename, "wb");

        if(ptr_old == 0)
            return  -1;

        if(ptr_new == 0)
        {
            fclose(ptr_old);
            return  -1;
        }

        while(1)
        {
            a  = fgetc(ptr_old);

            if(!feof(ptr_old))
                fputc(a, ptr_new);
            else
                break;
        }

        fclose(ptr_new);
        fclose(ptr_old);
        return  0;
    }

static int stillAlive()
{
    int i;
    char * name = "/tmp/backup/backup_";
    for(i = 0; i < 10; i++)
    {
        char filename[100];
        sprintf(filename, "%s%d", name, i);
        if(access(filename,F_OK) == 0)
        {
            if(nodesAlive[i] == 0)
            {
                printf("Erasing!\n");
                int otherIndex = 0;
                int j;
                for(j = 0; j < 10; j++)
                {
                    if(nodesAlive[j] == 1)
                    {
                        otherIndex = j;
                        break;
                    }
                }
                char filename2[100];
                sprintf(filename2, "%s%d", name, otherIndex);
                remove(filename);
                copy_file(filename2,filename);
            }
            nodesAlive[i] = 1;
        }else
        {
            nodesAlive[i] = 0;
        }
    }
}

static int checkValue(struct photo * p)
{
    stillAlive();
    int i,j;
    int agree = 0;
    int disagree = 0;
    char * name = "/tmp/backup/backup_";
    for(i = 0; i < 10; i++)
    {
        if(nodesAlive[i])
        {
            char filename[100];
            sprintf(filename, "%s%d", name, i);
            FILE *f = fopen(filename, "rb+");
            fseek(f, 0L, SEEK_END);
            long sz = ftell(f);
            fseek(f, 0L, SEEK_SET);
            struct photoBackup* photosNodes = malloc(sz);
            int agreed = 0;
            fread(photosNodes, sizeof(photosNodes[0]), sz/sizeof(photosNodes[0]),f);
            for(j = 0; j < sz/sizeof(photosNodes[0]); j++)
            {
                if(p->id == photosNodes[j].id)
                {
                    agree++;
                    agreed = 1;
                    break;
                }
            }
            if(!agreed)
            {
                disagree++;
            }
            free(photosNodes);
            fclose(f);
        }
    }
    if(agree >= disagree)
    {
        return 0;
    }
    else
    {
        return -1;
    }
 
}

static int putValue(struct photo *p)
{
    stillAlive();
    int i;
    long sz = 0;
    int agree = 0;
    int disagree = 0;
    char * name = "/tmp/backup/backup_";
    for(i = 0; i < 10; i++)
    {
        if(nodesAlive[i])
        {
            char filename[100];
            sprintf(filename, "%s%d", name, i);
            FILE *f = fopen(filename, "a");
            fwrite(&(p->id),sizeof(p->id), 1, f);
            fclose(f);
        }
    }
    return 0;
}

static int rpfs_getattr(const char *path, struct stat *stbuf)
{
        int res = 0;
        memset(stbuf, 0, sizeof(struct stat));
        if (strcmp(path, "/") == 0) {
                stbuf->st_mode = S_IFDIR | 0777;
                stbuf->st_nlink = 2;
        } else if (strcmp(path, master_path) == 0) {
                stbuf->st_mode = S_IFREG | 0777;
                stbuf->st_nlink = 1;
                stbuf->st_size = 1000;
        } else
                res = -ENOENT;
        return res;
}

static int rpfs_setxattr(const char *path, const char *name, const char *value,
                        size_t size, int flags)
{
        int res = lsetxattr(path, name, value, size, flags);
        if (res == -1)
                return -errno;
        return 0;
}


static int rpfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
        (void) offset;
        (void) fi;
        if (strcmp(path, "/") != 0)
                return -ENOENT;
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        filler(buf, master_path + 1, NULL, 0);
        return 0;
}

static int rpfs_truncate (const char * path, off_t size)
{
      /*  int res;
        res = truncate(path + 1, size);
        if (res == -1)
                return -errno;*/
        return 0;
}
static int rpfs_open(const char *path, struct fuse_file_info *fi)
{
       /* if (strcmp(path, master_path) != 0)
                return -ENOENT;
        if ((fi->flags & 3) != O_RDONLY)
                return -EACCES;*/
        return 0;
}

static int rpfs_write(const char *path, const char *buf, size_t size, off_t offset,
    struct fuse_file_info *fi) {

    struct timeval tstartFull;
    struct timeval tstartFullEnd;
    gettimeofday(&tstartFull,NULL);

    // Check path validity
    int i;
    for (i=0; i<strlen(path); i++) {
        if (i==0 && path[i]!='/')
            return -ENOENT;
        if (i!=0 && path[i]=='/')
            return -ENOENT;
    }

    // Extract instruction and filename
    int index = 0;
    for (i=0; i<strlen(buf);i++) {
        if (buf[i]=='t') {
            index = i + 1;
            break;
        }
    }

    char* instr = malloc(index);
    char* filename = malloc(strlen(buf)-index);

    memcpy(instr, buf, index);
    memcpy(filename, &buf[index+1], strlen(buf)-index);
    i = 0;
    while(filename[i] != ' ')
    {
        i++;
    }
    filename[i] = '\0';
    printf("%s\n", instr);
    printf("%s\n", filename);

    // Check instruction validity
    if (strcmp(instr, "get")!=0 && strcmp(instr, "post")!=0)
        return -ENOENT;

    // Hash
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5_CTX context;
    MD5_Init(&context);
    MD5_Update(&context, filename, strlen(filename));
    MD5_Final(digest, &context);

    // MD5 to String
    char md5string[MD5_DIGEST_LENGTH*2+1];
    for(i = 0; i < MD5_DIGEST_LENGTH; ++i)
        sprintf(&md5string[i*2], "%02x", (unsigned int)digest[i]);

    struct photo* p;
    // get - check hash - python get
    if (strcmp(instr, "get")==0) {
        HASH_FIND_STR(photos, md5string, p);
        if(p != NULL)
        {
            if(checkValue(p) == 0)
            {
                get(p->id);
            }else
            {
                printf("Nodes did not agree!\n");
            }
        }
        else
        {
            printf("File does not exist!\n");
        }
    }

    // post - hash - if file exist, md5 hash, python
    if (strcmp(instr, "post")==0) {
        p = (struct photo*) malloc(sizeof(struct photo));
        memcpy(p->md5string, md5string, strlen(md5string));
        //p->md5string = md5string;
        p->id = put(filename);
        putValue(p);
        HASH_ADD_STR(photos, md5string, p);
    }
    gettimeofday(&tstartFullEnd, NULL);
    printf("Time spent performing entire operation: %4ld seconds and %d microseconds\n", tstartFullEnd.tv_sec - tstartFull.tv_sec, tstartFullEnd.tv_usec - tstartFull.tv_usec);
    return size;
}

static struct fuse_operations rpfs_oper = {
        .getattr        = rpfs_getattr,
        .readdir        = rpfs_readdir,
        .open           = rpfs_open,
        .write          = rpfs_write,
        .setxattr       = rpfs_setxattr,
        .truncate       = rpfs_truncate,
};
int main(int argc, char *argv[])
{
        createBackup();
        return fuse_main(argc, argv, &rpfs_oper, NULL);
}