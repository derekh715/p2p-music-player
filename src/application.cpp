// CSCI3280 Phase 1
// Thomas

#include "application.h"
#include <filesystem>

struct MusicInfoCDT {
    Glib::ustring FileName = "", FilePath, SortFileName, SortTitle,
        DurationString = "99:59:59.999", Title = "", Album = "",
        Artist = "", Extension;
    std::string LRCFilePath = "";
    int Id = -1, Duration = 0, DurationInMilliseconds = 359999999,
        SortTitleAlphIndex = -1, SortArtistAlphIndex = -1,
        SortAlbumAlphIndex = -1, SortDurationInMillisecondsIndex = -1,
        SortFileNameAlphIndex = -1;
    Glib::RefPtr<Gdk::Pixbuf> pIcon;
    bool CoverArt = false, LRC = false, CanonicalWAV = true;
    Glib::ustring Checksum = "";
    // for debugging purposes
    friend std::ostream& operator<<(std::ostream& os, const MusicInfoCDT& m);
};

std::ostream& operator<<(std::ostream& os, const MusicInfoCDT& m) {
    std::cout << "For this MusicInfoCDT"
        << "\nFileName: " << m.FileName << "\nFilePath: " << m.FilePath
        << "\nExt: " << m.Extension
        << "\nDurationString: " << m.DurationString
        << "\nTitle: " << m.Title << "\nAlbum: " << m.Album
        << "\nArtist: " << m.Artist << "\nDuration: " << m.Duration
        << "\nDurationInMilliseconds: " << m.DurationInMilliseconds
        << "\nCoverArt: " << m.CoverArt << "\nLRC: " << m.LRC
        << "\nLRC Path: " << m.LRCFilePath << "\nId: " << m.Id
        << std::endl;
    return os;
}

Glib::ustring MyApplication::TimeString(int time) {
    if (time > 359999999)
        return Glib::ustring("XX:XX:XX");
    int h = 0, m = 0, s = 0, ms = 0;
    h = time / 3600000;
    m = (time % 3600000) / 60000;
    s = (time % 60000) / 1000;
    ms = time % 1000;
    char text[13];
    sprintf(text, "%02d:%02d:%02d.%03d", h, m, s, ms);
    return Glib::ustring(text);
}

MyApplication::MyApplication(const std::string& file, uint16_t port)
    : store(false, file),
    port(port), Gtk::Application("org.gtkmm.examples.application",
        Gio::APPLICATION_HANDLES_OPEN) {

    gst_init(NULL, NULL);

    // it depends on where you invoke the executable
    // if you are invoking it inside the src directory
    // then the if branch will be taken
    // else it will assume you are invoking it from the project root
    if (std::filesystem::is_regular_file("src")) {
        resources->create_from_file("src")->register_global();
    }
    else {
        resources->create_from_file("src/src")->register_global();
    }

    wav = new Wav;
    refBuilder = Gtk::Builder::create();
    try {
        refBuilder->add_from_resource("/my_app/layout");
    }
    catch (const Glib::FileError& ex) {
        std::cerr << "FileError: " << ex.what() << std::endl;
        exit(-1);
    }
    catch (const Glib::MarkupError& ex) {
        std::cerr << "MarkupError: " << ex.what() << std::endl;
        exit(-2);
    }
    catch (const Gtk::BuilderError& ex) {
        std::cerr << "BuilderError: " << ex.what() << std::endl;
        exit(-3);
    }

    pAudioIcon = Glib::RefPtr<Gdk::Pixbuf>(
        Gdk::Pixbuf::create_from_resource("/my_app/audio_icon.png")
        ->scale_simple(50, 50, Gdk::InterpType::INTERP_BILINEAR));
    pIcons = new std::vector<Glib::RefPtr<Gdk::Pixbuf>>(0);
    for (const std::string& ext : exts) {
        std::string icon_resource_path =
            "/my_app/" + ext.substr(1, ext.length() - 1);
        icon_resource_path += "_icon.png";
        if (Gio::Resource::get_file_exists_global_nothrow(icon_resource_path)) {
            Glib::RefPtr<Gdk::Pixbuf> pTypeIcon(
                Gdk::Pixbuf::create_from_resource(icon_resource_path.c_str())
                ->scale_simple(50, 50, Gdk::InterpType::INTERP_BILINEAR));
            pIcons->push_back(pTypeIcon);
        }
        else
            pIcons->push_back(pAudioIcon);
    }

    // this command here starts the client at port
    // default is 4000, can be overridden with command line args
    // it can be any port really, just an example
    start_client(port);
}

Glib::RefPtr<MyApplication> MyApplication::create(const std::string& filepath,
    uint16_t port) {
    return Glib::RefPtr<MyApplication>(new MyApplication(filepath, port));
}

