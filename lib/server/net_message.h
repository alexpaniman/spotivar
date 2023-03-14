#pragma once
#include "common_include.h"


//This messages support random protocols aka msg id
// They are easy to work with, because they support any standart data types

namespace sptv
{
    template <typename T>
    struct message_header
    {
        T id{};
        uint32_t size = 0;
    };

    template <typename T>
    struct message
    {
        message_header<T> header{};
        std::vector<uint8_t> body;

        size_t size() const
        {
            return sizeof(message_header<T>) + body.size();
        }

        friend std::ostream& operator << (std::ostream& os, const message<T>& message)
        {
            os << "ID " << int(message.header.id) << " " << "Size: " << message.header.size << "\n";
            return os;
        } 

        //this will help me to easily push any standart data in my message
        template <typename DataType>
        friend message<T>& operator << (message<T>& msg, const DataType& data){
            static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed");

            size_t i = msg.body.size(); //storing current size of a data

            msg.body.resize(i + sizeof(DataType));
            std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

            //here we update message size
            msg.header.size = msg.size();

            return msg; //so now i can chain << operators with my messages;
        }

        template <typename DataType>
        friend message<T>& operator << (message<T>& msg, const std::vector<DataType> buffer){
            size_t i = msg.body.size();

            msg.body.resize(i + buffer.size()*sizeof(DataType));
            std::memcpy (msg.body.data() + i, buffer.data(), sizeof(DataType)*buffer.size());

            msg.header.size = msg.size();

            return msg;
        }

        friend message<T>& operator << (message<T>& msg, const std::string string){
            size_t i = msg.body.size();

            msg.body.resize(i + string.size() + 1);
            std::memcpy (msg.body.data() + i, string.data(), string.size() + 1);

            msg.header.size = msg.size();

            return msg;
        }

        template <typename DataType>
        friend message<T>& operator >> (message<T>& msg, DataType& data){
            static_assert(std::is_standard_layout<DataType>::value, "Data can not be popped, its too complex");

            size_t i = msg.body.size() - sizeof(DataType);

            std::memcpy (&data, msg.body.data() + i, sizeof(DataType));
            msg.body.resize(i);

            msg.header.size = msg.size();

            return msg; 
        }
    };

    template <typename T>
    class connection;
    //{};


    template <typename T> 
    struct owned_message
    {
        std::shared_ptr<connection <T>> remote = nullptr;
        message<T> msg;

        friend std::ostream& operator << (std::ostream& os, const owned_message<T>& msg)
        {
            os << msg.msg;
            return msg;
        }
    };
}

 