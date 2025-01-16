/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "inLimbo", "index.html", [
    [ "<strong>inLimbo Music Player - Song Map Flow</strong>", "md_src_2dirsort_2SONGMAP.html", [
      [ "High-Level Flow Overview", "md_src_2dirsort_2SONGMAP.html#autotoc_md2", null ],
      [ "Step-by-Step Detailed Flow", "md_src_2dirsort_2SONGMAP.html#autotoc_md4", [
        [ "1. Inode Mapping with InodeFileMapper", "md_src_2dirsort_2SONGMAP.html#autotoc_md5", null ],
        [ "2. Inode Insertion into Red-Black Tree", "md_src_2dirsort_2SONGMAP.html#autotoc_md7", null ],
        [ "3. In-Order Traversal and Metadata Extraction", "md_src_2dirsort_2SONGMAP.html#autotoc_md9", null ],
        [ "4. Serialization into lib.bin", "md_src_2dirsort_2SONGMAP.html#autotoc_md11", null ],
        [ "5. UI Parsing the Song Map", "md_src_2dirsort_2SONGMAP.html#autotoc_md13", null ]
      ] ],
      [ "Conclusion", "md_src_2dirsort_2SONGMAP.html#autotoc_md15", null ]
    ] ],
    [ "NETWORK", "md_src_2network_2NETWORK.html", null ],
    [ "README", "md_README.html", null ],
    [ "COLORS inLimbo", "md_COLORS.html", [
      [ "Overview", "md_COLORS.html#autotoc_md33", [
        [ "How to Set Colors in config.toml", "md_COLORS.html#autotoc_md34", null ],
        [ "Available Colors and Their RGB Values", "md_COLORS.html#autotoc_md35", null ],
        [ "How to Use Colors in the inLimbo", "md_COLORS.html#autotoc_md36", null ],
        [ "Example of config.toml", "md_COLORS.html#autotoc_md37", null ]
      ] ]
    ] ],
    [ "CHANGELOG", "md_CHANGELOG.html", [
      [ "Format", "md_CHANGELOG.html#autotoc_md40", null ],
      [ "[ALPHA 0.1] — 26-12-2024", "md_CHANGELOG.html#autotoc_md42", [
        [ "Added", "md_CHANGELOG.html#autotoc_md43", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md44", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md45", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md46", null ]
      ] ],
      [ "[ALPHA 0.2] — 26-12-2024", "md_CHANGELOG.html#autotoc_md48", [
        [ "Added", "md_CHANGELOG.html#autotoc_md49", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md50", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md51", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md52", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md53", null ]
      ] ],
      [ "[ALPHA 0.3] — 27-12-2024", "md_CHANGELOG.html#autotoc_md55", [
        [ "Added", "md_CHANGELOG.html#autotoc_md56", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md57", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md58", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md59", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md60", null ]
      ] ],
      [ "[ALPHA 0.4] — 29-12-2024", "md_CHANGELOG.html#autotoc_md62", [
        [ "Added", "md_CHANGELOG.html#autotoc_md63", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md64", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md65", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md66", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md67", null ]
      ] ],
      [ "[ALPHA 0.5] — 31-12-2024", "md_CHANGELOG.html#autotoc_md69", [
        [ "Added", "md_CHANGELOG.html#autotoc_md70", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md71", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md72", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md73", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md74", null ]
      ] ],
      [ "[ALPHA 0.6] — 31-12-2024", "md_CHANGELOG.html#autotoc_md76", [
        [ "Added", "md_CHANGELOG.html#autotoc_md77", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md78", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md79", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md80", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md81", null ]
      ] ],
      [ "[ALPHA 0.7] — 02-01-2025", "md_CHANGELOG.html#autotoc_md83", [
        [ "Added", "md_CHANGELOG.html#autotoc_md84", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md85", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md86", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md87", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md88", null ]
      ] ],
      [ "[ALPHA 0.8] — 03-01-2025", "md_CHANGELOG.html#autotoc_md90", [
        [ "Added", "md_CHANGELOG.html#autotoc_md91", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md92", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md93", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md94", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md95", null ]
      ] ],
      [ "[ALPHA 0.9] — 03-01-2025", "md_CHANGELOG.html#autotoc_md97", [
        [ "Added", "md_CHANGELOG.html#autotoc_md98", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md99", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md100", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md101", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md102", null ]
      ] ],
      [ "[ALPHA 1.0] — 04-01-2025", "md_CHANGELOG.html#autotoc_md104", [
        [ "Added", "md_CHANGELOG.html#autotoc_md105", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md106", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md107", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md108", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md109", null ]
      ] ],
      [ "[ALPHA 1.1] — 05-01-2025", "md_CHANGELOG.html#autotoc_md111", [
        [ "Added", "md_CHANGELOG.html#autotoc_md112", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md113", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md114", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md115", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md116", null ]
      ] ],
      [ "[ALPHA 1.2] — 07-01-2025", "md_CHANGELOG.html#autotoc_md118", [
        [ "Added", "md_CHANGELOG.html#autotoc_md119", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md120", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md121", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md122", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md123", null ]
      ] ],
      [ "[ALPHA 1.3] — 10-01-2025", "md_CHANGELOG.html#autotoc_md125", [
        [ "Added", "md_CHANGELOG.html#autotoc_md126", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md127", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md128", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md129", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md130", null ]
      ] ],
      [ "[ALPHA 1.4] — 11-01-2025", "md_CHANGELOG.html#autotoc_md132", [
        [ "Added", "md_CHANGELOG.html#autotoc_md133", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md134", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md135", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md136", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md137", null ]
      ] ],
      [ "[ALPHA 1.5] — 12-01-2025", "md_CHANGELOG.html#autotoc_md139", [
        [ "Added", "md_CHANGELOG.html#autotoc_md140", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md141", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md142", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md143", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md144", null ]
      ] ],
      [ "[ALPHA 1.6] — 13-01-2025", "md_CHANGELOG.html#autotoc_md146", [
        [ "Added", "md_CHANGELOG.html#autotoc_md147", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md148", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md149", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md150", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md151", null ]
      ] ],
      [ "[ALPHA 1.7] — 13-01-2025", "md_CHANGELOG.html#autotoc_md153", [
        [ "Added", "md_CHANGELOG.html#autotoc_md154", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md155", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md156", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md157", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md158", null ]
      ] ],
      [ "[ALPHA 1.8] — 13-01-2025", "md_CHANGELOG.html#autotoc_md160", [
        [ "Added", "md_CHANGELOG.html#autotoc_md161", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md162", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md163", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md164", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md165", null ]
      ] ],
      [ "[ALPHA 1.9] — 14-01-2025", "md_CHANGELOG.html#autotoc_md167", [
        [ "Added", "md_CHANGELOG.html#autotoc_md168", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md169", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md170", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md171", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md172", null ]
      ] ]
    ] ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", null ],
        [ "Functions", "namespacemembers_func.html", null ],
        [ "Variables", "namespacemembers_vars.html", null ],
        [ "Typedefs", "namespacemembers_type.html", null ],
        [ "Enumerations", "namespacemembers_enum.html", null ]
      ] ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", null ],
        [ "Functions", "functions_func.html", null ],
        [ "Variables", "functions_vars.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Functions", "globals_func.html", null ],
        [ "Variables", "globals_vars.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html",
"md_src_2dirsort_2SONGMAP.html#autotoc_md9",
"structtiv_1_1size.html#acdd4846573496f9ecc1f958e765048c1"
];

var SYNCONMSG = 'click to disable panel synchronization';
var SYNCOFFMSG = 'click to enable panel synchronization';