Gtk::ApplicationWindow* MyApplication::create_appwindow() {
    pLogo = Glib::RefPtr<Gdk::Pixbuf>(
        Gdk::Pixbuf::create_from_resource("/my_app/music_player.png"));
    refBuilder->get_widget("ApplicationWindow1", pApplicationWindow1);
    pApplicationWindow1->set_icon(pLogo);

    refBuilder->get_widget("HeaderBar1", pHeaderBar1);
    pHeaderBar1->set_show_close_button(true);
    refBuilder->get_widget("ButtonAbout1", pButtonAbout1);
    refBuilder->get_widget("ButtonAbout1_Img", pButtonAbout1_Img);
    pButtonAbout1_Img->set_from_resource("/my_app/about.png");
    refBuilder->get_widget("MessageDialog1", pMessageDialog1);
    pMessageDialog1->set_modal(true);
    pMessageDialog1->add_button("Close", Gtk::RESPONSE_CLOSE);
    refBuilder->get_widget("Dialog1", pDialog1);
    pDialog1->set_modal(true);
    pButtonDialog1Cancel = pDialog1->add_button("Cancel", Gtk::RESPONSE_CANCEL);
    pButtonDialog1Save = pDialog1->add_button("Save", Gtk::RESPONSE_OK);
    refBuilder->get_widget("ImageCoverArt2", pImageCoverArt2);
    refBuilder->get_widget("EntryTitle1", pEntryTitle1);
    pEntryTitle1->set_editable(true);
    refBuilder->get_widget("EntryTime1", pEntryTime1);
    pEntryTime1->set_editable(false);
    refBuilder->get_widget("EntryArtist1", pEntryArtist1);
    pEntryArtist1->set_editable(true);
    refBuilder->get_widget("EntryAlbum1", pEntryAlbum1);
    pEntryAlbum1->set_editable(true);
    refBuilder->get_widget("EntryFileName1", pEntryFileName1);
    pEntryFileName1->set_editable(true);
    refBuilder->get_widget("ButtonSettings1", pButtonSettings1);
    refBuilder->get_widget("ButtonSettings1_Img", pButtonSettings1_Img);
    pButtonSettings1_Img->set_from_resource("/my_app/settings.png");
    refBuilder->get_widget("Dialog2", pDialog2);
    pDialog2->set_modal(true);
    pButtonDialog2Cancel = pDialog2->add_button("Cancel", Gtk::RESPONSE_CANCEL);
    pButtonDialog2Save = pDialog2->add_button("Save", Gtk::RESPONSE_OK);
    refBuilder->get_widget("CheckButton1", pCheckButton1);
    refBuilder->get_widget("AboutDialog1", pAboutDialog1);
    pAboutDialog1->set_icon(pLogo);
    pAboutDialog1->set_logo(pLogo);
    refBuilder->get_widget("ButtonReload1", pButtonReload1);
    refBuilder->get_widget("ButtonReload1_Img", pButtonReload1_Img);
    pButtonReload1_Img->set_from_resource("/my_app/reload.png");

    refBuilder->get_widget("CheckButton2", pCheckButton2);
    refBuilder->get_widget("EntryIp1", pEntryIp1);
    refBuilder->get_widget("ButtonAddIp1", pButtonAddIp1);
    refBuilder->get_widget("ButtonRemoveIp1", pButtonRemoveIp1);
    refBuilder->get_widget("ButtonRemoveAllIp1", pButtonRemoveAllIp1);
    refBuilder->get_widget("FileChooserDialog1", pFileChooserDialog1);
    pFileChooserDialog1->set_action(
        Gtk::FileChooserAction::FILE_CHOOSER_ACTION_SELECT_FOLDER);
    pFileChooserDialog1->add_button("Cancel", Gtk::RESPONSE_CANCEL);
    pFileChooserDialog1SelectButton =
        pFileChooserDialog1->add_button("Select", Gtk::RESPONSE_OK);
    pFileChooserDialog1->set_current_folder(DefaultDirectory);
    pHeaderBar1->set_title(std::filesystem::path(std::string(DefaultDirectory))
        .filename()
        .string());
    pHeaderBar1->set_subtitle(DefaultDirectory);

    refBuilder->get_widget("FileChooserButton1", pFileChooserButton1);
    pFileChooserButton1->set_action(
        Gtk::FileChooserAction::FILE_CHOOSER_ACTION_SELECT_FOLDER);
    pFileChooserButton1->add_shortcut_folder(UserDirectory + "\\Desktop");
    pFileChooserButton1->add_shortcut_folder(UserDirectory + "\\Documents");
    pFileChooserButton1->add_shortcut_folder(UserDirectory + "\\Downloads");
    pFileChooserButton1->add_shortcut_folder(UserDirectory + "\\Music");
    pFileChooserButton1->add_shortcut_folder(UserDirectory + "\\Videos");

    refBuilder->get_widget("VBox1", pVBox1);
    refBuilder->get_widget("HBox1", pHBox1);

    refBuilder->get_widget("ButtonShuffle1", pButtonShuffle1);
    refBuilder->get_widget("ButtonShuffle1_Img", pButtonShuffle1_Img);
    pButtonShuffle1_Img->set_from_resource("/my_app/shuffle_off.png");

    refBuilder->get_widget("ButtonPlay1", pButtonPlay1);
    refBuilder->get_widget("ButtonPlay1_Img", pButtonPlay1_Img);
    pButtonPlay1_Img->set_from_resource("/my_app/start.png");

    refBuilder->get_widget("ButtonBackward1", pButtonBackward1);
    refBuilder->get_widget("ButtonBackward1_Img", pButtonBackward1_Img);
    pButtonBackward1_Img->set_from_resource("/my_app/backward.png");

    refBuilder->get_widget("ButtonForward1", pButtonForward1);
    refBuilder->get_widget("ButtonForward1_Img", pButtonForward1_Img);
    pButtonForward1_Img->set_from_resource("/my_app/forward.png");

    refBuilder->get_widget("ButtonPrevious1", pButtonPrevious1);
    refBuilder->get_widget("ButtonPrevious1_Img", pButtonPrevious1_Img);
    pButtonPrevious1_Img->set_from_resource("/my_app/previous.png");

    refBuilder->get_widget("ButtonNext1", pButtonNext1);
    refBuilder->get_widget("ButtonNext1_Img", pButtonNext1_Img);
    pButtonNext1_Img->set_from_resource("/my_app/next.png");

    refBuilder->get_widget("VolumeButton1", pVolumeButton1);
    refBuilder->get_widget("VolumeButton1_Plus", pVolumeButton1_Plus);
    refBuilder->get_widget("VolumeButton1_Minus", pVolumeButton1_Minus);

    pAdjustmentVolume = Glib::RefPtr<Gtk::Adjustment>(
        Gtk::Adjustment::create(0, 0, 1.0, 0.01, 0.2, 0));
    pVolumeButton1->set_adjustment(pAdjustmentVolume);
    pVolumeButton1->set_value(1.0);

    refBuilder->get_widget("ScrolledWindow1", pScrolledWindow1);
    pTreeView1 = new Gtk::TreeView;

    refBuilder->get_widget("VBox2", pVBox2);
    refBuilder->get_widget("ImageCoverArt1", pImageCoverArt1);
    refBuilder->get_widget("LabelTitle1", pLabelTitle1);
    pLabelTitle1->set_label("Title:\t");
    pLabelTitle1->property_xalign() = 0;
    pLabelTitle1->set_max_width_chars(400);
    refBuilder->get_widget("LabelDuration1", pLabelDuration1);
    pLabelDuration1->set_label("Duration:\t");
    pLabelDuration1->property_xalign() = 0;
    pLabelDuration1->set_max_width_chars(400);
    refBuilder->get_widget("LabelArtist1", pLabelArtist1);
    pLabelArtist1->set_label("Artist:\t");
    pLabelArtist1->property_xalign() = 0;
    pLabelArtist1->set_max_width_chars(400);
    refBuilder->get_widget("LabelAlbum1", pLabelAlbum1);
    pLabelAlbum1->set_label("Album:\t");
    pLabelAlbum1->property_xalign() = 0;
    pLabelAlbum1->set_max_width_chars(400);
    refBuilder->get_widget("LabelFileName1", pLabelFileName1);
    pLabelFileName1->set_label("Filename:\t");
    pLabelFileName1->property_xalign() = 0;
    pLabelFileName1->set_max_width_chars(400);

    refBuilder->get_widget("DrawingArea1", pDrawingArea1);
    pDrawingArea1->signal_draw().connect(
        [this](const Cairo::RefPtr<Cairo::Context>& cr) -> bool {
            return on_DrawingArea1_draw(cr, nullptr);
        });

    refBuilder->get_widget("Label1", pLabel1);
    refBuilder->get_widget("Label2", pLabel2);
    refBuilder->get_widget("Scale1", pScale1);
    pAdjustment1 = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(
        refBuilder->get_object("Adjustment1"));

    pScale1->set_draw_value(false);

    pAdjustment1->set_lower(0);
    pAdjustment1->set_upper(359999999);
    pAdjustment1->set_page_increment(1);
    pScrolledWindow1->set_policy(Gtk::PolicyType::POLICY_NEVER,
        Gtk::PolicyType::POLICY_ALWAYS);
    pScrolledWindow1->add(*pTreeView1);

    pTreeModelColumnIcon = new Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>;
    pTreeModelColumnId = new Gtk::TreeModelColumn<int>;
    pTreeModelColumnTitle = new Gtk::TreeModelColumn<Glib::ustring>,
        pTreeModelColumnTime = new Gtk::TreeModelColumn<Glib::ustring>,
        pTreeModelColumnArtist = new Gtk::TreeModelColumn<Glib::ustring>,
        pTreeModelColumnAlbum = new Gtk::TreeModelColumn<Glib::ustring>,
        pTreeModelColumnFileName = new Gtk::TreeModelColumn<Glib::ustring>;

    pTreeModelColumnRecord1 = new Gtk::TreeModelColumnRecord;
    pTreeModelColumnRecord1->add(*pTreeModelColumnId);
    pTreeModelColumnRecord1->add(*pTreeModelColumnIcon);
    pTreeModelColumnRecord1->add(*pTreeModelColumnTitle);
    pTreeModelColumnRecord1->add(*pTreeModelColumnTime);
    pTreeModelColumnRecord1->add(*pTreeModelColumnArtist);
    pTreeModelColumnRecord1->add(*pTreeModelColumnAlbum);
    pTreeModelColumnRecord1->add(*pTreeModelColumnFileName);

    pListStore1 = Gtk::ListStore::create(*pTreeModelColumnRecord1);
    pTreeView1->set_model(pListStore1);
    pTreeView1->append_column("", *pTreeModelColumnIcon);
    pTreeView1->append_column("Title", *pTreeModelColumnTitle);
    pTreeView1->append_column("Time", *pTreeModelColumnTime);
    pTreeView1->append_column("Artist", *pTreeModelColumnArtist);
    pTreeView1->append_column("Album", *pTreeModelColumnAlbum);
    pTreeView1->append_column("Filename", *pTreeModelColumnFileName);
    for (int x = 0; x < pTreeView1->get_n_columns(); x++) {
        Gtk::TreeViewColumn* col = pTreeView1->get_column(x);
        col->set_sizing(Gtk::TreeViewColumnSizing::TREE_VIEW_COLUMN_AUTOSIZE);
        col->set_alignment(0);
        col->set_sort_indicator(false);
        col->set_clickable(true);
        if (x) {
            col->set_expand(true);
            col->set_min_width(150);
            col->set_sizing(
                Gtk::TreeViewColumnSizing::TREE_VIEW_COLUMN_AUTOSIZE);
            col->set_resizable(true);
        }
        else {
            col->set_resizable(false);
            col->set_expand(false);
            col->set_sizing(Gtk::TreeViewColumnSizing::TREE_VIEW_COLUMN_FIXED);
            col->set_fixed_width(75);
        }
        if (x == TITLE) {
            col->set_sort_indicator(true);
            col->set_sort_order(Gtk::SortType::SORT_ASCENDING);
        }
        if (x == 2) {
            col->set_expand(false);
            col->set_resizable(false);
            col->set_sizing(Gtk::TreeViewColumnSizing::TREE_VIEW_COLUMN_FIXED);
            col->set_min_width(150);
            col->set_fixed_width(150);
        }
    }

    refBuilder->get_widget("SearchEntry1", pSearchEntry1);
    pEntryCompletion1 = Gtk::EntryCompletion::create();
    pEntryCompletion1->set_model(pListStore1);
    pSearchEntry1->set_completion(pEntryCompletion1);
    pEntryCompletion1->set_text_column(*pTreeModelColumnTitle);
    pEntryCompletion1->set_match_func(
        sigc::mem_fun(*this, &MyApplication::on_EntryCompletion1_match));
    pEntryCompletion1->signal_match_selected().connect(
        sigc::mem_fun(*this,
            &MyApplication::on_EntryCompletion1_match_selected),
        false);

    CurrentMusic = new MusicInfoCDT;
    SelectedMusic = new MusicInfoCDT;
    EmptyMusic = new MusicInfoCDT;

    refBuilder->get_widget("ScrolledWindow2", pScrolledWindow2);
    pTreeModelColumnId2 = new Gtk::TreeModelColumn<int>;
    pTreeModelColumnLyric = new Gtk::TreeModelColumn<Glib::ustring>;
    pTreeModelColumnRecord2 = new Gtk::TreeModelColumnRecord;
    pTreeModelColumnRecord2->add(*pTreeModelColumnId2);
    pTreeModelColumnRecord2->add(*pTreeModelColumnLyric);
    pListStore2 = Gtk::ListStore::create(*pTreeModelColumnRecord2);
    pTreeView2 = new Gtk::TreeView;
    pTreeView2->set_model(pListStore2);
    pTreeView2->append_column("Lyrics:", *pTreeModelColumnLyric);
    pTreeView2->get_column(0)->set_sizing(
        Gtk::TreeViewColumnSizing::TREE_VIEW_COLUMN_AUTOSIZE);
    pTreeView2->get_column(0)->set_clickable(false);
    pTreeView2->get_column(0)->set_expand(true);
    pTreeView2->get_column(0)->set_alignment(0.5);
    pTreeView2->get_column(0)->set_sort_indicator(false);
    pTreeSelection2 = pTreeView2->get_selection();
    pTreeSelection2->set_mode(Gtk::SELECTION_SINGLE);
    pScrolledWindow2->add(*pTreeView2);

    refBuilder->get_widget("ScrolledWindow3", pScrolledWindow3);
    pTreeModelColumnId3 = new Gtk::TreeModelColumn<int>;
    pTreeModelColumnIp = new Gtk::TreeModelColumn<Glib::ustring>;
    pTreeModelColumnRecord3 = new Gtk::TreeModelColumnRecord;
    pTreeModelColumnRecord3->add(*pTreeModelColumnId3);
    pTreeModelColumnRecord3->add(*pTreeModelColumnIp);
    pListStore3 = Gtk::ListStore::create(*pTreeModelColumnRecord3);
    pTreeView3 = new Gtk::TreeView;
    pTreeView3->set_model(pListStore3);
    pTreeView3->append_column("Ip:", *pTreeModelColumnIp);
    pTreeView3->get_column(0)->set_sizing(
        Gtk::TreeViewColumnSizing::TREE_VIEW_COLUMN_AUTOSIZE);
    pTreeView3->get_column(0)->set_clickable(false);
    pTreeView3->get_column(0)->set_expand(true);
    pTreeView3->get_column(0)->set_alignment(0.5);
    pTreeView3->get_column(0)->set_sort_indicator(false);
    pTreeSelection3 = pTreeView3->get_selection();
    pTreeSelection3->set_mode(Gtk::SELECTION_SINGLE);
    pScrolledWindow3->add(*pTreeView3);
    pButtonAddIp1->signal_pressed().connect(
        sigc::mem_fun(*this, &MyApplication::on_ButtonAddIp1_clicked));
    pButtonRemoveIp1->signal_pressed().connect(
        sigc::mem_fun(*this, &MyApplication::on_ButtonRemoveIp1_clicked));
    pButtonRemoveAllIp1->signal_pressed().connect(
        sigc::mem_fun(*this, &MyApplication::on_ButtonRemoveAllIp1_clicked));

    MusicListChanged();

    pMenu1 = new Gtk::Menu();
    pMenu1Items = new std::vector<Gtk::MenuItem*>(0);
    for (const Glib::ustring label : Menu1ItemLabels) {
        Gtk::MenuItem* pMenu1Item = new Gtk::MenuItem(label);
        pMenu1Items->push_back(pMenu1Item);
        pMenu1->append(*pMenu1Item);
        pMenu1Item->signal_activate().connect(sigc::bind(
            sigc::mem_fun(*this, &MyApplication::on_Menu1Item_activate),
            label));
    }
    pMenu1->accelerate(*pApplicationWindow1);
    pMenu1->show_all();

    add_window(*pApplicationWindow1);
    pApplicationWindow1->show_all_children();

    Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &MyApplication::timeout1), 25,
        Glib::PRIORITY_HIGH_IDLE);
    pApplicationWindow1->signal_hide().connect(
        sigc::bind<Gtk::ApplicationWindow*>(
            sigc::mem_fun(*this, &MyApplication::on_hide_window),
            pApplicationWindow1));
    pMessageDialog1->signal_response().connect(
        sigc::mem_fun(*this, &MyApplication::on_MessageDialog1_response));
    pButtonDialog1Cancel->signal_clicked().connect(
        sigc::mem_fun(*this, &MyApplication::on_ButtonDialog1Cancel_clicked));
    pButtonDialog1Save->signal_clicked().connect(
        sigc::mem_fun(*this, &MyApplication::on_ButtonDialog1Save_clicked));
    pButtonSettings1->signal_clicked().connect(
        sigc::mem_fun(*this, &MyApplication::on_ButtonSettings1_clicked));
    pButtonDialog2Cancel->signal_clicked().connect(
        sigc::mem_fun(*this, &MyApplication::on_ButtonDialog2Cancel_clicked));
    pButtonDialog2Save->signal_clicked().connect(
        sigc::mem_fun(*this, &MyApplication::on_ButtonDialog2Save_clicked));
    pButtonAbout1->signal_clicked().connect(
        sigc::mem_fun(*this, &MyApplication::on_ButtonAbout1_clicked));
    pAboutDialog1->signal_response().connect(
        sigc::mem_fun(*this, &MyApplication::on_AboutDialog1_response));
    pFileChooserButton1->signal_selection_changed().connect(sigc::mem_fun(
        *this, &MyApplication::on_FileChooserButton1_selection_changed));
    pFileChooserDialog1SelectButton->signal_clicked().connect(sigc::mem_fun(
        *this, &MyApplication::on_FileChooserDialog1SelectButton_clicked));
    pButtonReload1->signal_clicked().connect(
        sigc::mem_fun(*this, &MyApplication::on_Reload_clicked));
    pButtonShuffle1->signal_clicked().connect(
        sigc::mem_fun(*this, &MyApplication::on_ButtonShuffle1_clicked));
    pButtonPlay1->signal_clicked().connect(
        sigc::mem_fun(*this, &MyApplication::on_ButtonPlay1_clicked));
    pButtonBackward1->signal_clicked().connect(
        sigc::mem_fun(*this, &MyApplication::on_ButtonBackward1_clicked));
    pButtonForward1->signal_clicked().connect(
        sigc::mem_fun(*this, &MyApplication::on_ButtonForward1_clicked));
    pButtonPrevious1->signal_clicked().connect(
        sigc::mem_fun(*this, &MyApplication::on_ButtonPrevious1_clicked));
    pButtonNext1->signal_clicked().connect(
        sigc::mem_fun(*this, &MyApplication::on_ButtonNext1_clicked));
    pVolumeButton1_Plus->signal_clicked().connect_notify(
        sigc::mem_fun(*this, &MyApplication::on_VolumeButton1_Plus_clicked),
        false);
    pVolumeButton1_Minus->signal_clicked().connect_notify(
        sigc::mem_fun(*this, &MyApplication::on_VolumeButton1_Minus_clicked),
        false);
    pScale1->signal_button_press_event().connect(
        sigc::mem_fun(*this, &MyApplication::on_Scale1_press_event), false);
    pScale1->signal_button_release_event().connect(
        sigc::mem_fun(*this, &MyApplication::on_Scale1_release_event), false);
    pAdjustment1->signal_value_changed().connect(
        sigc::mem_fun(*this, &MyApplication::on_Adjustment1_changed));
    pAdjustmentVolume->signal_value_changed().connect(
        sigc::mem_fun(*this, &MyApplication::on_AdjustmentVolume_changed));
    pTreeSelection1 = pTreeView1->get_selection();
    pTreeSelection1->set_mode(Gtk::SELECTION_SINGLE);
    pTreeSelection1->signal_changed().connect(
        sigc::mem_fun(*this, &MyApplication::on_TreeSelection1_changed));
    for (TreeViewColumns x = ICON; x <= FILENAME;
        x = static_cast<TreeViewColumns>(x + 1))
        pTreeView1->get_column(x)->signal_clicked().connect(
            sigc::bind(
                sigc::mem_fun(*this, &MyApplication::on_TreeViewColumn_Clicked),
                x),
            false);
    pTreeView1->signal_button_press_event().connect(
        sigc::mem_fun(*this, &MyApplication::on_TreeView1_button_press_event),
        false);
    pTreeSelection2->signal_changed().connect(
        sigc::mem_fun(*this, &MyApplication::on_TreeSelection2_changed));
    pTreeView2->signal_button_press_event().connect(
        sigc::mem_fun(*this, &MyApplication::on_TreeView2_button_press_event),
        false);

    return pApplicationWindow1;
}

