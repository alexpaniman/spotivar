#pragma once

#include "common_include.h"
#include "thread_safe_queues.h"
#include "net_connection.h"
#include "net_message.h"
#include <assert.h>
#include <filesystem>

//TO DO:: create small lib that will nicely travel through files in derectory

namespace sptv
{
    class spotivar_server{

    public:
        spotivar_server(const std::string lib_path, uint16_t port) // add start in the server constructor, so when it inited - server starts
             : asio_acceptor(context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)), track_lib_path(lib_path){

            wait_for_client_connection();

            thread_context = std::thread([this]() { context.run(); });
            std::cout << "[SERVER] started\n";
        };

        virtual ~spotivar_server(){
            finish();
        };
         
        bool finish(){
            context.stop();

            if (thread_context.joinable())
                thread_context.join();

            std::cout << "[SERVER] Stopped!\n";

            return true;
        };

        void wait_for_client_connection(){
            asio_acceptor.async_accept(
                [this](std::error_code error, boost::asio::ip::tcp::socket socket){ //does not work, without "error" variable??
                        std::cout << "New Connect " << socket.remote_endpoint() << "\n"; //this method return ip of connection

                        std::shared_ptr<connection<msg_type>> new_connection =  //create new object - new connection
                                std::make_shared<connection<msg_type>>(connection<msg_type>::owner::server, context, std::move(socket), deq_message_in);

                        if(on_client_connect(new_connection)){
                            deq_connections.push_back(std::move(new_connection));
                            deq_connections.back()->connect_to_client(id_counter ++); // here i start readheaders

                            std::cout << "[" << deq_connections.back()->get_id() << "] Connection Approved\n";
                        } //establish connection
                        else
                            std::cout << "Connection Denied\n";

                    //anyway, start to try again connect new user
                    wait_for_client_connection();
                }
                );
        };

        void message_all_clients(const message<msg_type>& msg, std::shared_ptr<connection<msg_type>> ignor_client = nullptr){
            bool invalid_client_exists = false;

            for (auto& client : deq_connections){
                if (client && client->is_connected()){
                    if (client != ignor_client)
                        client->send(msg);
                }
                else {
                    on_client_disconnect(client);
                    client.reset();
                    invalid_client_exists = true;
                }
            }

            if (invalid_client_exists)
                deq_connections.erase(std::remove(deq_connections.begin(), deq_connections.end(), nullptr), deq_connections.end());

        };

        void send_track_on_request(std::shared_ptr<connection<msg_type>> client, message<msg_type>& request){
            size_t track_indx;
            request >> track_indx;

            sptv::message<msg_type> track;
            track.header.id = msg_type::DownloadTrack;
            track << request.body;

            std::string file_name;

            file_name += track_lib_path;
            file_name += std::to_string(track_indx);
            file_name += file_extention;

            printf ("file path is %s\n", file_name.c_str());

            FILE* file_ptr   = fopen(file_name.c_str(), "r");
            assert(file_ptr != NULL); //SAME SHIT

            size_t track_size  = get_file_size(file_ptr);

            std::vector<uint8_t> temp_vector;
            temp_vector.resize(track_size);

            fread(temp_vector.data(), sizeof(char), track_size, file_ptr);
            fclose(file_ptr);

            track << temp_vector;

            client->send(track);
        };

        void send_track_list(std::shared_ptr<connection<msg_type>> client){
            message<msg_type> track_list;
            track_list.header.id = msg_type::GetList;

            for (const auto& entry : std::filesystem::directory_iterator(track_lib_path)){
                track_list_elem track = {}; //read_metadata_from_flac(entry.path().c_str());
                uint64_t track_hash    = 0; //generate_sound_hash_from_flac(entry.path().c_str());
                //insted use regex to take out hash, to not calculate it again

                track.index = track_hash;
                track_list << track;
            }
        };

        void indexing_server_library_on_start(){
            // iterator order does not specified!!!
            // works buggy, because some hidden files might exist

            for (const auto& entry : std::filesystem::directory_iterator(track_lib_path)){
                uint64_t track_hash = my_hash; //generate_sound_hash_from_flac(entry.path().c_str());

                std::string new_path;

                new_path += track_lib_path;
                new_path += std::to_string(track_hash);
                new_path += file_extention;

                // printf ("old path: %s\n", entry.path().c_str());
                // printf ("new path: %s\n", new_path.c_str());

                std::filesystem::path old_path;
                std::filesystem::path filesystem_new_path(new_path);

                old_path = entry.path();

                //before push, understand why i need to skip existinf files?
                //becouse when i test it, invisible file might stay still after deleting, so i try to copy in existing
                //inviz file ((( 
                std::filesystem::copy(old_path, filesystem_new_path, std::filesystem::copy_options::skip_existing);
                std::filesystem::remove(old_path);

                my_hash++;
            }
        };

