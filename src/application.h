// clang-format off
// CSCI3280 Phase 1
// Thomas

// #ifndef GTKMM_EXAMPLEAPPLICATION_H
// #define GTKMM_EXAMPLEAPPLICATION_H
#include "listfiles.h"
#include "lrc.h"
#include "wav.h"

#include <iostream>
#include <string>
#include <filesystem>
#include <algorithm>
#include <random>
#include <boost/regex.hpp>
#include <fstream>
#include <thread>
#include <exception>

#include <gtkmm.h>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/audio/streamvolume.h>

#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>
#include <taglib/mpegfile.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/mp4file.h>
#include <taglib/mp4tag.h>
#include <taglib/mp4coverart.h>
#include <taglib/id3v2tag.h>
#include <taglib/tbytevector.h>

class MyApplication;

class MyApplication: public Gtk::Application
{
protected:
    MyApplication();

public:
    static Glib::RefPtr<MyApplication> create();

    typedef struct _GstData {
        GstElement* playbin;
        gboolean playing;
        gboolean terminate;
        gboolean seek_enabled;
        gboolean seek_done;
        gint64 duration;
    } *GstData;

    GstData gstdata;
    GstBus* bus;
    GstMessage* msg;
    GstStateChangeReturn ret;

    GstElement* playbin;
    GstElement* pipeline;
    GstElement* appsrc;
    GstBuffer* buffer;
    GstStreamVolume* volume;

    GstElement *bin, *spectrum, *sink;
    GstPad *pad, *ghost_pad;

protected:
    void on_activate() override;

private:
    Wav* wav;
    Lrc* LrcFile;
    std::vector<Lyric> Lyrics;
    int SelectedLyricIndex, LyricIndex, LyricEndtime;
    enum State { PLAYING, PAUSED };
    State state = PAUSED;
    enum PlayModes { SINGLE, SHUFFLE_OFF, SHUFFLE_ON };
    PlayModes PlayMode = SHUFFLE_OFF;
    bool ShowFileInSubfolders = false;
    bool SystemTogglingPlayButton = false, UpdatingLyrics = false;
    bool resorting = false, TreeViewColumnClicked = false;
    double Volume = 1.0;
    bool IsPlaying = false, GstIsPlaying = false, GstNeedSeek = false, GstVolumeChanged = false;
    gint64 CurrentPosInMilliseconds = 0, PlayedInMilliseconds = 0;

    std::random_device RandomDevice;
    std::default_random_engine RandomEngine = std::default_random_engine{ RandomDevice() };

    Gtk::Button* pButtonSettings1 = nullptr;
    Gtk::Dialog* pDialog2 = nullptr;
    Gtk::CheckButton* pCheckButton1 = nullptr;
    Gtk::Button* pButtonDialog2Cancel = nullptr, * pButtonDialog2Save = nullptr;
    Gtk::SearchEntry* pSearchEntry1 = nullptr;
    Glib::RefPtr<Gtk::EntryCompletion> pEntryCompletion1;
    Gtk::Label* pLabelLyric1 = nullptr;
    Gtk::ScrolledWindow* pScrolledWindow2 = nullptr;
    Gtk::TreeModelColumn<int>* pTreeModelColumnId2;
    Gtk::TreeModelColumn<Glib::ustring>* pTreeModelColumnLyric = nullptr;
    Gtk::TreeModelColumnRecord* pTreeModelColumnRecord2 = nullptr;
    Glib::RefPtr<Gtk::ListStore> pListStore2;
    Gtk::TreeView* pTreeView2 = nullptr;
    Glib::RefPtr<Gtk::TreeSelection> pTreeSelection2;
    Glib::ustring UserDirectory = Glib::get_home_dir();
    Glib::ustring DefaultDirectory = Glib::get_home_dir() + "\\Music";
    Glib::ustring Directory = DefaultDirectory;


    enum TreeViewColumns { ICON, TITLE, TIME, ARTIST, ALBUM, FILENAME };
    TreeViewColumns SortColumn = TITLE;
    Gtk::SortType SortOrder = Gtk::SortType::SORT_ASCENDING;
    Glib::ustring TimeString(int time);

    typedef struct MusicInfoCDT* MusicInfoADT;
    MusicInfoADT CurrentMusic, SelectedMusic, EmptyMusic;
    std::vector<MusicInfoADT>* AllMusic = {};
    std::vector<MusicInfoADT>* AllMusicCopy = {};
    std::vector<MusicInfoADT>* AllMusicShuffled = {};
    int AllMusicShuffledSize = 0, AllMusicShuffledPos = 0;
    const std::vector<std::string> exts = {
        // ".aac", // Taglib does not support .aac
        ".mp3",
     ".wav",
     ".m4a",
     ".ogg",
     ".flac",
     //  ".mkv"
    };
    std::vector<Glib::RefPtr<Gdk::Pixbuf>>* pIcons;
    Glib::RefPtr<Gdk::Pixbuf> pMP3Icon, pWAVIcon, pAudioIcon, pLogo;
    Glib::RefPtr<Gio::Resource> resources;
    Glib::RefPtr<Gtk::Builder> refBuilder;