Glib::RefPtr<Gdk::Pixbuf>
MyApplication::select_icon(const std::filesystem::path file_path) {
    std::string file_ext = file_path.extension().string();
    int counter = 0;
    for (const std::string& ext : exts) {
        if (file_ext == ext)
            return pIcons->at(counter);
        counter++;
    }
}

void MyApplication::on_activate() {
    AllMusic = new std::vector<MusicInfoADT>;
    create_appwindow();
    pApplicationWindow1->present();
}

void MyApplication::on_hide_window(Gtk::ApplicationWindow* pApplicationWindow) {
    delete pApplicationWindow;
}

void MyApplication::MusicListChanged() {
    resorting = true;
    PauseMusic();
    CurrentMusic = EmptyMusic;
    ChangeMusic();
    set_music_list();
    update_tree_model();
    for (int x = 0; x < pTreeView1->get_n_columns(); x++)
        pTreeView1->get_column(x)->set_sort_indicator(false);
    pTreeView1->get_column(TITLE)->set_sort_indicator(true);
    pTreeView1->get_column(TITLE)->set_sort_order(
        Gtk::SortType::SORT_ASCENDING);
    SortColumn = TITLE;
    SortOrder = Gtk::SortType::SORT_ASCENDING;
    resorting = false;

    if (PlayMode == SHUFFLE_ON)
        Shuffle();
}

void MyApplication::set_music_list() {
    for (int x = 0; x < AllMusic->size(); x++)
        delete AllMusic->at(x);
    AllMusic->resize(0);
    std::vector<std::filesystem::path> files_path = ListFiles::listfiles(
        Directory, exts, true, ShowFileInSubfolders, false);
    std::vector<Track> collected;
    for (const std::filesystem::path& file_path : files_path) {
        MusicInfoADT _music = new MusicInfoCDT;
        std::string ext = file_path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        _music->Extension = Glib::ustring(ext).lowercase();

        _music->FileName = Glib::ustring(file_path.filename().string());
        _music->FileName =
            _music->FileName.substr(0, _music->FileName.size() - ext.length());
        _music->SortFileName = _music->FileName.lowercase();
        std::string FilePath = file_path.string();
        std::replace(FilePath.begin(), FilePath.end(), '\\', '/');
        _music->FilePath = Glib::ustring(FilePath);
        _music->pIcon = select_icon(file_path);
        std::string lrcfilepath =
            file_path.parent_path().string() + "/" + _music->FileName + ".lrc";
        if (std::filesystem::exists(lrcfilepath)) {
            _music->LRC = true;
            _music->LRCFilePath = lrcfilepath;
        }
        taglib_get_data(_music);
        collected.push_back(convert_music_info_to_track(*_music));
        AllMusic->push_back(_music);
    }

    for (auto& r : network_tracks) {
        MusicInfoADT _music = new MusicInfoCDT;
        std::string ext = std::filesystem::path(r.second.track.path).extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        _music->Extension = Glib::ustring(ext).lowercase();

        _music->FileName = Glib::ustring(std::filesystem::path(r.second.track.path).filename().string());
        _music->FileName =
            _music->FileName.substr(0, _music->FileName.size() - ext.length());
        _music->SortFileName = _music->FileName.lowercase();
        std::string FilePath = "Network";
        std::replace(FilePath.begin(), FilePath.end(), '\\', '/');
        _music->FilePath = Glib::ustring(FilePath);
        _music->pIcon = select_icon(std::filesystem::path(r.second.track.path));
        std::string lrcfilepath = r.second.track.lrcfile;
        if (lrcfilepath != "") {
            _music->LRC = true;
            _music->LRCFilePath = lrcfilepath;
        }
        _music->Album = r.second.track.album;
        _music->Artist = r.second.track.artist;
        _music->Title = r.second.track.title;
        _music->Checksum = r.second.track.checksum;
        collected.push_back(convert_music_info_to_track(*_music));
        AllMusic->push_back(_music);
    }


    std::thread add_tracks_in_background(
        [&collected, this]() { store.upsert_many(collected); });
    if (AllMusicCopy == nullptr)
        AllMusicCopy = new std::vector<MusicInfoADT>;
    else
        AllMusicCopy->resize(0);
    SortMusicListByTitleAlphabeticalOrder();
    for (int counter = 0; counter < AllMusic->size(); counter++) {
        AllMusic->at(counter)->Id = counter;
        AllMusicCopy->push_back(AllMusic->at(counter));
    }
    SortMusicListByDurationInMilliseconds();
    SortMusicListByArtistAlphabeticalOrder();
    SortMusicListByAlbumAlphabeticalOrder();
    SortMusicListByFileNameAlphabeticalOrder();
    SortMusicListByIndex(TITLE, Gtk::SortType::SORT_ASCENDING);
    add_tracks_in_background.join();
}

Track MyApplication::convert_music_info_to_track(const MusicInfoCDT& m) {
    Track t;
    t.path = m.FilePath;
    t.album = m.Album;
    t.artist = m.Artist;
    t.lrcfile = m.LRCFilePath;
    t.duration = m.DurationInMilliseconds;
    t.title = m.Title;
    return t;
}

void MyApplication::update_tree_model() {
    pListStore1->clear();
    for (int counter = 0; counter < AllMusic->size(); counter++) {
        Gtk::TreeModel::Row row = *(pListStore1->append());
        row[*pTreeModelColumnId] = AllMusic->at(counter)->Id;
        row[*pTreeModelColumnIcon] = AllMusic->at(counter)->pIcon;
        row[*pTreeModelColumnTitle] = AllMusic->at(counter)->Title;
        row[*pTreeModelColumnTime] = AllMusic->at(counter)->DurationString;
        row[*pTreeModelColumnArtist] = AllMusic->at(counter)->Artist;
        row[*pTreeModelColumnAlbum] = AllMusic->at(counter)->Album;
        row[*pTreeModelColumnFileName] = AllMusic->at(counter)->FileName;
    }
}

void MyApplication::Shuffle() {
    bool OriginalIsPlaying = IsPlaying;
    if (AllMusicShuffled == nullptr)
        AllMusicShuffled = new std::vector<MusicInfoADT>;
    else
        AllMusicShuffled->resize(0);
    AllMusicShuffledPos = 0;
    AllMusicShuffledSize = 0;

    ExtendAllMusicShuffled(true);

    CurrentMusic = AllMusicShuffled->at(0);
    ChangeMusic();
    if (OriginalIsPlaying)
        PlayMusic();
}

