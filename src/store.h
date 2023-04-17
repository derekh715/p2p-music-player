#ifndef STORE_H
#define STORE_H

#include "md5.h"
#include "store-types.h"
#include "util.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <system_error>
#include <vector>

/*
 * Store class: provides a convenient way to store things to or take things from
 * the database. Underneath this class it executes the SQL query. There is no
 * need to touch the implementation while you are using this class.
 *
 * PROCEDURES:
 * 1. create a database instance first
 * Store s(false, "name.db");
 * see the constructor function for more details
 *
 * 2. use the functions for storing tracks, updating tracks etc.
 * Here shows an example of creting a track and saving it
 * Track t = {
 *  .name = "Whatever"
 * };
 * s.create(t);
 *
 * All other methods are used similarly, with different return types.
 */
class Store {
  public:
    // initialize the database
    // drop_all: should we drop all tables?
    // filename: which file should sqlite use?
    Store(bool drop_all, std::string filename = "sqlite.db");

    // add one track into the database
    // return value indicates if it is successful or not
    bool create(Track &t, bool strict = false);

    // bool create_from_music_info(MusicInfoCDT &m);

    // get one track from the database
    // return value:
    // 1. returns a normal track if it is in the database
    // 2. If it is not in the database, still returns a track,
    // but with an id of -1, all other fields are uninitialized
    Track read(int id);

    // get all tracks from the database
    // returns a vector full of tracks
    // if there is nothing in the datbase, the vector is empty
    std::vector<Track> read_all();

    // update one track
    // return a boolean indicating if it is successful or not
    bool update(int id, Track &t, bool strict = false);

    // delete one track (can't use the word delete in C++)
    // return a boolean indicating if it is successful or not
    bool remove(int id);

    // search with text
    // returns a vector of search results
    // if nothing is matched, the vector is empty
    std::vector<Track> search(std::string str);

  private:
    // the database handle for executing each query
    // it should not be accessed outside this class
    SQLite::Database db;
    // helper function that turns a column into Track struct
    // the consumer should executeStep first
    void populate_track_from_get_column(SQLite::Statement &q, Track &t);
    // get absolute path of a file
    bool get_absolute_file(std::string name, std::filesystem::path &path);
};

#endif
