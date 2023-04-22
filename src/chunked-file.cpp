#include "chunked-file.h"

ChunkedFile::ChunkedFile(fs::path path, int chunk_size) {
    open_file(path, chunk_size);
}

ChunkedFile::ChunkedFile(){};

void ChunkedFile::open_file(fs::path path, int _chunk_size) {
    chunk_size = _chunk_size;
    failed = false;
    size = get_file_size(path);
    std::cout << "size is " << size << std::endl;
    if (size <= 0) {
        failed = true;
        return;
    }
    close();
    // floor division will miss the last incomplete chunk
    // so add one to it
    total_segments = size / chunk_size;
    if (size % chunk_size != 0) {
        total_segments += 1;
    }
    f = std::ifstream(path, std::ios::in | std::ios::binary);
    if (!f.is_open()) {
        failed = true;
        return;
    }
}

bool ChunkedFile::failure() { return failed; }

void ChunkedFile::close() {
    if (f.is_open()) {
        f.close();
    }
}

ChunkedFile::~ChunkedFile() { close(); }

bool ChunkedFile::get(int segment_id, std::vector<char> &body) {
    // segment_id is zero based
    if (failed || segment_id < 0 || segment_id >= total_segments) {
        return false; // segment out of range
    }
    // if we are writing the last segment, chances are the number of bytes
    // are not enough
    int bytes_to_be_read = size - segment_id * chunk_size;
    if (bytes_to_be_read > chunk_size) {
        bytes_to_be_read = chunk_size;
    }
    body.clear();
    body.resize(bytes_to_be_read);
    f.seekg(segment_id * chunk_size, std::ios::beg);
    f.read(body.data(), bytes_to_be_read);
    return true;
}
