#pragma once

#include "thread_safe_queues.h"
#include "net_message.h"
#include "connection.h"
#include "flac-reader/FLAC_track_info.h"
#include "assert.h"

void install_track(std::vector<uint8_t> track);

namespace sptv
{
    class spotivar_net_client {

    public:
        spotivar_net_client(const std::string host, const uint16_t port);

        ~spotivar_net_client();

        bool is_connected();

        void download_track(const std::string dest_path, size_t track_indx);

        std::vector<FLAC_track_info> get_tracks_list();
        
        void upload_track (const std::string path);

        // ts_queue<owned_message<msg_type>>& incoming (){
        //     return deq_messages_in;
        // };

    protected:
        boost::asio::io_context               context;
        std::thread                           thr_context;  //thread is needed for asio to perform its tasks
        
        //boost::asio::ip::tcp::socket          socket;       //boost::asio::ip::tcp::socket - typedef 
        std::unique_ptr<connection> to_server_conn;

        bool is_validated = false;
    
    private:
        ts_queue<owned_message<msg_type>>     deq_messages_in;
    };
};