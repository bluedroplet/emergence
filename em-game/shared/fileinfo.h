/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

#ifndef _FILEINFO_H
#define _FILEINFO_H

int get_file_info(char *filename, int *size, uint8_t *hash);
int compare_hashes(uint8_t *hash1, uint8_t *hash2);
#define FILEINFO_DIGEST_SIZE 20

#endif