void MyApplication::on_ButtonShuffle1_clicked() {
    if (AllMusic->size() > 0)
        if (PlayMode == SHUFFLE_OFF) {
            PlayMode = SHUFFLE_ON;
            pButtonShuffle1->set_label("Shuffle On");
            pButtonShuffle1_Img->set_from_resource("/my_app/shuffle_on.png");

            Shuffle();
        }
        else if (PlayMode == SHUFFLE_ON) {
            ShuffleOff();
        }
}
void MyApplication::ShuffleOff() {
    PlayMode = SHUFFLE_OFF;
    pButtonShuffle1->set_label("Shuffle Off");
    pButtonShuffle1_Img->set_from_resource("/my_app/shuffle_off.png");

    if (AllMusicShuffled != nullptr)
        AllMusicShuffled->resize(0);
    AllMusicShuffledPos = 0;
    AllMusicShuffledSize = 0;
}

void MyApplication::ExtendAllMusicShuffled(bool CurrentMusicAtFront) {
    std::vector<MusicInfoADT>* AllMusicShuffledNewSequence;
    AllMusicShuffledNewSequence = new std::vector<MusicInfoADT>;
    *AllMusicShuffledNewSequence = *AllMusicCopy;
    std::shuffle(AllMusicShuffledNewSequence->begin(),
        AllMusicShuffledNewSequence->end(), RandomEngine);
    if (CurrentMusicAtFront && CurrentMusic->Id >= 0) {
        AllMusicShuffledNewSequence->erase(
            std::remove(AllMusicShuffledNewSequence->begin(),
                AllMusicShuffledNewSequence->end(), CurrentMusic),
            AllMusicShuffledNewSequence->end());
        AllMusicShuffledNewSequence->insert(
            AllMusicShuffledNewSequence->begin(), CurrentMusic);
    }
    AllMusicShuffled->insert(AllMusicShuffled->end(),
        AllMusicShuffledNewSequence->begin(),
        AllMusicShuffledNewSequence->end());
    AllMusicShuffledSize += AllMusicShuffledNewSequence->size();
    delete AllMusicShuffledNewSequence;
}

void MyApplication::on_ButtonBackward1_clicked() {
    bool OriginalIsPlaying = IsPlaying;
    PauseMusic();
    CurrentPosInMilliseconds =
        (CurrentPosInMilliseconds < 10000 ? 0
            : CurrentPosInMilliseconds - 10000);
    pLabel1->set_text(TimeString(CurrentPosInMilliseconds));
    ScaleIsBeingMoved = true;
    pAdjustment1->set_value(CurrentPosInMilliseconds);
    ScaleIsBeingMoved = false;
    if (OriginalIsPlaying)
        PlayMusic();
}

void MyApplication::on_ButtonForward1_clicked() {
    bool OriginalIsPlaying = IsPlaying;
    PauseMusic();
    CurrentPosInMilliseconds =
        (CurrentPosInMilliseconds + 30000 > CurrentMusic->DurationInMilliseconds
            ? CurrentMusic->DurationInMilliseconds
            : CurrentPosInMilliseconds + 30000);
    pLabel1->set_text(TimeString(CurrentPosInMilliseconds));
    ScaleIsBeingMoved = true;
    pAdjustment1->set_value(CurrentPosInMilliseconds);
    ScaleIsBeingMoved = false;
    if (OriginalIsPlaying)
        PlayMusic();
}

void MyApplication::on_ButtonPrevious1_clicked() {
    if (PlayMode == SHUFFLE_OFF && CurrentMusic->Id >= 0) {
        bool OriginalIsPlaying = IsPlaying;
        CurrentMusic =
            AllMusic->at((GetSortIndex(CurrentMusic, SortColumn, SortOrder) +
                AllMusic->size() - 1) %
                int(AllMusic->size()));
        ChangeMusic();
        if (OriginalIsPlaying)
            PlayMusic();
    }
    if (PlayMode == SHUFFLE_ON && AllMusicShuffledPos > 0) {
        bool OriginalIsPlaying = IsPlaying;
        AllMusicShuffledPos--;
        CurrentMusic = AllMusicShuffled->at(AllMusicShuffledPos);
        ChangeMusic();
        if (OriginalIsPlaying)
            PlayMusic();
    }
}

void MyApplication::on_ButtonNext1_clicked() {
    if (PlayMode == SHUFFLE_OFF && CurrentMusic->Id >= 0) {
        bool OriginalIsPlaying = IsPlaying;
        CurrentMusic = AllMusic->at(
            ((GetSortIndex(CurrentMusic, SortColumn, SortOrder) + 1) %
                int(AllMusic->size())));
        ChangeMusic();
        if (OriginalIsPlaying)
            PlayMusic();
    }
    if (PlayMode == SHUFFLE_ON) {
        bool OriginalIsPlaying = IsPlaying;
        AllMusicShuffledPos++;
        if (AllMusicShuffledSize == AllMusicShuffledPos)
            ExtendAllMusicShuffled();
        CurrentMusic = AllMusicShuffled->at(AllMusicShuffledPos);
        ChangeMusic();
        if (OriginalIsPlaying)
            PlayMusic();
    }
}

void MyApplication::on_ButtonPlay1_clicked() {
    if (!SystemTogglingPlayButton) {
        if (state == PAUSED)
            PlayMusic();
        else
            PauseMusic();
    }
}

void MyApplication::on_VolumeButton1_Plus_clicked() {
    Volume = (Volume > 0.8 ? 1 : Volume + 0.2);
    pAdjustmentVolume->set_value(Volume);
    GstVolumeChanged = true;
}

void MyApplication::on_VolumeButton1_Minus_clicked() {
    Volume = (Volume < 0.2 ? 0 : Volume - 0.2);
    pAdjustmentVolume->set_value(Volume);
    GstVolumeChanged = true;
}

void MyApplication::on_AdjustmentVolume_changed() {
    Volume = pVolumeButton1->get_value();
    GstVolumeChanged = true;
}

void MyApplication::on_ButtonSettings1_clicked() {
    pCheckButton1->set_active(ShowFileInSubfolders);
    pCheckButton2->set_active(ShowFileFromNetwork);
    pDialog2->show();
    pDialog2->show_all();
    pDialog2->show_all_children();
}

void MyApplication::on_ButtonDialog2Cancel_clicked() { pDialog2->hide(); }
void MyApplication::on_ButtonDialog2Save_clicked() {
    bool OriginalShowFileInSubfolders = ShowFileInSubfolders,
        OriginalShowFileFromNetwork = ShowFileFromNetwork;
    ShowFileInSubfolders = pCheckButton1->get_active();
    ShowFileFromNetwork = pCheckButton2->get_active();
    pDialog2->hide();
    if (OriginalShowFileInSubfolders != ShowFileInSubfolders ||
        OriginalShowFileFromNetwork != ShowFileFromNetwork || IpsChanged)
        MusicListChanged();
}

void MyApplication::on_ButtonAbout1_clicked() { pAboutDialog1->run(); }

void MyApplication::on_AboutDialog1_response(int response_id) {
    if (response_id == Gtk::RESPONSE_DELETE_EVENT)
        pAboutDialog1->hide();
}

void MyApplication::on_Reload_clicked() { MusicListChanged(); }

void MyApplication::on_FileChooserButton1_selection_changed() {
    Directory = pFileChooserButton1->get_filename();
    MusicListChanged();
    pHeaderBar1->set_title(
        std::filesystem::path(std::string(Directory)).filename().string());
    pHeaderBar1->set_subtitle(Directory);
}

void MyApplication::on_FileChooserDialog1SelectButton_clicked() {
    Directory = pFileChooserDialog1->get_filename();
    MusicListChanged();
    pHeaderBar1->set_title(
        std::filesystem::path(std::string(Directory)).filename().string());
    pHeaderBar1->set_subtitle(Directory);
}

void MyApplication::on_TreeSelection1_changed() {
    if (!resorting) {
        Gtk::TreeModel::iterator iter = pTreeSelection1->get_selected();
        if (iter) {
            Gtk::TreeModel::Row row = *iter;
            int id = row[*pTreeModelColumnId];
            SelectedMusic = AllMusicCopy->at(id);
        }
    }
}

void MyApplication::on_Menu1Item_activate(Glib::ustring Label) {
    if (Label == "Play") {
        ShuffleOff();
        CurrentMusic = SelectedMusic;
        ChangeMusic();
    }
    if (Label == "Edit Properties") {

        DisplayCoverArtDialog1();
        pEntryTitle1->set_text(SelectedMusic->Title);
        pEntryTime1->set_text(SelectedMusic->DurationString);
        pEntryArtist1->set_text(SelectedMusic->Artist);
        pEntryAlbum1->set_text(SelectedMusic->Album);
        pEntryFileName1->set_text(SelectedMusic->FileName);
        pDialog1->show();
    }
}

bool MyApplication::on_TreeView1_button_press_event(
    GdkEventButton* button_event) {
    if (!TreeViewColumnClicked) {
        if ((button_event->type == GDK_BUTTON_PRESS) &&
            (button_event->button == 3))
            pMenu1->popup_at_pointer((GdkEvent*)button_event);

        if ((button_event->type == GDK_2BUTTON_PRESS)) {
            ShuffleOff();
            if (IsPlaying)
                PauseMusic();
            CurrentMusic = SelectedMusic;
            ChangeMusic();
            PlayMusic();
        }
    }
    TreeViewColumnClicked = false;
    return false;
}

bool MyApplication::on_Scale1_press_event(GdkEventButton* event) {
    if (state == PLAYING) {
        SystemTogglingPlayButton = true;
        PauseMusic();
        SystemTogglingPlayButton = false;
    }
    ScaleIsBeingMoved = true;
    return false;
}

bool MyApplication::on_Scale1_release_event(GdkEventButton* event) {
    ScaleIsBeingMoved = false;
    return false;
}

void MyApplication::on_Adjustment1_changed() {
    pLabel1->set_text(MyApplication::TimeString(int(pScale1->get_value())));
    GstNeedSeek = true;
    CurrentPosInMilliseconds = pScale1->get_value();
    if (ScaleIsBeingMoved)
        ResetLyric();
}

void MyApplication::taglib_get_data(MusicInfoADT _music) {
    TagLib::FileRef f(_music->FilePath.c_str());
    _music->Title = Glib::ustring((!(f.tag()->title().to8Bit(true).empty())
        ? f.tag()->title().to8Bit(true)
        : "None"));
    _music->SortTitle = _music->Title.lowercase();
    _music->Album = Glib::ustring((!(f.tag()->album().to8Bit(true).empty())
        ? f.tag()->album().to8Bit(true)
        : "None"));
    _music->Artist = Glib::ustring((!(f.tag()->artist().to8Bit(true).empty())
        ? f.tag()->artist().to8Bit(true)
        : "None"));
    _music->Duration = f.audioProperties()->length();
    _music->DurationInMilliseconds =
        f.audioProperties()->lengthInMilliseconds();
    _music->DurationString = TimeString(_music->DurationInMilliseconds);
    if (_music->Extension == ".mp3")
        _music->CoverArt = !(dynamic_cast<TagLib::MPEG::File*>(f.file())
            ->ID3v2Tag()
            ->frameListMap()["APIC"]
            .isEmpty());
    else if (_music->Extension == ".m4a")
        _music->CoverArt = !(dynamic_cast<TagLib::MP4::File*>(f.file())
            ->tag()
            ->item("covr")
            .toCoverArtList()
            .isEmpty());
    else
        _music->CoverArt = false;
}

