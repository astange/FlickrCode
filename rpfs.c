#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/md5.h>
#include <errno.h>
#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <sys/time.h>
#include <fcntl.h>
#include "uthash.h"
#include "python_caller.h"

static const char *hello_str = "Hello World!\n";
static const char *master_path = "/master.node";
static int hello_getattr(const char *path, struct stat *stbuf)
{
        int res = 0;
        memset(stbuf, 0, sizeof(struct stat));
        if (strcmp(path, "/") == 0) {
                stbuf->st_mode = S_IFDIR | 0777;
                stbuf->st_nlink = 2;
        } else if (strcmp(path, master_path) == 0) {
                stbuf->st_mode = S_IFREG | 0777;
                stbuf->st_nlink = 1;
                stbuf->st_size = strlen(hello_str);
        } else
                res = -ENOENT;
        return res;
}
static int hello_setxattr(const char *path, const char *name, const char *value,
                        size_t size, int flags)
{
        int res = lsetxattr(path, name, value, size, flags);
        if (res == -1)
                return -errno;
        return 0;
}


static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
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

static int hello_truncate (const char * path, off_t size)
{
      /*  int res;
        res = truncate(path + 1, size);
        if (res == -1)
                return -errno;*/
        return 0;
}
static int hello_open(const char *path, struct fuse_file_info *fi)
{
       /* if (strcmp(path, master_path) != 0)
                return -ENOENT;
        if ((fi->flags & 3) != O_RDONLY)
                return -EACCES;*/
        return 0;
}
static int hello_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
        size_t len;
        (void) fi;
        if(strcmp(path, master_path) != 0)
                return -ENOENT;
        len = strlen(hello_str);
        if (offset < len) {
                if (offset + size > len)
                        size = len - offset;
                memcpy(buf, hello_str + offset, size);
        } else
                size = 0;
        return size;
}

static int hello_write(const char *path, const char *buf, size_t size, off_t offset,
    struct fuse_file_info *fi) {

    printf("Hi\n");
    /*struct timeval tstartFull;
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
        if (buf[i]==' ') {
            index = i;
            break;
        }
    }

    char* instr = malloc(index);
    char* filename = malloc(strlen(buf)-index-1);

    memcpy(instr, buf, index);
    memcpy(filename, &buf[index+2], strlen(buf)-index);

    // Check instruction validity
    if (strcmp(instr, "get")!=0 && strcmp(instr, "post")!=0)
        return -1;

    // Check filename path validity, maybe not needed
    for (i=0; i<strlen(filename); i++)
        if (filename[i]=='/')
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
        get(p->id);
    }

    // post - hash - if file exist, md5 hash, python
    if (strcmp(instr, "post")==0) {
        p = (struct photo*) malloc(sizeof(struct photo));
        memcpy(p->md5string, md5string, strlen(md5string));
        //p->md5string = md5string;
        p->id = put(filename);
        HASH_ADD_STR(photos, md5string, p);
    }
    gettimeofday(&tstartFullEnd, NULL);
    printf("Time spent performing entire operation: %4ld seconds and %d microseconds\n", tstartFullEnd.tv_sec - tstartFull.tv_sec, tstartFullEnd.tv_usec - tstartFull.tv_usec);*/
    return size;
}
static struct fuse_operations hello_oper = {
        .getattr        = hello_getattr,
        .readdir        = hello_readdir,
        .open           = hello_open,
        .write          = hello_write,
        .read           = hello_read,
        .setxattr       = hello_setxattr,
        .truncate       = hello_truncate,
};
int main(int argc, char *argv[])
{
        return fuse_main(argc, argv, &hello_oper, NULL);
}

/*
struct photo {
    char md5string[MD5_DIGEST_LENGTH*2+1];
    unsigned long long id;
    UT_hash_handle hh;
};

struct photo* photos = NULL;

static int rpfs_getattr(const char *path, struct stat *stbuf)
{
        if(strcmp(path, "/") != 0){
                printf("BAD\n");
                return -ENOENT;
        }
        stbuf->st_mode = S_IFREG;
        stbuf->st_nlink = 1;
        stbuf->st_uid = getuid();
        stbuf->st_gid = getgid();
        stbuf->st_size = (1ULL << 32); // 4G 
        stbuf->st_blocks = 0;
        stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = time(NULL);
        return 0;
}

static int rpfs_open(const char *path, struct fuse_file_info *fi)
{
        if (strcmp(path, "/") != 0){
                printf("BAD\n");
                return -ENOENT;
        }
        if ((fi->flags & 3) != O_RDONLY)
                return -EACCES;
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
        if (buf[i]==' ') {
            index = i;
            break;
        }
    }

    char* instr = malloc(index);
    char* filename = malloc(strlen(buf)-index-1);

    memcpy(instr, buf, index);
    memcpy(filename, &buf[index+2], strlen(buf)-index);

    // Check instruction validity
    if (strcmp(instr, "get")!=0 && strcmp(instr, "post")!=0)
        return -1;

    // Check filename path validity, maybe not needed
    for (i=0; i<strlen(filename); i++)
        if (filename[i]=='/')
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
        get(p->id);
    }

    // post - hash - if file exist, md5 hash, python
    if (strcmp(instr, "post")==0) {
        p = (struct photo*) malloc(sizeof(struct photo));
        memcpy(p->md5string, md5string, strlen(md5string));
        //p->md5string = md5string;
        p->id = put(filename);
        HASH_ADD_STR(photos, md5string, p);
    }
    gettimeofday(&tstartFullEnd, NULL);
    printf("Time spent performing entire operation: %4ld seconds and %d microseconds\n", tstartFullEnd.tv_sec - tstartFull.tv_sec, tstartFullEnd.tv_usec - tstartFull.tv_usec);
    return size;
}

struct fuse_operations rpfs_oper = {
    .getattr = rpfs_getattr,
    .open = rpfs_open,
    .write = rpfs_write
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &rpfs_oper, NULL);
}*/