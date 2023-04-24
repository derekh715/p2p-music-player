#ifndef CHUNKED_FILE_H
#define CHUNKED_FILE_H

#include "util.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

// I am too lazy please forgive me
namespace fs = std::filesystem;

#define DEFAULT_CHUNK_SIZE 16384
/*
 * This class represents a file that can be accessed in chunks
 * For example, if you want to access chunk 15 of the file, get(15, body) can be
 * called to get the file. body will be filled with bytes of size
 * DEFAULT_CHUNK_SIZE
 */
struct ChunkedFile {
  public:
    // chunk_size is measured in bytes. So chunk_size = 1000 is 1000 bytes per
    // chunk
    ChunkedFile(fs::path path, int chunk_size = DEFAULT_CHUNK_SIZE);
    ChunkedFile();
    ~ChunkedFile();

    void open_file(fs::path path, int chunk_size = DEFAULT_CHUNK_SIZE);
    void open_file_with_segment_count(fs::path path, int segment_count);
    void close();

    // randomly get a segment from the file
    // the returned boolean indicates if the data is correcly written into
    // body
    // segment_id is zero based, so the last segment is total_segments - 1
    bool get(int segment_id, std::vector<char> &body);

    // has open file failed?
    bool failure();

    int size;
    int total_segments;
    bool failed;
    int chunk_size = 0;

  private:
    std::ifstream f;
};

#endif
