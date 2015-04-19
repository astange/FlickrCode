//
//  ReplicationNodeHandler.h
//  
//
//  Created by Michael Dunn on 4/14/15.
//
//

#ifndef ____ReplicationNodeHandler__
#define ____ReplicationNodeHandler__

#include <stdio.h>
#include <fuse.h>

static int rpfs_mkRep(const char *path, mode_t mode,size_t size,struct fuse_file_info *fi);
static int rpfs_checkCrash(const char *path, struct fuse_file_info *fi);
static int rpfs_init(int needed, const char **paths, char *metaDataCopy,size_t size, struct fuse_file_info *fi);

struct fuse_operations rpfs_oper = {
    .create = rpfs_create,
    .mkdir = rpfs_mkdir,
    .unlink = rpfs_unlink,
    .rmdir = rpfs_rmdir,
    .open = rpfs_open,
    .read = rpfs_read,
    .write = rpfs_write,
    .readdir = rpfs_readdir
};

#endif /* defined(____ReplicationNodeHandler__) */