void MyApplication::on_TreeViewColumn_Clicked(TreeViewColumns NewSortColumn) {
    TreeViewColumnClicked = true;
    if (NewSortColumn != ICON) {
        resorting = true;
        Gtk::SortType NewSortOrder =
            (SortColumn == NewSortColumn
                ? (SortOrder == Gtk::SortType::SORT_ASCENDING
                    ? Gtk::SortType::SORT_DESCENDING
                    : Gtk::SortType::SORT_ASCENDING)
                : Gtk::SortType::SORT_ASCENDING);
        SortColumn = NewSortColumn;
        SortMusicListByIndex(NewSortColumn, NewSortOrder);
        for (int x = 0; x < pTreeView1->get_n_columns(); x++)
            pTreeView1->get_column(x)->set_sort_indicator(false);
        update_tree_model();
        pTreeView1->get_column(NewSortColumn)->set_sort_indicator(true);
        pTreeView1->get_column(NewSortColumn)->set_sort_order(NewSortOrder);
        Gtk::TreeRow SelectedRow = pListStore1->children()[GetSortIndex(
            SelectedMusic, NewSortColumn, NewSortOrder)];
        pTreeSelection1->select(SelectedRow);
        pTreeView1->scroll_to_row(pListStore1->get_path(SelectedRow));
        SortOrder = NewSortOrder;
        resorting = false;
    }
}

int MyApplication::GetSortIndex(MusicInfoADT _music, TreeViewColumns SortColumn,
    Gtk::SortType SortOrder) {
    int index = -1;
    switch (SortColumn) {
    case TITLE:
        index = _music->SortTitleAlphIndex;
        break;
    case TIME:
        index = _music->SortDurationInMillisecondsIndex;
        break;
    case ARTIST:
        index = _music->SortArtistAlphIndex;
        break;
    case ALBUM:
        index = _music->SortAlbumAlphIndex;
        break;
    case FILENAME:
        index = _music->SortFileNameAlphIndex;
        break;
    }
    return (SortOrder == Gtk::SortType::SORT_ASCENDING
        ? index
        : (AllMusicCopy->size() - 1 - index));
}

void MyApplication::LoadMusic() {
    if (CurrentMusic->Extension == ".wav" && CurrentMusic->CanonicalWAV)
        if (!wav->openWavFile(CurrentMusic->FilePath.c_str()))
            CurrentMusic->CanonicalWAV = false;

    if (CurrentMusic->Extension != ".wav" || !CurrentMusic->CanonicalWAV) {
        Glib::ustring command =
            "playbin uri=\"file:///" + CurrentMusic->FilePath + "\"";
        pipeline = gst_parse_launch(command.c_str(), NULL);

        spectrum = gst_element_factory_make("spectrum", "spectrum");
        sink = gst_element_factory_make("autoaudiosink", "audio_sink");
        if (!spectrum || !sink) {
            g_printerr("Not all elements could be created.\n");
        }

        bin = gst_bin_new("audio_sink_bin");
        gst_bin_add_many(GST_BIN(bin), spectrum, sink, NULL);
        gst_element_link_many(spectrum, sink, NULL);

        pad = gst_element_get_static_pad(spectrum, "sink");
        ghost_pad = gst_ghost_pad_new("sink", pad);
        gst_pad_set_active(ghost_pad, TRUE);
        gst_element_add_pad(bin, ghost_pad);
        gst_object_unref(pad);
        g_object_set(G_OBJECT(spectrum), "bands", 128, "interval", 50000000,
            NULL);

        g_object_set(GST_OBJECT(pipeline), "audio-sink", bin, NULL);

        char* cwd = getcwd(cwd, 0); // ???
        // if(false) Glib::setenv("", NULL, 0); // ???
    }
}

void MyApplication::ChangeMusic() {
    PauseMusic();
    DisplayCoverArtSidebar();
    pLabelTitle1->set_label("Title:\t" + PrettyString(CurrentMusic->Title, 30));
    pLabelDuration1->set_label("Duration:\t" + CurrentMusic->DurationString);
    pLabelArtist1->set_label("Artist:\t" +
        PrettyString(CurrentMusic->Artist, 30));
    pLabelAlbum1->set_label("Album:\t" + PrettyString(CurrentMusic->Album, 30));
    pLabelFileName1->set_label("Filename:\t" +
        PrettyString(CurrentMusic->FileName, 30));

    CurrentPosInMilliseconds = 0;
    PlayedInMilliseconds = 0;
    pLabel1->set_text(TimeString(CurrentPosInMilliseconds));
    pLabel2->set_text(CurrentMusic->DurationString);
    pAdjustment1->set_lower(0);
    pAdjustment1->set_value(0);
    pAdjustment1->set_upper(CurrentMusic->DurationInMilliseconds);
    GstNeedSeek = false;
    if (CurrentMusic->Id >= 0) {
        LoadMusic();
        if (CurrentMusic->LRC) {
            // I hope this reset works
            LrcFile.reset(new Lrc(CurrentMusic->LRCFilePath.c_str()));
            Lyrics = LrcFile->GetAllLyrics();
            ResetLyric();

        }
        else {
            LyricIndex = 0;
            LyricEndtime = 0;
        }
    }
    UpdatingLyrics = true;
    update_tree_model_lyric();
    UpdatingLyrics = false;
}

void MyApplication::PauseMusic() {
    SystemTogglingPlayButton = true;
    pButtonPlay1->set_active(false);
    SystemTogglingPlayButton = false;
    state = PAUSED;
    pButtonPlay1->set_label("Play");
    pButtonPlay1_Img->set_from_resource("/my_app/start.png");

    if (IsPlaying && CurrentMusic->Id >= 0) {
        gst_element_set_state(pipeline, GST_STATE_PAUSED);
        IsPlaying = false;
        if (CurrentMusic->Extension == ".wav" && CurrentMusic->CanonicalWAV) {
            gst_object_unref(bus);
            gst_element_set_state(pipeline, GST_STATE_NULL);
            gst_object_unref(pipeline);
        }
    }
}

void MyApplication::PlayMusic() {
    if (!IsPlaying && CurrentMusic->Id >= 0) {
        IsPlaying = true;
        if (state == PAUSED) {
            SystemTogglingPlayButton = true;
            pButtonPlay1->set_active(true);
            SystemTogglingPlayButton = false;
        }
        state = PLAYING;
        pButtonPlay1->set_label("Pause");
        pButtonPlay1_Img->set_from_resource("/my_app/pause.png");

        if (CurrentMusic->Extension != ".wav" || !CurrentMusic->CanonicalWAV) {
            if (GstNeedSeek) {
                gst_element_seek_simple(
                    pipeline, GST_FORMAT_TIME,
                    GstSeekFlags(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT),
                    CurrentPosInMilliseconds * GST_MSECOND);
                GstNeedSeek = false;
            }
            gst_stream_volume_set_volume(GST_STREAM_VOLUME(pipeline),
                GST_STREAM_VOLUME_FORMAT_LINEAR,
                Volume);
            GstVolumeChanged = false;

            gst_element_set_state(pipeline, GST_STATE_PLAYING);
            bus = gst_element_get_bus(pipeline);

        }
        else {
            std::string command =
                "appsrc name=src ! rawaudioparse  format=pcm num-channels=" +
                std::to_string(wav->getNumChannels()) +
                " sample-rate=" + std::to_string(wav->getSampleRate()) +
                " pcm-format=GST_AUDIO_FORMAT_S" +
                std::to_string(wav->getBitsPerSample()) +
                "LE ! volume name=vol ! level ! audioconvert ! audioresample ! "
                "spectrum interval=50000000 bands=128 ! autoaudiosink  "
                "--gst-debug=2";
            pipeline = gst_parse_launch(command.c_str(), NULL);
            appsrc = gst_bin_get_by_name(GST_BIN(pipeline), "src");
            volume = (GstStreamVolume*)gst_bin_get_by_name(GST_BIN(pipeline),
                "vol");

            const int MaxBufSize = 2048;
            int pos = 0, bufSize = 0;

            unsigned long long int offset =
                CurrentPosInMilliseconds / 1000 * (wav->getSampleRate()) *
                (wav->getNumChannels()) * (wav->getBitsPerSample()) / 8;
            unsigned long long int offsetChunkSize =
                wav->getSubchunk2Size() - offset;

            bool Continue = true;
            while (Continue) {
                if (offsetChunkSize - pos > MaxBufSize)
                    bufSize = MaxBufSize;
                else {
                    bufSize = offsetChunkSize - pos;
                    Continue = false;
                }
                GstBuffer* buffer =
                    gst_buffer_new_allocate(NULL, bufSize, NULL);

                gst_buffer_fill(buffer, 0, wav->getData() + offset + pos,
                    bufSize);

                gst_app_src_push_buffer(GST_APP_SRC(appsrc), buffer);
                pos += MaxBufSize;
            }

            gst_app_src_end_of_stream(GST_APP_SRC(appsrc));
            gst_stream_volume_set_volume(
                volume, GST_STREAM_VOLUME_FORMAT_LINEAR, Volume);
            GstVolumeChanged = false;

            PlayedInMilliseconds = CurrentPosInMilliseconds;
            CurrentPosInMilliseconds = 0;

            gst_element_set_state(pipeline, GST_STATE_PLAYING);
            bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
        }
    }
    else
        PauseMusic();
}

void MyApplication::SortMusicListByIndex(TreeViewColumns SortColumn,
    Gtk::SortType SortOrder) {
    switch (SortColumn) {
    case TITLE:
        std::sort(AllMusic->begin(), AllMusic->end(),
            (SortOrder == Gtk::SortType::SORT_ASCENDING
                ? CompareByTitleAlphabeticalOrderIndexAscending
                : CompareByTitleAlphabeticalOrderIndexDescending));
        break;
    case TIME:
        std::sort(AllMusic->begin(), AllMusic->end(),
            (SortOrder == Gtk::SortType::SORT_ASCENDING
                ? CompareByDurationInMillisecondsIndexAscending
                : CompareByDurationInMillisecondsIndexDescending));
        break;
    case ARTIST:
        std::sort(AllMusic->begin(), AllMusic->end(),
            (SortOrder == Gtk::SortType::SORT_ASCENDING
                ? CompareByArtistAlphabeticalOrderIndexAscending
                : CompareByArtistAlphabeticalOrderIndexDescending));
        break;
    case ALBUM:
        std::sort(AllMusic->begin(), AllMusic->end(),
            (SortOrder == Gtk::SortType::SORT_ASCENDING
                ? CompareByAlbumAlphabeticalOrderIndexAscending
                : CompareByAlbumAlphabeticalOrderIndexDescending));
        break;
    case FILENAME:
        std::sort(AllMusic->begin(), AllMusic->end(),
            (SortOrder == Gtk::SortType::SORT_ASCENDING
                ? CompareByFileNameAlphabeticalOrderIndexAscending
                : CompareByFileNameAlphabeticalOrderIndexDescending));
        break;
    }
}

