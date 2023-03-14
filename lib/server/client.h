#pragma once

#include "common_include.h"
#include "thread_safe_queues.h"
#include "net_message.h"
#include "net_connection.h"
#include "assert.h"

//define somehow enum msg_type
namespace sptv
{
    class spotivar_net_client{

    public:
        spotivar_net_client(const std::string& host, const uint16_t port) : socket(context){
            //on construct, we want client to be imediately connected
            boost::asio::ip::tcp::resolver resolver(context);
            boost::asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(host, std::to_string(port));

            to_server_conn = std::make_unique<sptv::connection<msg_type>>(
            sptv::connection<msg_type>::owner::client,
            context,
            boost::asio::ip::tcp::socket(context), deq_messages_in);

            to_server_conn->connect_to_server(endpoint);
            thr_context = std::thread([this]() { context.run(); });
        };

        virtual ~spotivar_net_client(){
            if (is_connected()){
                to_server_conn->disconnect();
            }
            
            context.stop();

            if (thr_context.joinable())
                thr_context.join();

            to_server_conn.release();
        };


        bool is_connected(){
            if (to_server_conn)
                return to_server_conn->is_connected();
            else 
                return false;
        };

        void download_track(const std::string dest_path, size_t track_indx){ 
            message<msg_type> download_request;

            download_request.header.id = msg_type::DownloadTrack;
            download_request << dest_path;
            download_request << track_indx;

            to_server_conn->send(download_request);

            while (deq_messages_in.empty() || 
            !(deq_messages_in.back().msg.header.id != msg_type::DownloadTrack ||
              deq_messages_in.back().msg.header.id != msg_type::NoTrackExists)) {}; // block untill recieve

            owned_message<msg_type> msg = deq_messages_in.pop_back();

            if (msg.msg.header.id == msg_type::NoTrackExists)
                return; //exception later
            else
                install_track (msg.msg.body);
        };

        track_list_elem* get_tracks_list(){
            message<msg_type> list_request;
            list_request.header.id = msg_type::GetList;

            to_server_conn->send(list_request);

            while (deq_messages_in.empty() || 
            deq_messages_in.back().msg.header.id != msg_type::GetList) {}; // block untill recieve

            owned_message<msg_type> reply = deq_messages_in.pop_back();

            size_t msg_size      = reply.msg.body.size();
            size_t num_of_tracks = msg_size / sizeof(track_list_elem);
            
            track_list_elem* buffer = (track_list_elem*)calloc(msg_size, sizeof(char));
            //??maybe smart pointers instead?? how to implement??

            for (int i = 0; i < num_of_tracks; i ++){
                reply.msg >> buffer[i];
            }

            return buffer;
        };
        
        //this can be removed from class methods description
        void install_track(std::vector<uint8_t> track){
            int pos = 0;
            std::string path = "";

            while (track[pos] != '\0' && pos < track.size()){
                path.push_back(track[pos]);
                pos ++;
            }

            if (track[pos] != '\0')
                return; //EXCEPTION ALSO

            FILE* file = fopen(path.c_str(), "w");
            assert (file != NULL); //REMOVE

            size_t track_size = track.size() - (pos + 1);

            fwrite(track.data() + pos + 1, sizeof(char), track_size, file);
            fclose(file);
        };

        void upload_track (const std::string path){
            sptv::message<msg_type> track;
            track.header.id = msg_type::UploadTrack;

            FILE*  file      = fopen(path.c_str(), "r");
            size_t file_size = get_file_size(file);

            track.body.resize(file_size);
            fread(track.body.data(), sizeof(char), file_size, file);
            fclose(file);

            track.header.size = track.size();
            to_server_conn->send(track);
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
    
        //recieve_function realization is varya's job
        ts_queue<owned_message<msg_type>>& incoming (){
            return deq_messages_in;
        };

    protected:
        boost::asio::io_context               context;
        std::thread                           thr_context;  //thread is needed for asio to perform its tasks
        
        boost::asio::ip::tcp::socket          socket;
        std::unique_ptr<connection<msg_type>> to_server_conn;
    
    private:
        ts_queue<owned_message<msg_type>>     deq_messages_in;
    };
}