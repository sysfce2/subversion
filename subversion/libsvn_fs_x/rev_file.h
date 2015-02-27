/* rev_file.h --- revision file and index access data structure
 *
 * ====================================================================
 *    Licensed to the Apache Software Foundation (ASF) under one
 *    or more contributor license agreements.  See the NOTICE file
 *    distributed with this work for additional information
 *    regarding copyright ownership.  The ASF licenses this file
 *    to you under the Apache License, Version 2.0 (the
 *    "License"); you may not use this file except in compliance
 *    with the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing,
 *    software distributed under the License is distributed on an
 *    "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 *    KIND, either express or implied.  See the License for the
 *    specific language governing permissions and limitations
 *    under the License.
 * ====================================================================
 */

#ifndef SVN_LIBSVN_FS_X__REV_FILE_H
#define SVN_LIBSVN_FS_X__REV_FILE_H

#include "svn_fs.h"
#include "id.h"

/* In format 7, index files must be read in sync with the respective
 * revision / pack file.  I.e. we must use packed index files for packed
 * rev files and unpacked ones for non-packed rev files.  So, the whole
 * point is to open them with matching "is packed" setting in case some
 * background pack process was run.
 */

/* Opaque index stream type.
 */
typedef struct svn_fs_x__packed_number_stream_t
  svn_fs_x__packed_number_stream_t;

/* Location and content meta data for an index. */
typedef struct svn_fs_x__index_info_t
{
  /* Offset within the pack / rev file at which the index data starts. */
  apr_off_t start;

  /* First offset behind the index data. */
  apr_off_t end;

  /* MD5 checksum on the whole on-disk representation of the index. */
  svn_checksum_t *checksum;

} svn_fs_x__index_info_t;

/* Data file, including indexes data, and associated properties for
 * START_REVISION.  As the FILE is kept open, background pack operations
 * will not cause access to this file to fail.
 */
typedef struct svn_fs_x__revision_file_t
{
  /* first (potentially only) revision in the rev / pack file.
   * SVN_INVALID_REVNUM for txn proto-rev files. */
  svn_revnum_t start_revision;

  /* the revision was packed when the first file / stream got opened */
  svn_boolean_t is_packed;

  /* rev / pack file */
  apr_file_t *file;

  /* stream based on FILE and not NULL exactly when FILE is not NULL */
  svn_stream_t *stream;

  /* the opened P2L index stream or NULL.  Always NULL for txns. */
  svn_fs_x__packed_number_stream_t *p2l_stream;

  /* the opened L2P index stream or NULL.  Always NULL for txns. */
  svn_fs_x__packed_number_stream_t *l2p_stream;

  /* Copied from FS->FFD->BLOCK_SIZE upon creation.  It allows us to
   * use aligned seek() without having the FS handy. */
  apr_off_t block_size;

  /* Info on the L2P index within FILE.
   * Elements are -1 / NULL until svn_fs_x__auto_read_footer gets called. */
  svn_fs_x__index_info_t l2p_info;

  /* Info on the P2L index within FILE.
   * Elements are -1 / NULL until svn_fs_x__auto_read_footer gets called. */
  svn_fs_x__index_info_t p2l_info;

  /* pool containing this object */
  apr_pool_t *pool;
} svn_fs_x__revision_file_t;

/* Open the correct revision file for REV.  If the filesystem FS has
 * been packed, *FILE will be set to the packed file; otherwise, set *FILE
 * to the revision file for REV.  Return SVN_ERR_FS_NO_SUCH_REVISION if the
 * file doesn't exist.  Allocate *FILE in RESULT_POOL and use SCRATCH_POOL
 * for temporaries. */
svn_error_t *
svn_fs_x__open_pack_or_rev_file(svn_fs_x__revision_file_t **file,
                                svn_fs_t *fs,
                                svn_revnum_t rev,
                                apr_pool_t *result_pool,
                                apr_pool_t *scratch_pool);

/* Open the correct revision file for REV with read and write access.
 * If necessary, temporarily reset the file's read-only state.  If the
 * filesystem FS has been packed, *FILE will be set to the packed file;
 * otherwise, set *FILE to the revision file for REV.
 *
 * Return SVN_ERR_FS_NO_SUCH_REVISION if the file doesn't exist.
 * Allocate *FILE in RESULT_POOL and use SCRATCH_POOLfor temporaries. */
svn_error_t *
svn_fs_x__open_pack_or_rev_file_writable(svn_fs_x__revision_file_t **file,
                                         svn_fs_t *fs,
                                         svn_revnum_t rev,
                                         apr_pool_t *result_pool,
                                         apr_pool_t *scratch_pool);

/* Open the proto-rev file of transaction TXN_ID in FS and return it in *FILE.
 * Allocate *FILE in RESULT_POOL use and SCRATCH_POOL for temporaries.. */
svn_error_t *
svn_fs_x__open_proto_rev_file(svn_fs_x__revision_file_t **file,
                              svn_fs_t *fs,
                              svn_fs_x__txn_id_t txn_id,
                              apr_pool_t* result_pool,
                              apr_pool_t *scratch_pool);

/* Wrap the TEMP_FILE, used in the context of FS, into a revision file
 * struct, allocated in RESULT_POOL, and return it in *FILE.
 */
svn_error_t *
svn_fs_x__wrap_temp_rev_file(svn_fs_x__revision_file_t **file,
                             svn_fs_t *fs,
                             apr_file_t *temp_file,
                             apr_pool_t *result_pool);

/* Access functions */

/* Set *STREAM to the shared L2P data stream of FILE.  Initializes the
 * stream on demand.
 */
svn_error_t *
svn_fs_x__rev_file_l2p_index(svn_fs_x__packed_number_stream_t **stream,
                             svn_fs_x__revision_file_t *file);

/* Set *STREAM to the shared P2L data stream of FILE.  Initializes the
 * stream on demand.
 */
svn_error_t *
svn_fs_x__rev_file_p2l_index(svn_fs_x__packed_number_stream_t **stream,
                             svn_fs_x__revision_file_t *file);

/* Copy the L2P index info for FILE into *INFO.
 */
svn_error_t *
svn_fs_x__rev_file_l2p_info(svn_fs_x__index_info_t *info,
                            svn_fs_x__revision_file_t *file);

/* Copy the P2L index info for FILE into *INFO.
 */
svn_error_t *
svn_fs_x__rev_file_p2l_info(svn_fs_x__index_info_t *info,
                            svn_fs_x__revision_file_t *file);

/* Close all files and streams in FILE.
 */
svn_error_t *
svn_fs_x__close_revision_file(svn_fs_x__revision_file_t *file);

#endif
