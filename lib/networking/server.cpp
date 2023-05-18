#include "server.h"
#include "common_include.h"
#include "axp/exceptions.h"


// TODO: Why named in caps?
bool CHECK_NOT_SYSTEM(std::string file_name) {
    if (!strcmp(file_name.c_str(), ".DS_Store"))
        return false;
    else if (!strcmp(file_name.c_str(), "indexed.txt"))
        return false;
    else         
        return true;
};

bool CHECK_IF_INDEXED(std::string lib_path) {
    for (const auto& entry : std::filesystem::directory_iterator(lib_path)) {
        if (!strcmp(entry.path().filename().c_str(), "indexed.txt"))
            return true;
    }
    return false;
}

//-----------------------------------------------SUPPORT FUNCTIONS----------------------------------------------------//

size_t get_file_size(FILE* file){
    assert (file != NULL);
    int size = 0;
    int current_pos = ftell (file);

    fseek (file, 0, SEEK_END);
    size = ftell (file);
    fseek (file, 0, current_pos);

    return size;
};


void sign_as_indexed(std::string lib_path) {
    //create indexed file path//
    std::string file_path;
    file_path += lib_path;
    file_path += "indexed.txt";

    //open to write, it creates file if do not exist//
    FILE* file = fopen(file_path.c_str(), "w");

    if (file == nullptr)
        throw axp::exception {"Failed to open file, path: {}", file_path.c_str()};

    //write a short message//
    size_t sign_size = 128;
    fwrite("This directory was indexed by sptv server! Do not remove this file!\n", \
    sizeof(char), sign_size, file);

    fclose(file);
}

//--------------------------------------------------------------------------------------------------------------------//


//-------------------------------------------------BASE METHODS-------------------------------------------------------//

sptv::spotivar_server::spotivar_server(const std::string lib_path, uint16_t port)
    : asio_acceptor(context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)), track_lib_path(lib_path){
    
    //post a job to run a context event loop//
    async_new_connection();

    //run the event loop in a new thread, at least one event should exist//
    thread_context = std::thread([this]() {
        try {
            context.run();
        } catch(std::exception& exp) {
            //logging the error
            std::cout << "UNEXPECTED ERROR " << exp.what() << "\n";

            //restart event loop//
            context.stop();
            context.reset();

            //connect new users//
            async_new_connection();
            context.run();
        }
    });
    
    std::cout << "[SERVER] started\n";
};

sptv::spotivar_server::~spotivar_server(){
    context.stop();

    if (thread_context.joinable())
        thread_context.join();

    std::cout << "[SERVER] Stopped!\n";
};

void sptv::spotivar_server::async_new_connection(){ 
    asio_acceptor.async_accept(
        [this](std::error_code ec, boost::asio::ip::tcp::socket socket) {
            if (!ec) {
                std::cout << "New Connect " << socket.remote_endpoint() << "\n";
                uint32_t port_num = socket.remote_endpoint().port();

                //create new connection acossiated with new user// 
                std::shared_ptr<connection> new_connection = std::make_shared<connection>(
                connection::owner::server,
                context, std::move(socket),
                deq_message_in, this);

                //adding client into storage map//
                connections_map[port_num] = new_connection;
                //validating client...//
                connections_map[port_num]->validate_client(id_counter++);

                //connect new users//
                async_new_connection();
            }
            else {
                std::cout << "falls here\n";
                std::cout << "Accept failed from " << socket.remote_endpoint() << "\n";

                //closing connection//
                socket.close();

                throw axp::exception{"Failed to accept new user {}", ec.message()};
            }
        }
        );
};

void sptv::spotivar_server::operate_user_requests(){
    //wait for new users request or if there are some already//
    deq_message_in.wait();

    //iterating through whole non empty message deq//
    while (!deq_message_in.empty()) { 
        auto msg = deq_message_in.pop_front();

        //complete actions according to incomming msg type//
        on_message_recieved(msg.remote, msg.msg);
    }
};

