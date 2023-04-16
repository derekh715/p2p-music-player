#include "store.h"
#include "SQLiteCpp/Statement.h"
#include "util.h"
#include <cstdio>
#include <filesystem>
#include <system_error>

Store::Store(bool drop_all, std::string filename)
    : db(filename, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE) {
    if (drop_all) {
        db.exec("DROP TABLE IF EXISTS tracks");
        std::cout << "Tables dropped!" << std::endl;
    }
    db.exec("CREATE TABLE IF NOT EXISTS tracks ("
            "id INTEGER PRIMARY KEY,"
            "album STRING, artist STRING, title STRING,"
            "lrcfile STRING, path INTEGER, duration INTEGER,"
            "checksum STRING, filesize INTEGER"
            ")");
    std::cout << "Tables created!" << std::endl;
}
bool Store::create(Track &t, bool strict) {

    SQLite::Statement q(db, "INSERT INTO tracks (album, artist, title, "
                            "lrcfile, path, duration, checksum, filesize) "
                            "VALUES (:album, :artist, :title, :lrcfile, :path, "
                            ":duration, :checksum, :filesize)");
    q.bind(":album", t.album);
    q.bind(":artist", t.artist);
    q.bind(":title", t.title);

    std::filesystem::path abs;
    if (!t.lrcfile.empty()) {
        if (!get_absolute_file(t.lrcfile, abs)) {
            if (strict) {
                return false;
            }
            q.bind(":lrcfile", "");
        }
        q.bind(":lrcfile", abs.string());
    }

    if (!t.path.empty()) {
        if (!get_absolute_file(t.path, abs)) {
            if (strict) {
                return false;
            }
            q.bind(":path", "");
        } else {
            q.bind(":path", abs.string());
        }
    }
    q.bind(":duration", t.duration);
    // compute it!!!
    q.bind(":checksum", t.checksum);
    q.bind(":filesize", t.filesize);
    int nrows = q.exec();
    return nrows ==
           1; // something is wrong if zero rows or more rows are affected
}

bool Store::get_absolute_file(std::string name, std::filesystem::path &path) {
    std::error_code ec;
    std::filesystem::path rel = std::filesystem::path(name);
    path = std::filesystem::canonical(rel, ec);
    // that absolute path does not exist in the system
    if (ec.value() != 0) {
        // probably won't do this in the graphical interface...
        std::cerr << ec.message() << std::endl;
        return false;
    }
    return true;
}

Track Store::read(int id) {
    SQLite::Statement q(db, "SELECT * FROM tracks "
                            "WHERE id = :id");
    q.bind(":id", id);
    bool has_row = q.executeStep();
    if (!has_row) {
        Track t = {.id = -1};
        return t;
    }
    Track t;
    populate_track_from_get_column(q, t);
    return t;
}

void Store::populate_track_from_get_column(SQLite::Statement &q, Track &t) {
    int id = q.getColumn("id");
    const char *album = q.getColumn("album");
    const char *artist = q.getColumn("artist");
    const char *title = q.getColumn("title");
    const char *lrcfile = q.getColumn("lrcfile");
    const char *path = q.getColumn("path");
    int duration = q.getColumn("duration");
    const char *checksum = q.getColumn("checksum");
    int filesize = q.getColumn("filesize");

    t.id = id;
    t.album = std::string(album);
    t.artist = std::string(artist);
    t.title = std::string(title);
    t.lrcfile = std::string(lrcfile);
    t.duration = duration;
    t.checksum = std::string(checksum);
    t.filesize = filesize;
}

std::vector<Track> Store::read_all() {
    std::vector<Track> tracks;
    SQLite::Statement q(db, "SELECT * FROM tracks");
    while (q.executeStep()) {
        Track t;
        populate_track_from_get_column(q, t);
        tracks.push_back(t);
    }
    return tracks;
}

bool Store::update(int id, Track &t, bool strict) {
    std::filesystem::path abs;
    if (!t.lrcfile.empty()) {
        if (!get_absolute_file(t.lrcfile, abs)) {
            return false;
        }
    }
    SQLite::Statement q(db, "UPDATE tracks "
                            "SET "
                            "album = :album,"
                            "artist = :artist,"
                            "title = :title,"
                            "lrcfile = :lrcfile,"
                            "path = :path,"
                            "duration = :duration,"
                            "checksum = :checksum,"
                            "filesize = :filesize "
                            "WHERE id = :id");
    q.bind(":album", t.album);
    q.bind(":artist", t.artist);
    q.bind(":title", t.title);

    if (!t.lrcfile.empty()) {
        if (!get_absolute_file(t.path, abs)) {
            if (strict) {
                return false;
            }
            q.bind(":lrcfile", "");
        } else {
            q.bind(":lrcfile", abs.string());
        }
    }

    if (!t.path.empty()) {
        if (!get_absolute_file(t.path, abs)) {
            if (strict) {

                return false;
            }
            q.bind(":path", "");
        }
        q.bind(":path", abs.string());
    }
    q.bind(":duration", t.duration);
    // compute it!!!
    q.bind(":checksum", t.checksum);
    q.bind(":filesize", t.filesize);
    q.bind(":id", id);
    int nrows = q.exec();
    return nrows == 1;
}

bool Store::remove(int id) {
    SQLite::Statement q(db, "DELETE FROM tracks "
                            "WHERE id = :id");
    q.bind(":id", id);
    int nrows = q.exec();
    return nrows == 1;
};

std::vector<Track> Store::search(std::string str) {
    std::vector<Track> tracks;
    SQLite::Statement q(db, "SELECT * FROM tracks "
                            "WHERE album LIKE :album OR "
                            "artist LIKE :artist OR "
                            "title LIKE :title");
    char *sandwiched = new char[str.size() + 15];
    snprintf(sandwiched, str.size() + 15, "%%%s%%", str.c_str());
    q.bind(":album", sandwiched);
    q.bind(":artist", sandwiched);
    q.bind(":title", sandwiched);
    delete[] sandwiched;
    while (q.executeStep()) {
        Track t;
        populate_track_from_get_column(q, t);
        tracks.push_back(t);
    }
    return tracks;
}
