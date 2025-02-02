
#ifndef LRC_H
#define LRC_H

#include <string>
#include <vector>

struct Lyric {
    unsigned int startms; // only have start time in .lrc
    std::string s1;       // lyric with color
    char color = 0;       // 0:M, 1:F, 2:D
    std::string s2 = "";  // lyric without color
    bool operator<(const Lyric &b) const { return startms < b.startms; }
    bool operator==(const Lyric &rhs) const {
        return s1 == rhs.s1 && s2 == rhs.s2 && color == rhs.color &&
               endms == rhs.endms;
    }
    unsigned int endms;
};

struct Lrc {
  public:
    Lrc();
    Lrc(const char *path);
    Lyric getLyric(int millisec);
    int GetLyricIndex(int millisec);
    std::vector<std::string> getAllLyrics();
    static unsigned int sTimetoms(std::string s);
    static std::string
    removeSpace(std::string s); // remove space in the begining and ending
    bool failed(); // this is a getter so that the consumer cannot change this

    std::vector<Lyric> GetAllLyrics();
    std::string album;   //[al:Album where the song is from]
    std::string artist;  //[ar:Lyrics artist]
    std::string author;  //[au:Creator of the Songtext]
    std::string title;   //[ti:Lyrics (song) title]
    std::string length;  //[length:How long the song is]
    std::string creator; //[by:Creator of the LRC file]
    int offset; //[offset:+/- Overall timestamp adjustment in milliseconds, +
                // shifts time up, - shifts down]
    std::string editor;  //[re:The player or editor that created the LRC file]
    std::string version; //[ve:version of program]
    std::vector<Lyric> lys;
    bool failure;

  private:
    void init();
};

#endif
