/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   cutend_png;
    const int            cutend_pngSize = 26502;

    extern const char*   cutstart_png;
    const int            cutstart_pngSize = 26375;

    extern const char*   delete_png;
    const int            delete_pngSize = 6344;

    extern const char*   edit_png;
    const int            edit_pngSize = 21718;

    extern const char*   en_png;
    const int            en_pngSize = 11108;

    extern const char*   folder_png;
    const int            folder_pngSize = 10190;

    extern const char*   openfile_png;
    const int            openfile_pngSize = 20048;

    extern const char*   play_png;
    const int            play_pngSize = 63331;

    extern const char*   playstop_png;
    const int            playstop_pngSize = 8158;

    extern const char*   rename_png;
    const int            rename_pngSize = 2307;

    extern const char*   save_png;
    const int            save_pngSize = 19067;

    extern const char*   saveasnew_png;
    const int            saveasnew_pngSize = 21476;

    extern const char*   scissors_png;
    const int            scissors_pngSize = 42577;

    extern const char*   share_png;
    const int            share_pngSize = 4217;

    extern const char*   speaker_png;
    const int            speaker_pngSize = 11457;

    extern const char*   stop_png;
    const int            stop_pngSize = 54798;

    extern const char*   zh_png;
    const int            zh_pngSize = 8700;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 17;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
