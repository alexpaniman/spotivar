#pragma once
#include "common_include.h"
#include "flac-reader/FLAC_track_info.h"


namespace sptv {
    template <typename T>
    struct message_header {
        T id{};
        uint32_t size = 0;
        uint8_t ex_flag = 0;
    };

    template <typename T>
    struct message {
        message_header<T> header{};
        std::vector<uint8_t> body;

        size_t size() const {
            return sizeof(message_header<T>) + body.size();
        }

        friend std::ostream& operator << (std::ostream& os, const message<T>& message)
        {
            os << "ID " << int(message.header.id) << " " << "Size: " << message.header.size << "\n";
            return os;
        } 


        template <typename DataType>
        friend message<T>& operator << (message<T>& msg, const DataType& data) {

            //storing current size of a data
            size_t i = msg.body.size();

            msg.body.resize(i + sizeof(DataType));
            std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

            //here we update message size
            msg.header.size = msg.size();

            return msg;
        }

        template <typename DataType>
        friend message<T>& operator << (message<T>& msg, const std::vector<DataType>& buffer){

            size_t i = msg.body.size();

            msg.body.resize(i + buffer.size()*sizeof(DataType));
            std::memcpy (msg.body.data() + i, buffer.data(), sizeof(DataType)*buffer.size());

            msg.header.size = msg.size();

            return msg;
        };

        friend message<T>& operator << (message<T>& msg, const std::string& string) {
            size_t i = msg.body.size();

            msg.body.resize(i + string.size());
            std::memcpy (msg.body.data() + i, string.data(), string.size());

            msg.header.size = msg.size();

            return msg;
        };

        template <typename DataType>
        friend message<T>& operator >> (message<T>& msg, DataType& data) {

            assert(msg.body.size() >= sizeof(DataType));

            size_t pos = msg.body.size() - sizeof(DataType);

            std::memcpy (&data, msg.body.data() + pos, sizeof(DataType));
            msg.body.resize(pos);

            msg.header.size = msg.size();

            return msg; 
        }

        template <typename DataType>
        friend message<T>& operator >> (message<T>& msg, std::vector<DataType>& buffer){
            size_t buffer_size = buffer.size();
            buffer_size *= sizeof(DataType);

            // std::cout << "the buffer size is " << buffer_size << "\n";
            // std::cout << "the msg body size is " << msg.body.size() << "\n";

            assert(msg.body.size() >= buffer_size);

            size_t pos = msg.body.size() - buffer_size;
            std::memcpy(buffer.data(), msg.body.data() + pos, buffer_size);

            msg.body.resize(pos);
            msg.header.size = msg.size();
            
            return msg;
        }

        friend message<T>& operator >> (message<T>& msg, std::string& string) {
            size_t string_size = string.size();
            
            assert(msg.body.size() >= string_size);

            size_t pos = msg.body.size() - string_size;
            std::memcpy(string.data(), msg.body.data() + pos, string_size);

            msg.body.resize(pos);
            msg.header.size = msg.size();

            return msg;
        }

        friend message<T>& operator >> (message<T>& msg, FLAC_track_info& track_info) {
            size_t album_artist_size = 0;
            msg >> album_artist_size;
            std::string album_artist(album_artist_size, 0);
            msg >> album_artist;
            track_info.album_artist = album_artist;

            size_t genre_size = 0;
            msg >> genre_size;
            std::string genre(genre_size, 0);
            msg >> genre;
            track_info.genre = genre;

            size_t date_size = 0;
            msg >> date_size;
            std::string date(date_size, 0);
            msg >> date;
            track_info.date = date;

            size_t comment_size = 0;
            msg >> comment_size;
            std::string comment(comment_size, 0);
            msg >> comment;
            track_info.comment = comment;

            size_t album_size = 0;
            msg >> album_size;
            std::string album(album_size, 0);
            msg >> album;
            track_info.album = album;

            size_t artist_size = 0;
            msg >> artist_size;
            std::string artist(artist_size, 0);
            msg >> artist;
            track_info.artist = artist;

            size_t title_size = 0;
            msg >> title_size;
            std::string title(title_size, 0);
            msg >> title;
            track_info.title = title;

            msg >> track_info.hash;

            return msg;
        }
        friend message<T>& operator << (message<T>& msg, FLAC_track_info& track_info) {
            msg << track_info.hash;

            msg << track_info.title;
            msg << track_info.title.size();
            
            msg << track_info.artist;
            msg << track_info.artist.size();

            msg << track_info.album;
            msg << track_info.album.size();

            msg << track_info.comment;
            msg << track_info.comment.size();

            msg << track_info.date;
            msg << track_info.date.size();

            msg << track_info.genre;
            msg << track_info.genre.size();

            msg << track_info.album_artist;
            msg << track_info.album_artist.size();

            return msg;
        }
    };

    //forward declaration
    class connection;

    template <typename T>
    struct owned_message
    {
        std::shared_ptr<connection> remote = nullptr; //pointer to sender's connection
        message<T> msg;

        friend std::ostream& operator << (std::ostream& os, const owned_message<T>& msg) {
            os << msg.msg;
            return msg;
        }
    };
};

 
