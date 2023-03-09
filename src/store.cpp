#include "store.h"

bool operator==(const Track &lhs, const Track &rhs) {
    return (lhs.id == rhs.id && lhs.album == rhs.album &&
            lhs.artist == rhs.artist && lhs.author == rhs.author &&
            lhs.len == rhs.len && lhs.title == rhs.title);
}

Store::Store(bool development, std::string filename)
    : db(filename, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE) {
    if (development) {
        db.exec("DROP TABLE IF EXISTS tracks");
        std::cout << "Tables dropped!" << std::endl;
    }
    db.exec("CREATE TABLE IF NOT EXISTS tracks ("
            "id INTEGER PRIMARY KEY,"
            "album STRING, artist STRING, author STRING,"
            "title STRING, len INTEGER"
            ")");
    std::cout << "Tables created!" << std::endl;
}
bool Store::create(Track &t) {
    SQLite::Statement q(
        db, "INSERT INTO tracks (album, artist,  author, title, len) "
            "VALUES (:album, :artist, :author, :title, :len)");
    q.bind(":album", t.album);
    q.bind(":artist", t.artist);
    q.bind(":author", t.author);
    q.bind(":title", t.title);
    q.bind(":len", t.len);
    int nrows = q.exec();
    return nrows ==
           1; // something is wrong if zero rows or more rows are affected
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
    const char *author = q.getColumn("author");
    const char *title = q.getColumn("title");
    int len = q.getColumn("len");

    t.id = id;
    t.album = std::string(album);
    t.artist = std::string(artist);
    t.author = std::string(author);
    t.title = std::string(title);
    t.len = len;
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

bool Store::update(int id, Track &t) { return false; }
bool Store::remove(int id) { return false; };