void sptv::spotivar_server::index_track_library() {
    //check if directory is already indexed
    if (CHECK_IF_INDEXED(track_lib_path))
        return;

    for (const auto& entry : std::filesystem::directory_iterator(track_lib_path)) {
        std::string file_name = entry.path().filename().generic_string();

        if (CHECK_NOT_SYSTEM(file_name)){
            //get hash from file's samples to index
            uint64_t track_hash = my_hash; //axp_hash(entry.path());
            my_hash ++;

            //create new path
            std::string str_new_path;
            str_new_path += track_lib_path;
            str_new_path += std::to_string(track_hash);
            str_new_path += entry.path().extension().generic_string();
            std::cout << "new path: " << str_new_path << "\n";

            //prepare to copy from old to new
            std::filesystem::path new_path(str_new_path);
            std::filesystem::path old_path;
            old_path = entry.path();

            //copy
            std::filesystem::copy_file(old_path, new_path);
            //delete old file
            std::filesystem::remove(old_path);
        }
    }

    //create indexed.txt file to avoid double indexing
    sign_as_indexed(track_lib_path);
};
//-----------------------------------------------------------------------------------------------------------------//


//-----------------------------------------------SENDING FUNCTIONS-------------------------------------------------//

void sptv::spotivar_server::send_track(std::shared_ptr<connection> client, message<msg_type>& request) {
    size_t track_indx;
    request >> track_indx;

    sptv::message<msg_type> track;
    track.header.id = msg_type::DownloadTrack;
    track << request.body;

    std::string file_name;
    std::string file_extension = extensions[request.header.ex_flag];

    file_name += track_lib_path;
    file_name += std::to_string(track_indx);
    file_name += file_extension;

    std::cout << "Client ID: [" << client->get_id() << "] Request to download " << file_name.c_str() << "\n";
    FILE* file_ptr = fopen(file_name.c_str(), "r");

    if (file_ptr == nullptr)
        throw axp::exception {"Failed to open file, path: {}", file_name.c_str()};

    size_t track_size  = get_file_size(file_ptr);

    std::vector<uint8_t> temp_vector;
    temp_vector.resize(track_size);

    fread(temp_vector.data(), sizeof(char), track_size, file_ptr);
    fclose(file_ptr);

    track << temp_vector;

    client->send(track);
};

void sptv::spotivar_server::send_tracks_list(std::shared_ptr<connection> client){
    message<msg_type> track_list;
    track_list.header.id = msg_type::GetList;

    for (const auto& entry : std::filesystem::directory_iterator(track_lib_path)){
        std::string file_name = entry.path().filename().generic_string();

        if (CHECK_NOT_SYSTEM(file_name)) {
            FLAC_track_info track = {};
            FLAC_read_track_info(entry.path().c_str(), &track);

            size_t info_size = FLAC_info_size(&track);
            std::cout << info_size << "\n";

            track_list << track;
        }
    }

    client->send(track_list);
};

void sptv::spotivar_server::send_no_track(std::shared_ptr<connection> client){
    message<msg_type> reply;
    reply.header.id = msg_type::NoTrackExists;

    client->send(reply);
};

void sptv::spotivar_server::send_user_404_somethig_went_wrong(std::shared_ptr<connection> client) {
    message<msg_type> error;
    error.header.id = msg_type::Error404;

    client->send(error); 
};


//--------------------------------------------------------------------------------------------------------------------//


//--------------------------------------------FILESYSTEM SERVER BACKEND-----------------------------------------------// 
void sptv::spotivar_server::add_to_library(const std::vector<uint8_t>& track) {
    std::string file_path;
    file_path += track_lib_path;
    file_path += "new.flac";

    FILE*  file = fopen(file_path.c_str(), "w");
    
    if (file == nullptr)
        throw axp::exception {"Failed to open file, path {}, line {}", file_path.c_str(), __LINE__};
    
    size_t track_size = track.size();

    fwrite(track.data(), sizeof(char), track_size, file);
    fclose(file);
};

