#include "store-types.h"

bool operator==(const Track &lhs, const Track &rhs) {
    return (lhs.id == rhs.id && lhs.album == rhs.album &&
            lhs.artist == rhs.artist && lhs.title == rhs.title &&
            lhs.lrcfile == rhs.lrcfile && lhs.path == rhs.path &&
            lhs.duration == rhs.duration && lhs.checksum == rhs.checksum &&
            lhs.filesize == rhs.filesize);
}

std::ostream &operator<<(std::ostream &os, const Track &t) {
    os << "For track " << t.id << ":"
       << "\nTitle: " << t.title << "\nAlbum: " << t.album
       << "\nArtist: " << t.artist << "\nDuration: " << t.duration
       << "\nChecksum: " << t.checksum << "\nLyric File: " << t.lrcfile
       << "\nPath: " << t.path << std::endl;
    return os;
}
