#ifndef STORE_H
#define STORE_H

#include <SQLiteCpp/SQLiteCpp.h>
#include <iostream>

class Store {
  public:
    // initialize / create the database
    // development mode will drop everything
    Store(bool development, std::string filename = "sqlite.db");

  private:
    SQLite::Database db;
};

#endif