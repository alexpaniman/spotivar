#include "client.h"
#include "FLAC_track_info.h"
#include "common_include.h"
#include "axp/exceptions.h"
#include <stdlib.h>
#include <filesystem>



//---------------------------------------------------SUPPORT FUNCTIONS--------------------------------------------------//

void install_track(std::vector<uint8_t> track) {
    int pos = 0;
    std::string path = "";

    while (track[pos] != '\0' && pos < track.size()){
        path.push_back(track[pos]);
        pos ++;
    }

    FILE* file = fopen(path.c_str(), "w");

    if (file == nullptr || track[pos] != '\0')
        throw axp::exception{"Failed to install\n"};

    size_t track_size = track.size() - (pos + 1);

    //writing song's bytes into file//
    fwrite(track.data() + pos + 1, sizeof(char), track_size, file);

    //closing file at end//
    fclose(file);
};

size_t get_file_size (FILE* file){
    assert (file != NULL);
    int size = 0;
    int current_pos = ftell (file);

    fseek (file, 0, SEEK_END);
    size = ftell (file);
    fseek (file, 0, current_pos);

    return size;
}

size_t get_extension_flag(std::string extension){
    for (int i = 0; i < EXTENSIONS_SIZE; i ++) {
        if (!strcmp(extension.c_str(), extensions[i].c_str()))
            return i;
    }

    //returns .flac flag by default//
    return 0; 
};

//-------------------------------------------------------------------------------------------------------------//



//-------------------------------------------CLIENT CLASS FUNCTION---------------------------------------------//


sptv::spotivar_net_client::spotivar_net_client(const std::string host, const uint16_t port) {
    try {
        boost::asio::ip::tcp::resolver resolver(context);
        boost::asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(host, std::to_string(port));

        to_server_conn = std::make_unique <sptv::connection> (
        sptv::connection::owner::client,
        context,
        boost::asio::ip::tcp::socket(context),
        deq_messages_in);

        //connect to the server//
        to_server_conn->connect_to_server(endpoint);

        //expect read validation just on connection//
        //it is blocking for client, so no other actions are possable before validation is done//
        to_server_conn->read_server_validation();

    } catch (...) {
        throw axp::nested_exception{"Failed on start, restart your app"};
    }

    //starts context thread for async io//
    thr_context = std::thread([this]() {
    try {
        context.run();
    }
    catch(...) {
        if (!to_server_conn->is_connected())
            throw axp::nested_exception{"Lost connection to server"};
        else 
            throw axp::nested_exception{"Unexpected error"};
    } });
};

sptv::spotivar_net_client::~spotivar_net_client() {
    context.stop();

    if (thr_context.joinable())
        thr_context.join();

    if (is_connected()){
        to_server_conn->disconnect();
    }

    //why should i reset unique_ptr to avoid memory leaks??//
    std::cout << to_server_conn << "\n";
    to_server_conn.reset();
};

bool sptv::spotivar_net_client::is_connected() {
    if (to_server_conn)
        return to_server_conn->is_connected();
    else 
        return false;
};

void sptv::spotivar_net_client::download_track(const std::string dest_path, size_t track_indx) { 
    message<msg_type> download_request;

    download_request.header.id = msg_type::DownloadTrack;
    download_request << dest_path;
    download_request << track_indx;

    to_server_conn->send(download_request);

    bool wait_status = true;

    try {
        while (wait_status) {
            // block untill recieve //
            deq_messages_in.wait();

            header_id id = deq_messages_in.back().msg.header.id;

            if (id == msg_type::NoTrackExists) {
                //pop message from deq//
                deq_messages_in.pop_back();

                throw axp::exception{"No track on server with index {}", track_indx};
            }
            else if (id == msg_type::Error404) {
                //pop message from deq//
                deq_messages_in.pop_back();

                throw axp::exception{"System error on server 404, call Aleksey for help"};
            }
            else {
                owned_message<msg_type> msg = deq_messages_in.pop_back();

                //track recieved successfuly, ready to install//
                install_track(msg.msg.body);

                wait_status = false;
            }
        }
    } catch (...) {
        throw axp::nested_exception{"Failed to download\n"};
    }
};

std::vector<FLAC_track_info> sptv::spotivar_net_client::get_tracks_list() {
    message<msg_type> list_request;
    list_request.header.id = msg_type::GetList;

    to_server_conn->send(list_request);

    while (deq_messages_in.empty() || 
    deq_messages_in.back().msg.header.id != msg_type::GetList) {}; // block untill recieve

    owned_message<msg_type> reply = deq_messages_in.pop_back();

    size_t msg_size      = reply.msg.body.size();
    size_t num_of_tracks = msg_size / sizeof(FLAC_track_info);

    std::cout << "List vector size is " << num_of_tracks << "\n";

    std::vector<FLAC_track_info> track_list(num_of_tracks);

    for (int i = 0; i < num_of_tracks; i ++){
        reply.msg >> track_list[i];
    }

    return track_list;
};

void sptv::spotivar_net_client::upload_track (const std::string path) {
    sptv::message<msg_type> track;
    track.header.id = msg_type::UploadTrack;

    std::filesystem::path path_to_track(path);
    track.header.ex_flag = get_extension_flag(path_to_track.extension().generic_string());

    FILE*  file      = fopen(path.c_str(), "r");

    if (file == nullptr)
        throw axp::exception {"Failed to open file on line {}", __LINE__};

    size_t file_size = get_file_size(file);

    track.body.resize(file_size);
    fread(track.body.data(), sizeof(char), file_size, file);
    fclose(file);

    track.header.size = track.size();
    to_server_conn->send(track);
};

//-----------------------------------------------------------------------------------------------------------------//