    Gtk::MessageDialog* pMessageDialog1 = nullptr;
    Gtk::Button* pButtonMessageDialog1Close = nullptr;

    Gtk::Dialog* pDialog1 = nullptr;
    Gtk::Image* pImageCoverArt2 = nullptr;
    Gtk::Entry* pEntryTitle1 = nullptr, * pEntryTime1 = nullptr, * pEntryArtist1 = nullptr, * pEntryAlbum1 = nullptr, * pEntryFileName1 = nullptr;
    Gtk::Button* pButtonDialog1Cancel = nullptr, * pButtonDialog1Save = nullptr;

    Gtk::ApplicationWindow* pApplicationWindow1 = nullptr;
    Gtk::Box* pVBox1 = nullptr, * pVBox2 = nullptr, * pHBox1 = nullptr, * pHBox2 = nullptr, * pHBox3 = nullptr;
    Gtk::ToggleButton* pButtonPlay1 = nullptr;
    Gtk::Button* pButtonShuffle1 = nullptr, * pButtonPrevious1 = nullptr, * pButtonNext1 = nullptr, * pButtonBackward1 = nullptr, * pButtonForward1 = nullptr, * pVolumeButton1_Plus = nullptr, * pVolumeButton1_Minus = nullptr, * pButtonReload1 = nullptr, * pButtonAbout1 = nullptr;
    Gtk::Image* pButtonShuffle1_Img = nullptr, * pButtonPlay1_Img = nullptr, * pButtonBackward1_Img = nullptr, * pButtonForward1_Img = nullptr, * pButtonPrevious1_Img = nullptr, * pButtonNext1_Img = nullptr, * pButtonReload1_Img = nullptr, * pButtonAbout1_Img = nullptr, * pButtonSettings1_Img = nullptr;
    Gtk::VolumeButton* pVolumeButton1 = nullptr;

    Gtk::ScrolledWindow* pScrolledWindow1 = nullptr;

    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>* pTreeModelColumnIcon = nullptr;
    Gtk::TreeModelColumn<int>* pTreeModelColumnId = nullptr;
    Gtk::TreeModelColumn<Glib::ustring>* pTreeModelColumnTitle = nullptr, * pTreeModelColumnTime = nullptr, * pTreeModelColumnArtist = nullptr, * pTreeModelColumnAlbum = nullptr, * pTreeModelColumnFileName = nullptr;
    Gtk::TreeModelColumnRecord* pTreeModelColumnRecord1 = nullptr;
    Glib::RefPtr<Gtk::ListStore> pListStore1;
    Gtk::TreeView* pTreeView1 = nullptr;
    Glib::RefPtr<Gtk::TreeViewColumn> pTreeViewColumnIcon, pTreeViewColumnTitle, pTreeViewColumnTime, pTreeViewColumnArtist, pTreeViewColumnAlbum, pTreeViewColumnFileName;
    Glib::RefPtr<Gtk::TreeSelection> pTreeSelection1;

    std::vector<Glib::ustring> Menu1ItemLabels = { "Play", "Add To Playlist", "Edit Properties" };
    Gtk::Menu* pMenu1 = nullptr;
    std::vector<Gtk::MenuItem*>* pMenu1Items;

    Gtk::Image* pImageCoverArt1 = nullptr;
    Gtk::Label* pLabelTitle1 = nullptr, * pLabelDuration1 = nullptr, * pLabelArtist1 = nullptr, * pLabelAlbum1 = nullptr, * pLabelFileName1 = nullptr;
    bool ScaleIsBeingMoved = false;
    Gtk::Label* pLabel1 = nullptr, * pLabel2 = nullptr;
    Gtk::Scale* pScale1 = nullptr;
    Glib::RefPtr<Gtk::Adjustment> pAdjustment1, pAdjustmentVolume;
    Gtk::HeaderBar* pHeaderBar1 = nullptr;
    Gtk::AboutDialog* pAboutDialog1 = nullptr;
    Gtk::FileChooserButton* pFileChooserButton1 = nullptr;
    Gtk::Button* pFileChooserDialog1SelectButton = nullptr;
    Gtk::FileChooserDialog* pFileChooserDialog1 = nullptr;

    Gtk::ApplicationWindow* create_appwindow();
    Glib::RefPtr<Gdk::Pixbuf> select_icon(const std::filesystem::path file_path);

    void SortMusicListByIndex(TreeViewColumns SortColumn, Gtk::SortType SortOrder);

    void set_music_list();
    void update_tree_model();

    void on_ButtonShuffle1_clicked();
    void on_ButtonPlay1_clicked();
    void on_ButtonBackward1_clicked();
    void on_ButtonForward1_clicked();
    void on_ButtonPrevious1_clicked();
    void on_ButtonNext1_clicked();

