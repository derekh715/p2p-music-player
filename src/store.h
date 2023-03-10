#ifndef STORE_H
#define STORE_H

#include <SQLiteCpp/SQLiteCpp.h>
#include <iostream>
#include <vector>

struct Track {
    int id;
    std::string album;
    std::string artist;
    std::string author;
    std::string title;
    std::string lrcfile;
    int len;
    friend bool operator==(const Track &lhs, const Track &rhs);
};

class Store {
  public:
    // initialize / create the database
    // development mode will drop everything
    Store(bool development, std::string filename = "sqlite.db");

    // add one track into the database
    bool create(Track &t);

    // get one track from the database
    // return value:
    // 1. returns a normal track if it is in the database
    // 2. returns a track with id -1 if it is NOT in the database
    // all other fields are uninitialized
    Track read(int id);

    // get all tracks from the database
    std::vector<Track> read_all();

    // update one track
    bool update(int id, Track &t);

    // delete one track (can't use the word delete in C++)
    bool remove(int id);

    // search with text
    std::vector<Track> search(std::string str);

  private:
    SQLite::Database db;
    // helper function that turns a column into Track struct
    // the consumer should executeStep first
    void populate_track_from_get_column(SQLite::Statement &q, Track &t);
    bool get_absolute_file(std::string name, std::filesystem::path &path);
};

#endif
