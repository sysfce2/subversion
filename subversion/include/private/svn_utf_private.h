/**
 * @copyright
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
 * @endcopyright
 *
 * @file svn_utf_private.h
 * @brief UTF validation and normalization routines
 */

#ifndef SVN_UTF_PRIVATE_H
#define SVN_UTF_PRIVATE_H

#include <apr.h>
#include <apr_pools.h>

#include "svn_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* Return TRUE if the string SRC of length LEN is a valid UTF-8 encoding
 * according to the rules laid down by the Unicode 4.0 standard, FALSE
 * otherwise.  This function is faster than svn_utf__last_valid().
 */
svn_boolean_t
svn_utf__is_valid(const char *src, apr_size_t len);

/* As for svn_utf__is_valid but SRC is NULL terminated. */
svn_boolean_t
svn_utf__cstring_is_valid(const char *src);

/* Return a pointer to the first character after the last valid UTF-8
 * potentially multi-byte character in the string SRC of length LEN.
 * Validity of bytes from SRC to SRC+LEN-1, inclusively, is checked.
 * If SRC is a valid UTF-8, the return value will point to the byte SRC+LEN,
 * otherwise it will point to the start of the first invalid character.
 * In either case all the characters between SRC and the return pointer - 1,
 * inclusively, are valid UTF-8.
 *
 * See also svn_utf__is_valid().
 */
const char *
svn_utf__last_valid(const char *src, apr_size_t len);

/* As for svn_utf__last_valid but uses a different implementation without
   lookup tables.  It avoids the table memory use (about 400 bytes) but the
   function is longer (about 200 bytes extra) and likely to be slower when
   the string is valid.  If the string is invalid this function may be
   faster since it returns immediately rather than continuing to the end of
   the string.  The main reason this function exists is to test the table
   driven implementation.  */
const char *
svn_utf__last_valid2(const char *src, apr_size_t len);

const char *
svn_utf__cstring_from_utf8_fuzzy(const char *src,
                                 apr_pool_t *pool,
                                 svn_error_t *(*convert_from_utf8)
                                              (const char **,
                                               const char *,
                                               apr_pool_t *));


/* Fill the given BUFFER with an NFD UCS-4 representation of the UTF-8
 * string STR. If LEN is 0, assume STR is NUL-terminated; otherwise
 * look only at the first LEN bytes in STR. If the returned
 * RESULT_LENGTH is greater than the supplied BUFFER_LENGTH, the the
 * contents of the buffer are indeterminate; otherwise the buffer up
 * to RESULT_LENGTH contains the normalized string representation.
 *
 * A returned error may indicate that STR contains invalid UTF-8 or
 * invalid Unicode codepoints. Any error message comes from utf8proc.
 */
svn_error_t *
svn_utf__decompose_normalized(const char *str, apr_size_t len,
                              apr_int32_t *buffer, apr_size_t buffer_length,
                              apr_size_t *result_length);


/* Compare two arrays of UCS-4 codes, BUFA of length LENA and BUFB of
 * length LENB. Return 0 if they're equal, a negative value if BUFA is
 * greater than BUFB, otherwise a positive value.
 */
int
svn_utf__ucs4cmp(const apr_int32_t *bufa, apr_size_t lena,
                 const apr_int32_t *bufb, apr_size_t lenb);


/* Return the version of the wrapped utf8proc library. */
const char *
svn_utf__utf8proc_version(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SVN_UTF_PRIVATE_H */
