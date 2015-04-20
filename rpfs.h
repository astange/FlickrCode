//
//  rpfs.h
//  
//
//  Created by Michael Dunn on 4/19/15.
//
//

#ifndef _rpfs_h
#define _rpfs_h

static int rpfs_writeBackup(mode_t mode, size_t size,struct fuse_file_info *fi);
static int rpfs_checkCrash(struct fuse_file_info *fi);
static int rpfs_init(int backups);

#endif
