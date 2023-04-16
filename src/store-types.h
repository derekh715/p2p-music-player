#ifndef TYPE_H
#define TYPE_H

#include <iostream>
#include <string>

/*
 * Track struct: represents a row in the tracks table
 */
struct Track {
    // stores the auto generated id of sqlite
    // this field does not need to be initialized if you are just creating
    // an instance of track
    int id;
    // the following fields are almost the same as the lrc file headers
    std::string album = "";
    std::string artist = "";
    std::string title = "";
    // the path to the LRC file
    // NOTE: it can be relative or absolute
    // but it will be converted to an absolute path before being stored to the
    // database, also for audio path
    std::string lrcfile = "";
    std::string path = "";
    // the length of the audio file, in milliseconds
    int duration = 0;
    // compute the file checksum before storing it into the database
    std::string checksum = "";
    // file size of the audio file
    int filesize = 0;
    // == can determine if the two track files are the "same"
    // "same" means all fields equal
    friend bool operator==(const Track &lhs, const Track &rhs);
    friend std::ostream &operator<<(std::ostream &os, const Track &t);
};

#endif
