/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

.intel_syntax noprefix
.global rdtsc

.balign 16
rdtsc:
		rdtsc
		ret
