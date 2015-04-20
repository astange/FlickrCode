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

struct photo {
    char md5string[MD5_DIGEST_LENGTH*2+1];
    unsigned long long id;
    UT_hash_handle hh;
};

struct photo* photos = NULL;
static const char *master_path = "/master.node";
const char **nodeListing;


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
    char* filename = malloc(strlen(buf)-index-1);

    memcpy(instr, buf, index);
    memcpy(filename, &buf[index+1], strlen(buf)-index);

    printf("%s\n", instr);
    printf("%s\n", filename);

    // Check instruction validity
    if (strcmp(instr, "get")!=0 && strcmp(instr, "post")!=0)
        return -ENOENT;

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
        if(!p){
            return -ENOENT;
        }
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
/* michael's shitty code*/
static int rpfs_mkRep(const char *path, mode_t mode, size_t size,struct fuse_file_info *fi){
    int erMsg;
    erMsg = rpfs_create(path, mode);
    if(erMsg == -errno){
        return -errno;
    }
    char *metaDataCopy;
    rpfs_read(masterPath, metaDataCopy, size, offset, fi);
    rpfs_write(path, metaDataCopy, size, fi);

    return erMsg;
}

static int rpfs_checkCrash(const char *path, struct fuse_file_info *fi){
    int errMsg;
    DIR *dp = opendir(path);
    if(dp == NULL){
        return -errno;
    }
    /*fills list of directory entries. Dirents can be seen at: http://pubs.opengroup.org/onlinepubs/007908775/xsh/dirent.h.html*/
    struct dirent *currDir;

    int index=0;
    while(NodeListing[index].d_name != NULL){
        if(strcmp(currDir[index].d_name,NodeListing[index].d_name)!=0){
            errMsg= rpfs_mkRep(strcat(path, NodeListing[index].d_name),S_IFREG|0777,
                               sizeof(NodeListing[index].d_name), fi);

        }
    }
    /* TO DO:
     Check err clauses
     */
    return errMsg;

}

//creates nodes
static int rpfs_init(int needed, size_t size, struct fuse_file_info *fi){
    int i = 0;
    for(i; i<needed; i++){
        strcpy(nodeListing[i], "/BackupNode_");
        strcat(nodeListing[i], i);
    }
    i = 0;
    for (i; i < needed; i++) {
        /*TO DO:
         Check failure events
         */
        rpfs_mkRep(nodeListing[i], S_IFREG | 0777, size, fi);
    }
    return 0;
}

/**
 * Create and open a file
 *
 * If the file does not exist, first create it with the specified
 * mode, and then open it.
 *
 * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the mknod() and open() methods
 * will be called instead.
 *
 * Introduced in version 2.5
 *
 * mode is 0777 | S_IFREG (if regular file)
 */
static int rpfs_create(const char *path, mode_t mode, struct fuse_file_info *fi){
    int fd;
    fd = creat(path, mode);
    if(fd < 0)
        return -errno;
    return fd;

}

/** Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes.  An exception to this is when the
 * 'direct_io' mount option is specified, in which case the return
 * value of the read system call will reflect the return value of
 * this operation.
 *
 * Changed in version 2.2
 */

int rpfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    int errMsg = 0;
    errMsg = pread(fi->fh, buf, size, offset);
    if (errMsg < 0)
        return -errMsg;

    return errMsg;
}

static struct fuse_operations rpfs_oper = {
        .getattr        = rpfs_getattr,
        .readdir        = rpfs_readdir,
        .open           = rpfs_open,
        .write          = rpfs_write,
        .setxattr       = rpfs_setxattr,
        .truncate       = rpfs_truncate,
        .create         = rpfs_create,
        .read           = rpfs_read
};
int main(int argc, char *argv[])
{
        return fuse_main(argc, argv, &rpfs_oper, NULL);
}