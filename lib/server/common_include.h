#pragma once

#include <memory>
#include <queue>
#include <thread>
#include <mutex>
#include <iostream>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

enum class msg_type : uint32_t
{
    DownloadTrack,
    GetList,
    UploadTrack,
    NoTrackExists
};

typedef struct track_info
{
    //char[10] name;
    size_t index;
} track_list_elem;



