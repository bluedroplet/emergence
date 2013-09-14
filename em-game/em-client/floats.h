/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

void gzread_floating_images(gzFile file);
void render_floating_images();
int generate_and_write_scaled_floating_images(gzFile infile, gzFile outfile);
void clear_floating_images();
