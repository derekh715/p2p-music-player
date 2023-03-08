#include "lrc.h"
#include "util.h"
#include <fstream>
#include <algorithm>
#include <iostream>

Lrc::Lrc(const char *path){
    // init
    album = "";
    artist = "";
    author = "";
    title = "";
    length = "";
    creator = "";
    offset = 0;
    editor = "";
    version = "";
    lys.clear();
    lys.push_back({0, ""}); // no lyric since 0 ms

    std::ifstream fin(path);
    while(!fin.eof()){
        std::string s;
        std::getline(fin, s);
        rtrim(s);
        int pos = s.find(":");

        // maybe empty string
        if(pos == -1)
            continue;

        // header
        std::string substring = s.substr(pos+1, s.rfind("]")-pos-1);
        switch(s.at(pos-1)){
            case 'l': album = substring; break;     // al
            case 'r': artist = substring; break;    // ar
            case 'u': author = substring; break;    // au
            case 'i': title = substring; break;     // ti
            case 'h': length = substring; break;    // length
            case 'y': creator = substring; break;   // by
            case 't': offset = std::stoi(substring); break; // offset
            case 'e':
                if(s.at(pos-1)=='r')
                    editor = substring;     // re
                else
                    version = substring;    // ve
                break;
        }

        // not timestamp(header) -> next line
        if(s.at(pos-1)<'0' || s.at(pos-1)>'9')
            continue;

        // extract (maybe multiple) [timestamp]
        std::vector<std::string> time(0);
        int openPos = s.find("[");
        while(openPos != -1){       // !=-1 -> have pos for "["
            int closePos = s.find("]"); // assume "[", "]" are in pairs
            time.push_back(s.substr(openPos+1, closePos-openPos-1));
            if(closePos+1 < s.size()){
                s = removeSpace(s.substr(closePos+1));
            }else   // if no char after ']'
                s = "";
            openPos = s.find("[");
        }   // e.g. s: [] D: ?<>?<>? => D: ?<>?<>?

        // Simple format extended (color: M, F, D)
        char color;
        if(s.length() >= 3 && s[1]==':' && s[2]==' '){
            switch(s[0]){
                case 'M':       // "M: "
                    color = 0;
                    s = s.substr(3);
                    break;
                case 'F':       // "F: "
                    color = 1;
                    s = s.substr(3);
                    break;
                case 'D':       // "D: "
                    color = 2;
                    s = s.substr(3);
                    break;
                default: color = lys.back().color;  // "?: "
            }   // e.g. s: D: ?<>?<>? => ?<>?<>?
        }else   // doesn't have "M: " etc.
            color = lys.back().color;

        // not Enhanced LRC format -> process each timestamp -> next line
        if(s.find("<")==-1){
            // s += '\0';
            for(int i = 0; i < time.size(); i++){
                unsigned int ms = sTimetoms(time[i]);
                if(ms>=0)
                    lys.push_back({ms, s, color});
                else
                    continue;
            }
            continue;   // next line;
        }

        // Enhanced LRC (assume only one [timestamp])
        // split <timestamp> and word
        openPos = s.find("<");  //?<>?<>?
        std::vector<std::string> words(0);
        while(openPos!=-1){
            words.push_back(removeSpace(s.substr(0,openPos)));
            s = s.substr(openPos);  //<>?<>?
            openPos = s.find("<");  // update openPos after shorten s
            int closePos = s.find(">");
            time.push_back(s.substr(openPos+1, closePos-openPos-1));
            if(closePos+1 < s.size()){
                s = removeSpace(s.substr(closePos+1));
            }else   // if no char after '>'
                s = "";             //?<>?
            openPos = s.find("<");
        }
        words.push_back(removeSpace(s));
        while(time.size()>words.size()) // keep size the same
            words.push_back("");

        for(int i=0; i<time.size(); i++){
            unsigned int ms = sTimetoms(time[i]);
            std::string s1(""), s2("");
            for(int j=0; j<words.size(); j++)
                if(j<=i)
                    s1 += words[j] + (j==words.size()-1 ? "" : " ");
                else
                    s2 += words[j] + (j==words.size()-1 ? "" : " ");
            lys.push_back({ms, s1, color, s2});
        }
        // lys.back().s1 += '\0';
    }
    fin.close();

    std::stable_sort(lys.begin(), lys.end());

    for(int i=1; i<lys.size(); i++) // lys[0] is empty
        if(lys[i].ms-offset)  // in case resulted ms<0
            lys[i].ms -= offset;

    // for(int i=0; i<lys.size(); i++)
    //     printf("%d %s %d %s\n",lys[i].ms, lys[i].s1.c_str(), lys[i].color, lys[i].s2.c_str());
}

unsigned int Lrc::sTimetoms(std::string s){ // mm:ss.xx -> ms
    s = removeSpace(s);
    int pos1 = s.find(":");
    int pos2 = s.find(".");
    if(pos1==-1 || pos2==-1 || pos1>=pos2)
        return -1;
    unsigned int mm = std::stoi(s.substr(0,pos1));
    unsigned int ss = std::stoi(s.substr(pos1+1,pos2-pos1-1));
    unsigned int xx = std::stoi(s.substr(pos2+1));
    return xx*10 + ss*1000 + mm*60000;
}

std::string Lrc::removeSpace(std::string s){
    while(s.front() == ' ')
        s.erase(0, 1);
    while(s.back() == ' ')
        s.pop_back();
    return s;
}

Lyric Lrc::getLyric(int millisec){
    for(int i=0; i<lys.size(); i++)
        if(lys[i].ms>millisec)
            return lys[i-1];
    return lys.back();
}

std::vector<std::string> Lrc::getAllLyrics(){
    std::vector<std::string> tmp(0);
    for(int i = 1; i < lys.size(); i++) { //lys[0] is empty
        // if(lys[i].s1.back() == '\0')
        if (!lys[i].s1.empty()) {
            tmp.push_back(lys[i].s1 + lys[i].s2);
        } else {
            tmp.push_back(std::string(""));
        }
    }
    return tmp;
}