bool sptv::spotivar_server::check_if_exists(std::string file_extension) {
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
    return false; 

    std::string track_path;
    track_path += track_lib_path;
    track_path += "new.flac";

    uint64_t track_hash = 1;  //generate_hash_from_sound(track_path);

    std::string track_check_path;
    track_check_path += track_lib_path;
    track_check_path += std::to_string(track_hash);
    track_check_path += file_extension;

    //try to open a file//
    int file_d = open(track_check_path.c_str(), O_WRONLY);
    
    if (file_d > 0) {
        close(file_d);
        return true;
    }
    else
        return false;
};

bool sptv::spotivar_server::check_if_exists(size_t index, std::string file_extension) {
    std::string file_path;
    file_path += track_lib_path;
    file_path += std::to_string(index);
    file_path += file_extension;

    //try to open a file//
    int file_d = open(file_path.c_str(), O_WRONLY); 
    
    if (file_d > 0)
        return true;
    else 
        return false;
};

void sptv::spotivar_server::index_new_track(std::string file_extension){
    std::string   track_path;
    track_path += track_lib_path;
    track_path += "new.flac";

    uint64_t track_hash = my_hash; //generate_hash_from_sound(track_path);

    std::string new_track_path;
    new_track_path += track_lib_path;
    new_track_path += std::to_string(track_hash);
    new_track_path += file_extension;

    int error_code = rename(track_path.c_str(), new_track_path.c_str());

    if (error_code != 0)
        throw axp::exception {"Failed to rename file, path {}", track_path.c_str()};

    my_hash++;
};

void sptv::spotivar_server::remove_new() {
    std::string track_path_to_remove;
    track_path_to_remove += track_lib_path;
    track_path_to_remove += "new.flac";

    bool result_stat = std::filesystem::remove(track_path_to_remove.c_str());

    if (!result_stat)
        throw axp::exception {"Failed to remove file {}", track_path_to_remove.c_str()};

};

//--------------------------------------------------------------------------------------------------------------------//


//-----------------------------------------------ON ACTION FUNCTIONS--------------------------------------------------//

void sptv::spotivar_server::on_download_request(std::shared_ptr<connection> client, sptv::message<msg_type>& msg){
    try {
        size_t index;
        msg >> index;

        std::string file_extension = extensions[msg.header.ex_flag];

        if (check_if_exists(index, file_extension)){
            msg << index;
            send_track(client, msg);
        }
        else 
            send_no_track(client);
    } catch (...) {
        std::cout << "Failed on download\n";
        throw axp::nested_exception {"Failed on download"};
    }
};

void sptv::spotivar_server::on_upload_request(std::shared_ptr<connection> client, sptv::message<msg_type>& msg){
    try {
        std::cout << "Client ID: [" << client->get_id() << "] Request to upload\n";
        add_to_library(msg.body);

        std::string file_extension = extensions[msg.header.ex_flag];

        if (!check_if_exists(file_extension))
            index_new_track(file_extension);
        else
            remove_new();
    } catch (...) {
        std::cout << "Failed on upload track\n";
        throw axp::nested_exception {"Failed on upload"};
    }
};

void sptv::spotivar_server::on_message_recieved(std::shared_ptr<connection> client, sptv::message<msg_type>& msg){
    try {
        switch (msg.header.id){
            case msg_type::DownloadTrack:
                on_download_request(client, msg);
            break;

            case msg_type::UploadTrack: 
                on_upload_request(client, msg);
            break;

            case msg_type::GetList:
                send_tracks_list(client);
            break;
        }
    } catch (...) {
        std::cout << "[" << client->get_id() << "] Failed on operate user request, msg info: " << msg;
        

        //server finished with unexpected error//
        send_user_404_somethig_went_wrong(client);
    }
};


//--------------------------------------------------------------------------------------------------------------------//
