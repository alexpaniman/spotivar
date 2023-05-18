#pragma once

#include "thread_safe_queues.h"

namespace sptv {
    class spotivar_server;
}

#include "connection.h"
#include "net_message.h"
#include "flac-reader/FLAC_track_info.h"
#include <assert.h>
#include <filesystem>
#include <unordered_map>

namespace sptv
{
    class spotivar_server{

    public:
        spotivar_server(const std::string lib_path, uint16_t port);

        ~spotivar_server();

        //initial methods//
        void async_new_connection();
        void operate_user_requests();
        void index_track_library();
        //---------------//


        //sending functions//
        void send_track      (std::shared_ptr<connection> client, message<msg_type>& request);
        void send_tracks_list(std::shared_ptr<connection> client);
        void send_no_track   (std::shared_ptr<connection> client);
        void send_user_404_somethig_went_wrong(std::shared_ptr<connection> client);
        //-----------------//

        //filesystem functions//
        void add_to_library (const std::vector<uint8_t>& track);
        bool check_if_exists(size_t index, std::string file_extension);
        bool check_if_exists(std::string file_extension);
        void index_new_track(std::string file_extension);
        void remove_new();
        //---------------------//


        //initial handlers//
        void on_message_recieved(std::shared_ptr<connection> client, message<msg_type>& msg);
        void on_upload_request  (std::shared_ptr<connection> client, message<msg_type>& msg);
        void on_download_request(std::shared_ptr<connection> client, message<msg_type>& msg);

        void on_client_validated        (std::shared_ptr<connection> client);
        void on_client_failed_validation(std::shared_ptr<connection> client);
        void on_client_disconnect       (std::shared_ptr<connection> client);
        //----------------//

    private:
        ts_queue<owned_message<msg_type>> deq_message_in;
        std::unordered_map<uint32_t, std::shared_ptr<connection>> connections_map;

        boost::asio::io_context context;
        std::thread             thread_context;
  
        boost::asio::ip::tcp::acceptor asio_acceptor;

        const std::string track_lib_path;

        uint64_t my_hash = 0;
        uint32_t id_counter = 10000;
};
}