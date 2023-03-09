#include "store.h"

Store::Store(bool development, std::string filename)
    : db(filename, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE) {
    if (development) {
        db.exec("DROP TABLE tracks");
        std::cout << "Tables dropped!" << std::endl;
    }
    db.exec("CREATE TABLE IF NOT EXISTS tracks ("
            "id INTEGER PRIMARY KEY,"
            "album STRING, artist STRING, author STRING,"
            "title STRING, len INTEGER, creator STRING"
            ")");
    std::cout << "Tables created!" << std::endl;
};