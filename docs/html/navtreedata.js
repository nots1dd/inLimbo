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
    [ "README", "md_README.html", [
      [ "Getting Started with inLimbo", "md_README.html#autotoc_md26", [
        [ "Why", "md_README.html#autotoc_md20", null ],
        [ "Features", "md_README.html#autotoc_md21", null ],
        [ "LOOKS", "md_README.html#autotoc_md22", null ],
        [ "DEPENDENCIES", "md_README.html#autotoc_md23", null ],
        [ "GOAL", "md_README.html#autotoc_md24", null ],
        [ "BUILDING", "md_README.html#autotoc_md25", null ],
        [ "Running inLimbo", "md_README.html#autotoc_md27", null ],
        [ "Cache and Directory Management", "md_README.html#autotoc_md28", null ],
        [ "Command-Line Arguments", "md_README.html#autotoc_md29", null ],
        [ "CONFIGURATION", "md_README.html#autotoc_md30", [
          [ "General Navigation", "md_README.html#autotoc_md31", null ],
          [ "Scrolling and Selection", "md_README.html#autotoc_md32", null ],
          [ "Playback Controls", "md_README.html#autotoc_md33", null ],
          [ "Seeking and Volume Control", "md_README.html#autotoc_md34", null ],
          [ "Song and Queue Management", "md_README.html#autotoc_md35", null ],
          [ "Song Information and UI Navigation", "md_README.html#autotoc_md36", null ],
          [ "Status Bar & UI Settings", "md_README.html#autotoc_md37", null ]
        ] ],
        [ "DEBUGGING", "md_README.html#autotoc_md38", null ],
        [ "DIRECTORY SORTING", "md_README.html#autotoc_md39", null ],
        [ "TESTING", "md_README.html#autotoc_md40", null ],
        [ "DOCUMENTATION", "md_README.html#autotoc_md41", null ],
        [ "CREDITS", "md_README.html#autotoc_md42", null ]
      ] ]
    ] ],
    [ "inLimbo Build Guide", "md_BUILD.html", [
      [ "Available Build Targets", "md_BUILD.html#autotoc_md45", [
        [ "all", "md_BUILD.html#autotoc_md46", null ],
        [ "build", "md_BUILD.html#autotoc_md48", null ],
        [ "rebuild", "md_BUILD.html#autotoc_md50", null ],
        [ "asan (AddressSanitizer)", "md_BUILD.html#autotoc_md52", null ],
        [ "asan_run", "md_BUILD.html#autotoc_md54", null ],
        [ "tsan (ThreadSanitizer)", "md_BUILD.html#autotoc_md56", null ],
        [ "tsan_run", "md_BUILD.html#autotoc_md58", null ],
        [ "build-test", "md_BUILD.html#autotoc_md60", null ],
        [ "build-test-all", "md_BUILD.html#autotoc_md62", null ],
        [ "clean", "md_BUILD.html#autotoc_md64", null ],
        [ "init", "md_BUILD.html#autotoc_md66", null ],
        [ "build-global", "md_BUILD.html#autotoc_md68", null ],
        [ "build-global-uninstall", "md_BUILD.html#autotoc_md70", null ],
        [ "verbose", "md_BUILD.html#autotoc_md72", null ]
      ] ],
      [ "MANUAL BUILD:", "md_BUILD.html#autotoc_md74", null ]
    ] ],
    [ "COLORS inLimbo", "md_COLORS.html", [
      [ "Overview", "md_COLORS.html#autotoc_md76", [
        [ "How to Set Colors in config.toml", "md_COLORS.html#autotoc_md77", null ],
        [ "Available Colors and Their RGB Values", "md_COLORS.html#autotoc_md78", null ],
        [ "How to Use Colors in the inLimbo", "md_COLORS.html#autotoc_md79", null ],
        [ "Example of config.toml", "md_COLORS.html#autotoc_md80", null ]
      ] ]
    ] ],
    [ "CHANGELOG", "md_CHANGELOG.html", [
      [ "Format", "md_CHANGELOG.html#autotoc_md83", null ],
      [ "[ALPHA 0.1] — 26-12-2024", "md_CHANGELOG.html#autotoc_md85", [
        [ "Added", "md_CHANGELOG.html#autotoc_md86", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md87", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md88", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md89", null ]
      ] ],
      [ "[ALPHA 0.2] — 26-12-2024", "md_CHANGELOG.html#autotoc_md91", [
        [ "Added", "md_CHANGELOG.html#autotoc_md92", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md93", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md94", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md95", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md96", null ]
      ] ],
      [ "[ALPHA 0.3] — 27-12-2024", "md_CHANGELOG.html#autotoc_md98", [
        [ "Added", "md_CHANGELOG.html#autotoc_md99", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md100", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md101", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md102", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md103", null ]
      ] ],
      [ "[ALPHA 0.4] — 29-12-2024", "md_CHANGELOG.html#autotoc_md105", [
        [ "Added", "md_CHANGELOG.html#autotoc_md106", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md107", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md108", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md109", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md110", null ]
      ] ],
      [ "[ALPHA 0.5] — 31-12-2024", "md_CHANGELOG.html#autotoc_md112", [
        [ "Added", "md_CHANGELOG.html#autotoc_md113", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md114", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md115", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md116", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md117", null ]
      ] ],
      [ "[ALPHA 0.6] — 31-12-2024", "md_CHANGELOG.html#autotoc_md119", [
        [ "Added", "md_CHANGELOG.html#autotoc_md120", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md121", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md122", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md123", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md124", null ]
      ] ],
      [ "[ALPHA 0.7] — 02-01-2025", "md_CHANGELOG.html#autotoc_md126", [
        [ "Added", "md_CHANGELOG.html#autotoc_md127", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md128", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md129", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md130", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md131", null ]
      ] ],
      [ "[ALPHA 0.8] — 03-01-2025", "md_CHANGELOG.html#autotoc_md133", [
        [ "Added", "md_CHANGELOG.html#autotoc_md134", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md135", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md136", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md137", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md138", null ]
      ] ],
      [ "[ALPHA 0.9] — 03-01-2025", "md_CHANGELOG.html#autotoc_md140", [
        [ "Added", "md_CHANGELOG.html#autotoc_md141", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md142", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md143", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md144", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md145", null ]
      ] ],
      [ "[ALPHA 1.0] — 04-01-2025", "md_CHANGELOG.html#autotoc_md147", [
        [ "Added", "md_CHANGELOG.html#autotoc_md148", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md149", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md150", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md151", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md152", null ]
      ] ],
      [ "[ALPHA 1.1] — 05-01-2025", "md_CHANGELOG.html#autotoc_md154", [
        [ "Added", "md_CHANGELOG.html#autotoc_md155", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md156", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md157", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md158", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md159", null ]
      ] ],
      [ "[ALPHA 1.2] — 07-01-2025", "md_CHANGELOG.html#autotoc_md161", [
        [ "Added", "md_CHANGELOG.html#autotoc_md162", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md163", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md164", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md165", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md166", null ]
      ] ],
      [ "[ALPHA 1.3] — 10-01-2025", "md_CHANGELOG.html#autotoc_md168", [
        [ "Added", "md_CHANGELOG.html#autotoc_md169", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md170", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md171", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md172", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md173", null ]
      ] ],
      [ "[ALPHA 1.4] — 11-01-2025", "md_CHANGELOG.html#autotoc_md175", [
        [ "Added", "md_CHANGELOG.html#autotoc_md176", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md177", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md178", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md179", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md180", null ]
      ] ],
      [ "[ALPHA 1.5] — 12-01-2025", "md_CHANGELOG.html#autotoc_md182", [
        [ "Added", "md_CHANGELOG.html#autotoc_md183", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md184", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md185", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md186", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md187", null ]
      ] ],
      [ "[ALPHA 1.6] — 13-01-2025", "md_CHANGELOG.html#autotoc_md189", [
        [ "Added", "md_CHANGELOG.html#autotoc_md190", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md191", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md192", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md193", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md194", null ]
      ] ],
      [ "[ALPHA 1.7] — 13-01-2025", "md_CHANGELOG.html#autotoc_md196", [
        [ "Added", "md_CHANGELOG.html#autotoc_md197", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md198", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md199", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md200", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md201", null ]
      ] ],
      [ "[ALPHA 1.8] — 13-01-2025", "md_CHANGELOG.html#autotoc_md203", [
        [ "Added", "md_CHANGELOG.html#autotoc_md204", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md205", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md206", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md207", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md208", null ]
      ] ],
      [ "[ALPHA 1.9] — 14-01-2025", "md_CHANGELOG.html#autotoc_md210", [
        [ "Added", "md_CHANGELOG.html#autotoc_md211", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md212", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md213", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md214", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md215", null ]
      ] ],
      [ "[ALPHA 2.0] — 16-01-2025", "md_CHANGELOG.html#autotoc_md217", [
        [ "Added", "md_CHANGELOG.html#autotoc_md218", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md219", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md220", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md221", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md222", null ]
      ] ],
      [ "[ALPHA 2.1] — 17-01-2025", "md_CHANGELOG.html#autotoc_md224", [
        [ "Added", "md_CHANGELOG.html#autotoc_md225", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md226", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md227", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md228", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md229", null ]
      ] ],
      [ "[ALPHA 2.2] — 19-01-2025", "md_CHANGELOG.html#autotoc_md231", [
        [ "Added", "md_CHANGELOG.html#autotoc_md232", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md233", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md234", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md235", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md236", null ]
      ] ],
      [ "[ALPHA 2.3] — 22-01-2025", "md_CHANGELOG.html#autotoc_md238", [
        [ "Added", "md_CHANGELOG.html#autotoc_md239", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md240", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md241", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md242", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md243", null ]
      ] ],
      [ "[ALPHA 2.4] — 24-01-2025", "md_CHANGELOG.html#autotoc_md245", [
        [ "Added", "md_CHANGELOG.html#autotoc_md246", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md247", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md248", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md249", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md250", null ]
      ] ],
      [ "[ALPHA 2.5] — 25-01-2025", "md_CHANGELOG.html#autotoc_md252", [
        [ "Added", "md_CHANGELOG.html#autotoc_md253", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md254", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md255", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md256", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md257", null ]
      ] ],
      [ "[ALPHA 2.6] — 26-01-2025", "md_CHANGELOG.html#autotoc_md259", [
        [ "Added", "md_CHANGELOG.html#autotoc_md260", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md261", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md262", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md263", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md264", null ]
      ] ],
      [ "[ALPHA 2.7] — 27-01-2025", "md_CHANGELOG.html#autotoc_md266", [
        [ "Added", "md_CHANGELOG.html#autotoc_md267", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md268", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md269", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md270", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md271", null ]
      ] ],
      [ "[ALPHA 2.8] — 29-01-2025", "md_CHANGELOG.html#autotoc_md273", [
        [ "Added", "md_CHANGELOG.html#autotoc_md274", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md275", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md276", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md277", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md278", null ]
      ] ],
      [ "[ALPHA 2.9] — 01-02-2025", "md_CHANGELOG.html#autotoc_md280", [
        [ "Added", "md_CHANGELOG.html#autotoc_md281", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md282", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md283", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md284", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md285", null ]
      ] ],
      [ "[ALPHA 3.0] — 02-02-2025", "md_CHANGELOG.html#autotoc_md287", [
        [ "Added", "md_CHANGELOG.html#autotoc_md288", null ],
        [ "Changed", "md_CHANGELOG.html#autotoc_md289", null ],
        [ "Fixed", "md_CHANGELOG.html#autotoc_md290", null ],
        [ "Removed", "md_CHANGELOG.html#autotoc_md291", null ],
        [ "Known Issues to fix in immediate commits", "md_CHANGELOG.html#autotoc_md292", null ]
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
        [ "All", "functions.html", "functions_dup" ],
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
        [ "Enumerations", "globals_enum.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html",
"md_CHANGELOG.html#autotoc_md141",
"namespaceTrueColors.html#aeafabc145297298741b798e8fdc52a6dacec7ca178e2f7b1ae3a4600868c57aca",
"structSongTreeState.html#a469567f9eaec41bf0657a301dd5b4101"
];

var SYNCONMSG = 'click to disable panel synchronization';
var SYNCOFFMSG = 'click to enable panel synchronization';