        void update(){
            deq_message_in.wait(); // this lock is used to prevent useless while looping

            while (!deq_message_in.empty()) { //tacking all messages from the deq
                auto msg = deq_message_in.pop_front();
                on_message(msg.remote, msg.msg);
            }
        };

        void add_to_library(const std::vector<uint8_t>& track) {
            std::string file_path;
            file_path += track_lib_path;
            file_path += "new.flac";

            FILE*  file = fopen(file_path.c_str(), "w");
            size_t track_size = track.size();

            fwrite(track.data(), sizeof(char), track_size, file);
            fclose(file);
        };

        bool check_if_exists(){
            return false; //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//

            std::string track_path = "";
            track_path += track_lib_path;
            track_path += "new.flac";

            uint64_t track_hash = 1;//generate_hash_from_sound(track_path);

            std::string track_check_path = "";
            track_check_path += track_lib_path;
            track_check_path += std::to_string(track_hash);
            track_check_path += file_extention;

            int file_d = open(track_check_path.c_str(), O_WRONLY); //try to open a file
            
            if (errno == 2) {//this errno returns if file does not exist 
                errno = 0;
                return false;
            }

            else if (file_d > 0){
                close (file_d);
                return true;
            }
            else {
                //throw exception
            }
        };

        bool check_if_exists(size_t index){
            std::string file_path = "";
            file_path += track_lib_path;
            file_path += std::to_string(index);
            file_path += file_extention;

            fprintf (stderr, "trying to open a file %s\n", file_path.c_str());
            int file_d = open(file_path.c_str(), O_WRONLY); //try to open a file
            perror("fuuuuck ");
            
            if (errno == 2){ //this errno returns if file does not exist
                errno = 0;
                return false;
            }
            
            else if (file_d > 0){
                close (file_d);
                return true;
            }
            else {
                //throw exception
            }
        };

        void index_new_track(){
            std::string   track_path; //also can be in a private field
            track_path += track_lib_path;
            track_path += "new.flac";

            uint64_t track_hash = my_hash; //generate_hash_from_sound(track_path);

            std::string new_track_path;
            new_track_path += track_lib_path;
            new_track_path += std::to_string(track_hash);
            new_track_path += file_extention;

            rename (track_path.c_str(), new_track_path.c_str());

            my_hash++;
        };

        void remove_new(){
            std::string track_path_to_remove;
            track_path_to_remove += track_lib_path;
            track_path_to_remove += "new.flac";

            remove(track_path_to_remove.c_str());
        };

        void on_download_request(std::shared_ptr<connection<msg_type>> client, message<msg_type>& msg){
            size_t index;
            msg >> index;

            if (check_if_exists(index)){
                msg << index;
                send_track_on_request(client, msg);
            }
            else 
                send_no_track_exists(client);
        };

        void on_upload_request(message<msg_type>& msg){
            add_to_library(msg.body);

            if (!check_if_exists())
                index_new_track();
            else
                remove_new();
        };

        void send_no_track_exists(std::shared_ptr<connection<msg_type>> client){
            message<msg_type> reply;
            reply.header.id = msg_type::NoTrackExists;

            fprintf (stderr, "send reply to user\n");
            client->send(reply);
        };

        //checker that will deside weather to connect user or not 
        bool on_client_connect(std::shared_ptr<connection<msg_type>> client){
            return true;
        };

        //called when client disconnect, this thing will detect did anyone disconnect
        bool on_client_disconnect(std::shared_ptr<connection<msg_type>> client){
            return false;
        };

        bool on_message(std::shared_ptr<connection<msg_type>> client, message<msg_type>& msg){
            switch (msg.header.id){
 
                case msg_type::DownloadTrack:
                    on_download_request(client, msg);
                break;

                case msg_type::UploadTrack: 
                    on_upload_request(msg);
                break;

                case msg_type::GetList:
                    send_track_list(client);
                break;

            }

            return true;
        };

        size_t get_file_size (FILE* file){
            assert (file != NULL);
            int size = 0;
            int current_pos = ftell (file);

            fseek (file, 0, SEEK_END);
            size = ftell (file);
            fseek (file, 0, current_pos);

            return size;
        };

    private:
        ts_queue<owned_message<msg_type>> deq_message_in;
        std::deque<std::shared_ptr<connection<msg_type>>> deq_connections;

        boost::asio::io_context context;
        std::thread             thread_context;
  
        boost::asio::ip::tcp::acceptor asio_acceptor; //this class will get users sockets

        const std::string track_lib_path;
        const std::string file_extention = ".flac";

        uint64_t my_hash = 0;

        uint32_t id_counter = 10000;
};
}