bool MyApplication::CompareByTitleAlphabeticalOrder(const MusicInfoADT& a,
    const MusicInfoADT& b) {
    return (a->SortTitle < b->SortTitle);
}

void MyApplication::SortMusicListByTitleAlphabeticalOrder() {
    std::sort(AllMusic->begin(), AllMusic->end(),
        CompareByTitleAlphabeticalOrder);
    for (int counter = 0; counter < AllMusic->size(); counter++)
        AllMusic->at(counter)->SortTitleAlphIndex = counter;
}

bool MyApplication::CompareByTitleAlphabeticalOrderIndexAscending(
    const MusicInfoADT& a, const MusicInfoADT& b) {
    return (a->SortTitleAlphIndex < b->SortTitleAlphIndex);
}

bool MyApplication::CompareByTitleAlphabeticalOrderIndexDescending(
    const MusicInfoADT& a, const MusicInfoADT& b) {
    return (a->SortTitleAlphIndex > b->SortTitleAlphIndex);
}

bool MyApplication::CompareByDurationInMilliseconds(const MusicInfoADT& a,
    const MusicInfoADT& b) {
    if (a->DurationInMilliseconds != b->DurationInMilliseconds)
        return (a->DurationInMilliseconds < b->DurationInMilliseconds);
    else
        return CompareByTitleAlphabeticalOrder(a, b);
}

void MyApplication::SortMusicListByDurationInMilliseconds() {
    std::sort(AllMusic->begin(), AllMusic->end(),
        CompareByDurationInMilliseconds);
    for (int counter = 0; counter < AllMusic->size(); counter++)
        AllMusic->at(counter)->SortDurationInMillisecondsIndex = counter;
}

bool MyApplication::CompareByDurationInMillisecondsIndexAscending(
    const MusicInfoADT& a, const MusicInfoADT& b) {
    return (a->SortDurationInMillisecondsIndex <
        b->SortDurationInMillisecondsIndex);
}

bool MyApplication::CompareByDurationInMillisecondsIndexDescending(
    const MusicInfoADT& a, const MusicInfoADT& b) {
    return (a->SortDurationInMillisecondsIndex >
        b->SortDurationInMillisecondsIndex);
}

bool MyApplication::CompareByAlbumAlphabeticalOrder(const MusicInfoADT& a,
    const MusicInfoADT& b) {
    if (a->Album != b->Album)
        return (a->Album.lowercase()) < (b->Album.lowercase());
    else
        return CompareByTitleAlphabeticalOrder(a, b);
}

void MyApplication::SortMusicListByAlbumAlphabeticalOrder() {
    std::sort(AllMusic->begin(), AllMusic->end(),
        CompareByAlbumAlphabeticalOrder);
    for (int counter = 0; counter < AllMusic->size(); counter++)
        AllMusic->at(counter)->SortAlbumAlphIndex = counter;
}

bool MyApplication::CompareByAlbumAlphabeticalOrderIndexAscending(
    const MusicInfoADT& a, const MusicInfoADT& b) {
    return (a->SortAlbumAlphIndex < b->SortAlbumAlphIndex);
}

bool MyApplication::CompareByAlbumAlphabeticalOrderIndexDescending(
    const MusicInfoADT& a, const MusicInfoADT& b) {
    return (a->SortAlbumAlphIndex > b->SortAlbumAlphIndex);
}

bool MyApplication::CompareByArtistAlphabeticalOrder(const MusicInfoADT& a,
    const MusicInfoADT& b) {
    if (a->Artist != b->Artist)
        return (a->Artist.lowercase()) < (b->Artist.lowercase());
    else
        return CompareByAlbumAlphabeticalOrder(a, b);
}

void MyApplication::SortMusicListByArtistAlphabeticalOrder() {
    std::sort(AllMusic->begin(), AllMusic->end(),
        CompareByArtistAlphabeticalOrder);
    for (int counter = 0; counter < AllMusic->size(); counter++)
        AllMusic->at(counter)->SortArtistAlphIndex = counter;
}

bool MyApplication::CompareByArtistAlphabeticalOrderIndexAscending(
    const MusicInfoADT& a, const MusicInfoADT& b) {
    return (a->SortArtistAlphIndex < b->SortArtistAlphIndex);
}

bool MyApplication::CompareByArtistAlphabeticalOrderIndexDescending(
    const MusicInfoADT& a, const MusicInfoADT& b) {
    return (a->SortArtistAlphIndex > b->SortArtistAlphIndex);
}

bool MyApplication::CompareByFileNameAlphabeticalOrder(const MusicInfoADT& a,
    const MusicInfoADT& b) {
    return (a->SortFileName < b->SortFileName);
}

void MyApplication::SortMusicListByFileNameAlphabeticalOrder() {
    std::sort(AllMusic->begin(), AllMusic->end(),
        CompareByFileNameAlphabeticalOrder);
    for (int counter = 0; counter < AllMusic->size(); counter++)
        AllMusic->at(counter)->SortFileNameAlphIndex = counter;
}

bool MyApplication::CompareByFileNameAlphabeticalOrderIndexAscending(
    const MusicInfoADT& a, const MusicInfoADT& b) {
    return (a->SortFileNameAlphIndex < b->SortFileNameAlphIndex);
}

bool MyApplication::CompareByFileNameAlphabeticalOrderIndexDescending(
    const MusicInfoADT& a, const MusicInfoADT& b) {
    return (a->SortFileNameAlphIndex > b->SortFileNameAlphIndex);
}

bool MyApplication::timeout1() {

    if (IsPlaying) {
        if (GstVolumeChanged) {
            if (CurrentMusic->Extension != ".wav" ||
                !CurrentMusic->CanonicalWAV)
                gst_stream_volume_set_volume((GstStreamVolume*)(pipeline),
                    GST_STREAM_VOLUME_FORMAT_LINEAR,
                    Volume);
            else
                g_object_set(volume, "volume", Volume, NULL);
            GstVolumeChanged = false;
        }
        msg = gst_bus_timed_pop_filtered(bus, 0 * GST_MSECOND,
            GstMessageType(GST_MESSAGE_ERROR |
                GST_MESSAGE_EOS |
                GST_MESSAGE_ELEMENT));
        if (msg != NULL && GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS) {
            if (PlayMode == SINGLE) {
                PauseMusic();
                pAdjustment1->set_value(0);
            }
            else
                on_ButtonNext1_clicked();
            gst_message_unref(msg);
        }
        else {
            if (msg != NULL && GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ELEMENT) {
                const GstStructure* s = gst_message_get_structure(msg);
                update_spectrum_data(s);
                gst_message_unref(msg);
            }
            else if (msg != NULL)
                gst_message_unref(msg);

            gst_element_query_position(pipeline, GST_FORMAT_TIME,
                &CurrentPosInMilliseconds);
            CurrentPosInMilliseconds /= 1000000;
            pLabel1->set_text(
                TimeString(PlayedInMilliseconds + CurrentPosInMilliseconds));
            pAdjustment1->set_value(PlayedInMilliseconds +
                CurrentPosInMilliseconds);
            if (CurrentMusic->LRC && CurrentPosInMilliseconds > LyricEndtime &&
                LyricIndex != Lyrics.size() - 1) {
                LyricIndex++;
                LyricEndtime = Lyrics[LyricIndex].endms;
                Gtk::TreeRow SelectedRow = pListStore2->children()[LyricIndex];
                pTreeSelection2->select(SelectedRow);
                pTreeView2->scroll_to_row(pListStore2->get_path(SelectedRow));
            }
        }
    }
    return true;
}

void MyApplication::DisplayCoverArtSidebar() {
    if (CurrentMusic->CoverArt && CurrentMusic->Extension == ".mp3") {
        auto file = TagLib::MPEG::File(CurrentMusic->FilePath.c_str());
        auto AttachedPictureFrame =
            (TagLib::ID3v2::AttachedPictureFrame*)*file.ID3v2Tag()
            ->frameListMap()["APIC"]
            .begin();
        std::string CoverArtFileName;
        if (AttachedPictureFrame->mimeType() == "image/jpeg")
            CoverArtFileName = "CoverArt.jpg";
        else if (AttachedPictureFrame->mimeType() == "image/png")
            CoverArtFileName = "CoverArt.png";
        std::ofstream fout(CoverArtFileName.c_str(),
            std::ios::out | std::ios::binary);
        fout.write((const char*)AttachedPictureFrame->picture().data(),
            AttachedPictureFrame->picture().size());
        fout.close();
        pImageCoverArt1->set(Glib::RefPtr<Gdk::Pixbuf>(
            Gdk::Pixbuf::create_from_file(CoverArtFileName)
            ->scale_simple(100, 100, Gdk::InterpType::INTERP_BILINEAR)));
        std::filesystem::remove(CoverArtFileName);
    }

    else if (CurrentMusic->CoverArt && CurrentMusic->Extension == ".m4a") {
        TagLib::MP4::CoverArt CoverArt =
            TagLib::MP4::File(CurrentMusic->FilePath.c_str())
            .tag()
            ->item("covr")
            .toCoverArtList()
            .front();
        std::string CoverArtFileName;
        switch (CoverArt.format()) {
        case TagLib::MP4::CoverArt::Format::BMP:
            CoverArtFileName = "CoverArt.bmp";
            break;
        case TagLib::MP4::CoverArt::Format::GIF:
            CoverArtFileName = "CoverArt.gif";
            break;
        case TagLib::MP4::CoverArt::Format::JPEG:
            CoverArtFileName = "CoverArt.jpeg";
            break;
        case TagLib::MP4::CoverArt::Format::PNG:
            CoverArtFileName = "CoverArt.png";
            break;
        }
        std::ofstream fout(CoverArtFileName.c_str(),
            std::ios::out | std::ios::binary);
        fout.write(CoverArt.data().data(), CoverArt.data().size());
        fout.close();
        pImageCoverArt1->set(Glib::RefPtr<Gdk::Pixbuf>(
            Gdk::Pixbuf::create_from_file(CoverArtFileName)
            ->scale_simple(100, 100, Gdk::InterpType::INTERP_BILINEAR)));
        std::filesystem::remove(CoverArtFileName);
    }

    else
        pImageCoverArt1->set(CurrentMusic->pIcon);
}

