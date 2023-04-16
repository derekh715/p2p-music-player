#include "store-type.h"

bool operator==(const Track &lhs, const Track &rhs) {
    return (lhs.id == rhs.id && lhs.album == rhs.album &&
            lhs.artist == rhs.artist && lhs.author == rhs.author &&
            lhs.len == rhs.len && lhs.title == rhs.title &&
            lhs.lrcfile == rhs.lrcfile);
}

std::ostream &operator<<(std::ostream &os, const Track &t) {
    os << "For track " << t.id << ":"
       << "\nTitle: " << t.title << "\nAlbum: " << t.album
       << "\nAuthor: " << t.author << "\nArtist: " << t.artist
       << "\nDuration: " << t.len << "\nLyric File" << t.lrcfile << std::endl;
    return os;
}
