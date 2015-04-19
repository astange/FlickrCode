#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/md5.h>
#include <errno.h>
#include <fuse.h>
#include "uthash.h"
#include "python_caller.h"

struct photo {
    char md5string[MD5_DIGEST_LENGTH*2+1];
    unsigned long long id;
    UT_hash_handle hh;
};

struct photo* photos = NULL;

int rpfs_write(const char *path, const char *buf, size_t size, off_t offset,
    struct fuse_file_info *fi) {

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
    for(int i = 0; i < MD5_DIGEST_LENGTH; ++i)
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

    return 0;
}

struct fuse_operations rpfs_oper = {
    .write = rpfs_write
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &rpfs_oper, NULL);
}