void MyApplication::DisplayCoverArtDialog1() {
    if (SelectedMusic->CoverArt && SelectedMusic->Extension == ".mp3") {
        auto file = TagLib::MPEG::File(SelectedMusic->FilePath.c_str());
        auto AttachedPictureFrame =
            (TagLib::ID3v2::AttachedPictureFrame*)*file.ID3v2Tag()
            ->frameListMap()["APIC"]
            .begin();
        std::string CoverArtFileName;
        if (AttachedPictureFrame->mimeType() == "image/jpeg")
            CoverArtFileName = "CoverArt.jpg";
        else if (AttachedPictureFrame->mimeType() == "image/png")
            CoverArtFileName = "CoverArt.png";
        std::ofstream fout(CoverArtFileName.c_str(),
            std::ios::out | std::ios::binary);
        fout.write((const char*)AttachedPictureFrame->picture().data(),
            AttachedPictureFrame->picture().size());
        fout.close();
        pImageCoverArt2->set(Glib::RefPtr<Gdk::Pixbuf>(
            Gdk::Pixbuf::create_from_file(CoverArtFileName)
            ->scale_simple(400, 400, Gdk::InterpType::INTERP_BILINEAR)));
        std::filesystem::remove(CoverArtFileName);
    }

    else if (SelectedMusic->CoverArt && SelectedMusic->Extension == ".m4a") {
        TagLib::MP4::CoverArt CoverArt =
            TagLib::MP4::File(SelectedMusic->FilePath.c_str())
            .tag()
            ->item("covr")
            .toCoverArtList()
            .front();
        std::string CoverArtFileName;
        switch (CoverArt.format()) {
        case TagLib::MP4::CoverArt::Format::BMP:
            CoverArtFileName = "CoverArt.bmp";
            break;
        case TagLib::MP4::CoverArt::Format::GIF:
            CoverArtFileName = "CoverArt.gif";
            break;
        case TagLib::MP4::CoverArt::Format::JPEG:
            CoverArtFileName = "CoverArt.jpeg";
            break;
        case TagLib::MP4::CoverArt::Format::PNG:
            CoverArtFileName = "CoverArt.png";
            break;
        }
        std::ofstream fout(CoverArtFileName.c_str(),
            std::ios::out | std::ios::binary);
        fout.write(CoverArt.data().data(), CoverArt.data().size());
        fout.close();
        pImageCoverArt2->set(Glib::RefPtr<Gdk::Pixbuf>(
            Gdk::Pixbuf::create_from_file(CoverArtFileName)
            ->scale_simple(400, 400, Gdk::InterpType::INTERP_BILINEAR)));
        std::filesystem::remove(CoverArtFileName);
    }

    else
        pImageCoverArt2->set(SelectedMusic->pIcon);
}

void MyApplication::on_ButtonDialog1Cancel_clicked() { pDialog1->hide(); }
void MyApplication::on_ButtonDialog1Save_clicked() {
    std::string NewTitle = std::string(pEntryTitle1->get_text()),
        NewArtist = std::string(pEntryArtist1->get_text()),
        NewAlbum = std::string(pEntryAlbum1->get_text()),
        NewFileName = std::string(pEntryFileName1->get_text());

    boost::smatch smatch;
    if (boost::regex_match(NewTitle, smatch, boost::regex("[ -~]*")) &&
        boost::regex_match(NewArtist, smatch, boost::regex("[ -~]*")) &&
        boost::regex_match(NewAlbum, smatch, boost::regex("[ -~]*")) &&
        boost::regex_match(NewFileName, smatch,
            boost::regex("[a-zA-Z0-9 ()_,.'-]*"))) {
        pDialog1->hide();
        PauseMusic();
        taglib_set_data(SelectedMusic, pEntryTitle1->get_text(),
            pEntryArtist1->get_text(), pEntryAlbum1->get_text(),
            pEntryFileName1->get_text());
        MusicListChanged();
    }
    else
        pMessageDialog1->show();
}

void MyApplication::taglib_set_data(MusicInfoADT _music, Glib::ustring NewTitle,
    Glib::ustring NewArtist,
    Glib::ustring NewAlbum,
    Glib::ustring NewFileName) {
    std::string NewFilePath =
        std::filesystem::path(std::string(_music->FilePath))
        .parent_path()
        .string() +
        "/" + NewFileName + SelectedMusic->Extension;
    std::filesystem::rename(
        std::filesystem::path(std::string(_music->FilePath)),
        std::filesystem::path(NewFilePath));
    if (_music->LRC) {
        std::string NewLRCFilePath =
            std::filesystem::path(_music->LRCFilePath).parent_path().string() +
            "/" + NewFileName + ".lrc";
        std::filesystem::rename(std::filesystem::path(_music->LRCFilePath),
            std::filesystem::path(NewLRCFilePath));
    }
    TagLib::FileRef f(NewFilePath.c_str());
    f.tag()->setTitle(TagLib::String(NewTitle));
    f.tag()->setArtist(TagLib::String(NewArtist));
    f.tag()->setAlbum(TagLib::String(NewAlbum));
    f.save();
}

void MyApplication::ResetLyric() {
    if (CurrentMusic->Id >= 0 && CurrentMusic->LRC) {
        LyricIndex = LrcFile->GetLyricIndex(CurrentPosInMilliseconds);
        LyricEndtime = Lyrics.at(LyricIndex).endms;
        Gtk::TreeRow SelectedRow = pListStore2->children()[LyricIndex];
        pTreeSelection2->select(SelectedRow);
        pTreeView2->scroll_to_row(pListStore2->get_path(SelectedRow));
    }
}

void MyApplication::update_tree_model_lyric() {
    pListStore2->clear();
    if (CurrentMusic->LRC)
        for (int counter = 0; counter < Lyrics.size(); counter++) {
            Gtk::TreeModel::Row row = *(pListStore2->append());
            row[*pTreeModelColumnId2] = counter;
            row[*pTreeModelColumnLyric] = Lyrics[counter].s1;
        }
    else {
        Gtk::TreeModel::Row row = *(pListStore2->append());
        row[*pTreeModelColumnLyric] = "<No Lyrics Available>";
    }
}

void MyApplication::on_TreeSelection2_changed() {
    if (!UpdatingLyrics) {
        Gtk::TreeModel::iterator iter = pTreeSelection2->get_selected();
        if (iter) {
            Gtk::TreeModel::Row row = *iter;
            int id = row[*pTreeModelColumnId2];
            SelectedLyricIndex = id;
        }
    }
}

bool MyApplication::on_TreeView2_button_press_event(
    GdkEventButton* button_event) {
    if (!UpdatingLyrics)
        if (CurrentMusic->LRC && button_event->type == GDK_2BUTTON_PRESS) {
            bool OriginalIsPlaying = IsPlaying;
            PauseMusic();
            CurrentPosInMilliseconds = Lyrics[SelectedLyricIndex].startms;
            pLabel1->set_text(TimeString(CurrentPosInMilliseconds));
            ScaleIsBeingMoved = true;
            pAdjustment1->set_value(CurrentPosInMilliseconds);
            ScaleIsBeingMoved = false;
            if (OriginalIsPlaying)
                PlayMusic();
        }
    return false;
}

void MyApplication::on_MessageDialog1_response(int response_id) {
    if (response_id == Gtk::RESPONSE_CLOSE)
        pMessageDialog1->hide();
}

// can this function run regardless of whether the user has opened a folder?
// also can I asynchronously add things to the completion list??
bool MyApplication::on_EntryCompletion1_match(
    const Glib::ustring& key, const Gtk::TreeModel::const_iterator& iter) {

    // now here is the list of tracks that matches key, where can I add to the
    // results? I can convert it to a MusicInfoCDT if needed
    std::vector<Track> tracks = store.search(key);

    if (iter) {
        Gtk::TreeRow row = *iter;
        if (Glib::ustring(row[*pTreeModelColumnTitle])
            .lowercase()
            .find(key.lowercase()) != Glib::ustring::npos ||
            Glib::ustring(row[*pTreeModelColumnArtist])
            .lowercase()
            .find(key.lowercase()) != Glib::ustring::npos ||
            Glib::ustring(row[*pTreeModelColumnAlbum])
            .lowercase()
            .find(key.lowercase()) != Glib::ustring::npos)
            return true;
    }
    return false;
}

bool MyApplication::on_EntryCompletion1_match_selected(
    const Gtk::TreeModel::const_iterator& iter) {
    if (iter) {
        pSearchEntry1->set_text("");
        bool OriginalIsPlaying = IsPlaying;
        Gtk::TreeRow row = *iter;
        ShuffleOff();
        CurrentMusic = AllMusicCopy->at(row[*pTreeModelColumnId]);
        ChangeMusic();
        if (OriginalIsPlaying)
            PlayMusic();
        return true;
    }
    return false;
}

void MyApplication::update_spectrum_data(const GstStructure* s) {
    const gchar* name = gst_structure_get_name(s);
    if (strcmp(name, "spectrum") != 0)
        return;

    const GValue* magnitudes_value = gst_structure_get_value(s, "magnitude");

    bool changed = false;

    for (guint i = 0; i < spect_bands; ++i) {
        const GValue* mag = gst_value_list_get_value(magnitudes_value, i);

        if (mag != NULL || changed) {
            magnitudes.push_back(g_value_get_float(mag));
            if (!changed) {
                changed = true;
                magnitudes.erase(magnitudes.begin(),
                    magnitudes.begin() + spect_bands);
            }
        }
        else {
            magnitudes.push_back(-60);
        }
    }

    // if(changed && !done_draw){
    //     std::cout << "droped\n";
    // }
    if (changed && done_draw) {
        done_draw = false;
        pDrawingArea1->queue_draw();
    }
}

bool MyApplication::on_DrawingArea1_draw(
    const Cairo::RefPtr<Cairo::Context>& cr, const GdkEventExpose* event) {

    // Get the dimensions of the drawing area widget
    int width = pDrawingArea1->Gtk::Widget::get_allocated_width();
    int height = pDrawingArea1->Gtk::Widget::get_allocated_height();

    // Set the background color to white
    cr->set_source_rgb(1.0, 1.0, 1.0);
    cr->rectangle(0, 0, width, height);
    cr->fill();

    // random color
    static double r = 0.5, g = 0.5, b = 0.5;
    r += 0.1 * ((double)rand() / RAND_MAX - 0.5);
    g += 0.1 * ((double)rand() / RAND_MAX - 0.5);
    b += 0.1 * ((double)rand() / RAND_MAX - 0.5);
    r = r < 0.1 ? 0.1 : (r > 0.9 ? 0.9 : r);
    g = g < 0.1 ? 0.1 : (g > 0.9 ? 0.9 : g);
    b = b < 0.1 ? 0.1 : (b > 0.9 ? 0.9 : b);
    cr->set_source_rgb(r, g, b);

    std::vector<double> value;
    for (guint i = 0; i < spect_bands; ++i) {
        double v = 0;
        for (int j = 0; j < 5; j++)
            v += magnitudes[i + ((4 - j) * spect_bands)] / (j * 2 + 1);
        v /= 1.0 + 1.0 / 3 + 1.0 / 5 + 1.0 / 7 + 1.0 / 9;
        v += 60;
        value.push_back(v);
    }

    for (int i = 0; i < spect_bands; i += 4) {
        double avg;
        if (i + 3 < spect_bands)
            avg = (value[i] + value[i + 1] + value[i + 2] + value[i + 3]) / 4;
        else
            avg = value[i];
        value.insert(value.begin(), avg);
    }

    for (int i = 0; i < value.size(); ++i) {
        cr->rectangle((int)i * round((double)width / value.size()),
            height / 2.0 - round(value[i]) * height / 100.0,
            (int)round((double)width / value.size()),
            (round(value[i]) * height / 50.0) == 0
            ? 1
            : (round(value[i]) * height / 50.0));
        cr->fill();
    }

    cr->stroke();

    done_draw = true;
    return true;
}

