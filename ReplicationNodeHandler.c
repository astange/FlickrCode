//
//  ReplicationNodeHandler.c
//  
//
//  Created by Michael Dunn on 4/14/15.
//
//

#include "ReplicationNodeHandler.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>


struct dirent *NodeListing;
const char *masterPath;

static int rpfs_mkRep(const char *path, mode_t mode,size_t size,struct fuse_file_info *fi){
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
    rpfs_readdir(path, currDir, int(* fuse_fill_dir_t)(void, const char, const struct stat, off_t), fi);
   
    int index=0;
    while(NodeListing[index] != NULL){
        if(strcmp(currDir[index].d_name,NodeListing[index].d_name)!=0){
            errMsg= rpfs_mkRep(strcat(path, NodeListing[index].d_name), 777,
                               sizeof(NodeListing[index].d_name), fi);
            
        }
    }
    /* TO DO:
        Check err clauses
     */
    return errMsg;
    
}

//recreates nodes, can do up to the needed amount
static int rpfs_init(int needed, const char **paths, char *metaDataCopy,size_t size, struct fuse_file_info *fi){
    for (int i = 0; i < needed; i++) {
        /*TO DO:
         Check failure events
         */
        rpfs_mkRep(paths[i], 777, size, fi);
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
 * mode is 777
 */
static int rpfs_create(const char *path, mode_t mode, struct fuse_file_info *fi){
    int fd;
    fd = creat(path, mode);
    if(fd < 0)
        return -errno;
    return fd;
    
}

/** Write data to an open file
 *
 * Write should return exactly the number of bytes requested
 * except on error.  An exception to this is when the 'direct_io'
 * mount option is specified (see read operation).
 *
 * Changed in version 2.2
 */

int rpfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    int errMsg = 0;
    
    errMsg = pwrite(fi->fh, buf, size, offset);
    if (retstat < 0)
        return -errno;
    
    return errMsg;
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
    if (retstat < 0)
        return -errMsg;
    
    return errMsg;
}

//removes node, basic version
static int rpfs_unlink(const char *path){
    int res;
    res = unlink(path);
    if (res == -1)
        return -errno;
    return 0;
}
//This gives us a list of all files within root directiory, not sure how filler works exactly
/* helpful links: http://sourceforge.net/p/fuse/wiki/Readdir%28%29/ 
 http://sourceforge.net/p/fuse/wiki/Fuse_fill_dir_t%28%29/ */
static int rpfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi){
    DIR *dp;
    struct dirent *de;
    (void) offset;
    (void) fi;
    dp = opendir(path);
    
    /* check if user can open the directory */
    if (dp == NULL)
        return -errno;
    
    /* add all directory entries to buffer */
    while ((de = readdir(dp)) != NULL) {
        if (filler(buf, de->d_name, &st, 0))
            break;
    }
    closedir(dp);
    return 0;
}

//Directory methods
static int rpfs_mkdir(const char *path, mode_t mode){
        int res;
        res = mkdir(path, mode);
        if (res == -1)
                return -errno;
        return 0;
}

static int rpfs_rmdir(const char *path){
        int res;
        res = rmdir(path);
        if (res == -1)
                return -errno;
        return 0;
}


