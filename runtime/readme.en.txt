DirImage
Plugin for Total Commander 5.51 or higher, adds the ability to quickly view directories with images in the form of a thumbnails grid.
Works both in "thumbnail view" mode and through the internal lister.

DirImage is licensed under the GNU Lesser General Public License:
https://www.gnu.org/licenses/lgpl.html

Plugin homepage:
https://github.com/SkyBlackHub/DirImage

Detailed description:
The plugin only works with folders. When called, it scans the target folder (optionally checking subfolders) looking for image files that have valid extensions (see extensions).
The images found in the folder are displayed page by page in the form of a grid of thumbnails with a size of columns x rows.

All configuration is done through the DirImage.ini configuration file in the plugin folder.
INI file structure:
[general]
Base plugin settings.

language
Menu language. Available languages: en - English, ru - Russian.

log
Logging level, useful for debugging. Available levels:
none - disable logging
error - display only error messages
info - also informational messages
debug - all messages
Default value: none

log_file
Message output file. If not specified, messages will be output to standard std::cerr and std::cout.

[view]
[thumbs]
[common]
These sections contain the display settings. The [view] section is responsible for working in the lister mode, and the [thumbs] section is responsible for working in the thumbnail view mode.
The [common] section can contain common settings for these two sections.
For example, if you set rows=3 in [common] and do not set this setting in other sections, then the value 3 will be used for them.
If set in the [view] section rows=2, then this value will be used for it.

enabled
Enable/disable operation in this mode.
Valid values: true, false
Default value: true

extensions
A list of file extensions, separated by a space, that will be loaded.
At the moment, allowed all image types (including the popular jpg, png, gif, bmp, tga and psd) that are supported by the FreeImage library.
A complete list can be found at: http://freeimage.sourceforge.net/features.html and in the documentation: http://freeimage.sourceforge.net/documentation.html
Default value: jpg jpeg gif png bmp gif webp

columns
The number of images shown in the grid horizontally.
Default value: 2

rows
The number of images shown in the grid vertically.
Default value: 2

background
The background color, can be set by three digits separated by a space in RGB format (0 0 0 - 255 255 255).
Default value: 0 0 0

enlarge
Whether or not to enlarge images that are smaller than the current size of one cell in the grid.
The current cell size is equal to the size of the window/thumbnail divided by the number of images in it (ie rows x columns).
In other words cell's width = window's width / columns and cell's height = window's height / rows.
Valid values: true, false
Default value: false

ignore_dots
Ignore ".." pseudodirectories.
Useful if you need to quickly view information about a directory with images (i.e. do not use DirImage).
Valid values: true, false
Default value: true

filter
The filter to use when scaling images.
At the moment, all filters that are supported by the FreeImage library are valid. You can read more about these filters in the documentation at: http://freeimage.sourceforge.net/documentation.html
The box filter is the fastest and least quality of all. However, when reducing the image, it gives quite acceptable quality. If additonally enlarge is turned off, then this filter is recommended.
Valid values: box, bicubic, bilinear, bspline, catmullrom, lanczos3
Default value: box

adaptive
Responsive grid size. If the number of images per page is less than the area of the grid, then its size will be reduced to avoid voids.
Default value: true

transparency_grid
Show checkerboard as a background for translucent images.
Default value: true

pad_h
pad_v
Distance in pixels between thumbnails in the grid horizontally and vertically
Default value: 0, 0

shift
Skip the specified number of thumbnails from the start when displaying.
Convenient when, for example, you do not need to show the first image.
Default value: 0

files_limit
The maximum number of checked files after which the search in the directory stops.
Useful to prevent freezes on directories with too many files.
Default value: 1000

deep_scan
Enable deep look into subdirectories if nothing is found in the parent directory.
Default value: false

deep_scan_level
The nesting level of subdirectories that can be descended to in search of images.
Default value: 1

deep_scan_limit
The maximum number of images that can be loaded from each subdirectory.
Default value: 1

deep_scan_files_limit
Same as files_limit, but for subdirectories.
It makes sense to use small values to avoid freezes on large directories.
Default value: 100

info
Show informational message (current page, current images range, images count, etc.)
Default value: 1

info_color
The text color of the informational message, specified by three digits separated by a space in RGB format (0 0 0 - 255 255 255).
Default value: 0 255 0

info_size
The text size of the informational message.
Default value: 16