    void on_hide_window(Gtk::ApplicationWindow* window);
    void on_Adjustment1_changed();
    void on_AdjustmentVolume_changed();
    bool on_Scale1_press_event(GdkEventButton* event);
    bool on_Scale1_release_event(GdkEventButton* event);
    void on_TreeViewColumnTitle_Clicked();
    void on_TreeViewColumnTime_Clicked();
    void on_TreeViewColumn_Clicked(TreeViewColumns CurrentSortColumn);
    bool on_TreeView1_button_press_event(GdkEventButton* button_event);
    void on_TreeSelection1_changed();
    void on_Menu1Item_activate(Glib::ustring Label);
    void on_VolumeButton1_Plus_clicked();
    void on_VolumeButton1_Minus_clicked();
    bool timeout1();
    void taglib_get_data(MusicInfoADT _music);
    void ResetTreeViewColumnHeaders();

    void PlayMusic();
    void PauseMusic();
    void ChangeMusic();
    void LoadMusic();

    static bool CompareByTitleAlphabeticalOrder(const MusicInfoADT& a, const MusicInfoADT& b);
    static bool CompareByTitleAlphabeticalOrderIndexAscending(const MusicInfoADT& a, const MusicInfoADT& b);
    static bool CompareByTitleAlphabeticalOrderIndexDescending(const MusicInfoADT& a, const MusicInfoADT& b);
    void SortMusicListByTitleAlphabeticalOrder();

    static bool CompareByDurationInMilliseconds(const MusicInfoADT& a, const MusicInfoADT& b);
    static bool CompareByDurationInMillisecondsIndexAscending(const MusicInfoADT& a, const MusicInfoADT& b);
    static bool CompareByDurationInMillisecondsIndexDescending(const MusicInfoADT& a, const MusicInfoADT& b);
    void SortMusicListByDurationInMilliseconds();


    static bool CompareByArtistAlphabeticalOrder(const MusicInfoADT& a, const MusicInfoADT& b);
    void SortMusicListByArtistAlphabeticalOrder();
    static bool CompareByArtistAlphabeticalOrderIndexAscending(const MusicInfoADT& a, const MusicInfoADT& b);
    static bool CompareByArtistAlphabeticalOrderIndexDescending(const MusicInfoADT& a, const MusicInfoADT& b);

    static bool CompareByAlbumAlphabeticalOrder(const MusicInfoADT& a, const MusicInfoADT& b);
    void SortMusicListByAlbumAlphabeticalOrder();
    static bool CompareByAlbumAlphabeticalOrderIndexAscending(const MusicInfoADT& a, const MusicInfoADT& b);
    static bool CompareByAlbumAlphabeticalOrderIndexDescending(const MusicInfoADT& a, const MusicInfoADT& b);

    static bool CompareByFileNameAlphabeticalOrder(const MusicInfoADT& a, const MusicInfoADT& b);
    void SortMusicListByFileNameAlphabeticalOrder();
    static bool CompareByFileNameAlphabeticalOrderIndexAscending(const MusicInfoADT& a, const MusicInfoADT& b);
    static bool CompareByFileNameAlphabeticalOrderIndexDescending(const MusicInfoADT& a, const MusicInfoADT& b);


    int GetSortIndex(MusicInfoADT _music, TreeViewColumns SortColumn, Gtk::SortType SortOrder);

    void on_AboutDialog1_response(int response_id);
    void on_ButtonAbout1_clicked();
    void on_FileChooserButton1_selection_changed();
    void on_FileChooserDialog1SelectButton_clicked();
    void on_Reload_clicked();

    void MusicListChanged();
    void DisplayCoverArtDialog1();
    void DisplayCoverArtSidebar();
    void ExtendAllMusicShuffled(bool CurrentMusicAtFront = false);

    void ShuffleOff();
    void Shuffle();

    void on_ButtonDialog1Cancel_clicked();
    void on_ButtonDialog1Save_clicked();

    void taglib_set_data(MusicInfoADT _music, Glib::ustring NewTitle, Glib::ustring NewArtist, Glib::ustring NewAlbum, Glib::ustring NewFileName);

    void ResetLyric();
    void update_tree_model_lyric();
    void on_TreeSelection2_changed();
    bool on_TreeView2_button_press_event(GdkEventButton* button_event);

    void on_MessageDialog1_response(int response_id);
    bool on_EntryCompletion1_match(const Glib::ustring& key, const Gtk::TreeModel::const_iterator& iter);
    bool on_EntryCompletion1_match_selected(const Gtk::TreeModel::const_iterator& iter);

    void on_ButtonSettings1_clicked();
    void on_ButtonDialog2Cancel_clicked();
    void on_ButtonDialog2Save_clicked();

    Gtk::DrawingArea* pDrawingArea1 = nullptr;
    bool done_draw = true;
    const guint spect_bands = 128;
    std::vector<double> magnitudes = std::vector<double>(spect_bands*5, -60);
    bool on_DrawingArea1_draw(const Cairo::RefPtr<Cairo::Context>& cr, const GdkEventExpose* event);
    void update_spectrum_data(const GstStructure *s);

    Glib::ustring PrettyString(const Glib::ustring& str, const int MaxLength);
};

// #endif /* GTKMM_EXAMPLEAPPLICATION_H */