Glib::ustring MyApplication::PrettyString(const Glib::ustring& str,
    const int MaxLength) {
    if (str.length() > MaxLength)
        return (str.substr(0, MaxLength - 3) + "...");
    else
        return str;
}

void MyApplication::on_ButtonAddIp1_clicked() {
    std::string NewIp = std::string(pEntryIp1->get_text());
    boost::smatch smatch;
    // now the regex becomes:
    // match the first three octets xxx.xxx.xxx.
    // match the last octet xxx
    // capture the four octets xxx.xxx.xxx.xxx
    // match the port (if there is one) (captured)
    // so smatch will be { full, four octets, port }
    if (boost::regex_match(
        NewIp, smatch,
        boost::regex(
            "^((?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?\\.)"
            "{3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)"
            "):?(\\d{4,5})?$"))) // ^((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.?\b){4}$
    {
        std::string port_no = "4000";
        // I added this line, so that when a new ip is added, it will attempt
        // to connect to that ip immediately
        // also on port 4000 because why not
        if (smatch[2].length() != 0) {
            port_no = smatch[2];
        }
        // try to get the online database immediatey when the connection is
        // established
        bool success = client->connect_to_peer(smatch[1], port_no);
        // if the ip cannot be resolved, don't add it to the ip list
        if (!success) {
            return;
        }
        IpsChanged = true;
        pEntryIp1->set_text("");
        NetworkIps.push_back(NewIp);
        update_tree_model3();
    }
}

void MyApplication::on_ButtonRemoveIp1_clicked() {
    Gtk::TreeModel::iterator iter = pTreeSelection3->get_selected();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        int id = row[*pTreeModelColumnId3];
        // sorry how do I get the ip address here?
        // is it like that?
        // nevermind I think it is
        auto ip = NetworkIps.at(id);
        std::cout << ip << std::endl;
        std::string port = "4000";
        std::string address = ip;
        auto pos = ip.find(":");
        // has a port
        if (pos != std::string::npos) {
            address = ip.substr(0, pos);
            port = ip.substr(pos + 1, std::string::npos);
        }
        client->remove_socket_by_ip(address, std::stoi(port));
        NetworkIps.erase(std::next(NetworkIps.begin(), id));
        update_tree_model3();
    }
}

void MyApplication::on_ButtonRemoveAllIp1_clicked() {
    // this removes the connections one by one
    for (auto& n : NetworkIps) {
        client->remove_socket_by_ip(n, 4000);
    }
    NetworkIps = {};
    update_tree_model3();
}

void MyApplication::update_tree_model3() {
    std::cout << "update_tree_model3" << std::endl;
    pListStore3->clear();
    for (int counter = 0; counter < NetworkIps.size(); counter++) {
        Gtk::TreeModel::Row row = *(pListStore3->append());
        row[*pTreeModelColumnId3] = counter;
        row[*pTreeModelColumnIp] = NetworkIps.at(counter);
        std::cout << counter << std::endl;
    }
}

// network / application related functions
void MyApplication::start_client(uint16_t port) {
    // I really hope by capturing the class instance in this way
    // handle message actually works
    // if not we are screwed (looks like we are not scrwed yet :) )
    // here we are passing handle_message to ApplicationClient
    // so that when ApplicationClient has messages, handle_message will be
    // invoked
    client = std::make_unique<ApplicationClient>(
        // capturing MyApplication, don't know how much cpying is done
        // but who cares?
        port, [this](MessageWithOwner& t) { handle_message(t); },
        [this](peer_id id) { on_connect(id); },
        [this](peer_id id) { on_disconnect(id); });
}

void MyApplication::handle_message(MessageWithOwner& t) {
    // see which type of message the incoming message is
    switch (t.msg.header.type) {
    case MessageType::GET_LYRICS:
        handle_get_lyrics(t);
        break;
    case MessageType::RETURN_LYRICS:
        handle_return_lyrics(t);
        break;
    case MessageType::NO_SUCH_LYRICS:
        handle_no_such_lyrics(t);
        break;
    case MessageType::GET_DATABASE:
        handle_get_database(t);
        break;
    case MessageType::RETURN_DATABASE:
        handle_return_database(t);
        break;

        // these messages are for streaming audio files
        // but I have no idea how to stream audio files!
    case MessageType::GET_AUDIO_FILE:
    case MessageType::PREPARE_AUDIO_SHARING:
    case MessageType::PREPARED_AUDIO_SHARING:
    case MessageType::NO_SUCH_AUDIO_FILE:
    case MessageType::HAS_AUDIO_FILE:
    case MessageType::GET_AUDIO_SEGMENT:
    case MessageType::RETURN_AUDIO_SEGMENT:

        // so we also don't handle it
        // these cases do not have to handled, they are only used in the
        // interleaving images example
    case MessageType::PREPARE_PICTURE_SHARING:
    case MessageType::PREPARED_PICTURE_SHARING:
    case MessageType::HAS_PICTURE_FILE:
    case MessageType::GET_PICTURE_SEGMENT:
    case MessageType::RETURN_PICTURE_SEGMENT:
    case MessageType::NO_SUCH_PICTURE_SEGMENT:
        // we also don't handle these two
    case MessageType::GET_TRACK_INFO:
    case MessageType::RETURN_TRACK_INFO:
    case MessageType::NO_SUCH_TRACK:
        // these cases are just for sanity testing, they are also not used in
        // the real application
    case MessageType::PING:
    case MessageType::PONG:
    case MessageType::NOTHING:
        break;
    }
}

void MyApplication::on_connect(peer_id id) {
    Message m(MessageType::GET_DATABASE);
    client->push_message(id, m);
}

void MyApplication::on_disconnect(peer_id id) { remove_network_tracks(id); }

void MyApplication::ask_client_for_file_with_this_checksum(
    std::string checksum) {
    Message m(MessageType::GET_AUDIO_FILE);
    GetAudioFile gaf;
    gaf.checksum = checksum;
    m << gaf;
    auto it = network_tracks.find(checksum);
    // no file in the network tracks has this checksum
    if (it == network_tracks.end()) {
        return;
    }
    for (auto& id : it->second.ids) {
        client->push_message(id, m);
    }
}

// NOTE: this handler is invoked when ANOTHER PEER is getting your database
void MyApplication::handle_get_database(MessageWithOwner& t) {
    ReturnDatabase rd;
    rd.tracks = store.read_all();
    Message m(MessageType::RETURN_DATABASE);
    m << rd;
    client->push_message(t.id, m);
    // now t.msg.tracks will have all the searched tracks from the peers
    // where should I put the tracks to?
}

// NOTE: this handler is invoked when ANOTHER PEER returns you with their
// database
void MyApplication::handle_return_database(MessageWithOwner& t) {
    ReturnDatabase rd;
    t.msg >> rd;
    std::cout << "Here are the results from client " << t.id << std::endl;
    // append the tracks from peer to the network tracks vector
    for (auto& r : rd.tracks) {
        std::cout << r << std::endl;
        // already has this track!
        auto it = network_tracks.find(r.checksum);
        if (it != network_tracks.end()) {
            it->second.ids.push_back(t.id);
        }
        else {
            network_tracks[r.checksum] = {
                .ids = {t.id},
                .track = r,
            };
        }
    }
}

void MyApplication::remove_network_tracks(peer_id id) {
    std::cout << "[REMOVE NETWORK TRACKS] matching: " << id << std::endl;
    for (auto it = network_tracks.begin(); it != network_tracks.end();) {
        std::cout << it->second.track << std::endl;
        std::cout << "Ids: ";
        for (auto& id : it->second.ids) {
            std::cout << id << " ";
        }
        std::cout << std::endl;
        // remove that peer id from the ids array
        it->second.ids.erase(
            std::find(it->second.ids.begin(), it->second.ids.end(), id));
        // if the array has no ids, that means no peer owns that track
        // remove it
        if (it->second.ids.empty()) {
            std::cout << "Removed this record " << it->second.track
                << std::endl;
            network_tracks.erase(it++);
        }
        else {
            ++it;
        }
    }
}

// NOTE: this handler is invoked when ANOTHER PEER returns some track info to
// you
// void MyApplication::handle_return_track_info(MessageWithOwner &t) {
//     ReturnTrackInfo ti;
//     t.msg >> ti;
//     // now t.msg.tracks will have all the searched tracks from the peers
//     // where should I put the tracks to?
// }

// NOTE: this handler handles the case of a peer NOT HAVING THAT TRACK
// in this case it does nothing
// void MyApplication::handle_no_such_track(MessageWithOwner &t) {
//     NoSuchTrack nt;
//     t.msg >> nt;
//     std::cout << nt.title << "is not owned by client " << t.id << std::endl;
// }

// NOTE: this handler is invoked when ANOTHER PEER asks you for a track
// void MyApplication::handle_get_track_info(MessageWithOwner &t) {
//     GetTrackInfo gti;
//     t.msg >> gti;
//     auto results = store.search(gti.title);
//     if (results.empty()) {
//         NoSuchTrack nst;
//         Message m(MessageType::NO_SUCH_TRACK);
//         nst.title = gti.title;
//         m << nst;
//         client->push_message(t.id, m);
//     } else {
//         Message m(MessageType::RETURN_TRACK_INFO);
//         // message class allows pushing the vectors into it
//         ReturnTrackInfo rti{.tracks = results, .title = gti.title};
//         m << rti;
//         client->push_message(t.id, m);
//     }
// }

// NOTE: this handler is invoked when ANOTHER PEER asks you for lyrics
void MyApplication::handle_get_lyrics(MessageWithOwner& t) {
    GetLyrics gl;
    t.msg >> gl;
    Lrc f(gl.filename.c_str());
    if (f.failed()) {
        NoSuchLyrics nsl{ gl.filename };
        Message m(MessageType::NO_SUCH_LYRICS);
        m << nsl;
        client->push_message(t.id, m);
    }
    else {
        Message m(MessageType::RETURN_LYRICS);
        ReturnLyrics rl{
            .lyrics = f,
            .filename = gl.filename,
        };
        m << rl;
        client->push_message(t.id, m);
    }
}

// NOTE: this is invoked when ANOTHER PEER returns you some lyrics
void MyApplication::handle_return_lyrics(MessageWithOwner& t) {
    ReturnLyrics rl;
    t.msg >> rl;
    if (rl.lyrics.failed()) {
        std::cout << "The lyrics file failed for some reason." << std::endl;
        return;
    }

    // should I set the lyrics like this?
    LrcFile = std::make_unique<Lrc>(rl.lyrics);
}

// NOTE: this is invoked when ANOTHER PEER doesn't have that lyrics
// in this case it does nothing
void MyApplication::handle_no_such_lyrics(MessageWithOwner& t) {
    NoSuchLyrics nl;
    t.msg >> nl;
}
