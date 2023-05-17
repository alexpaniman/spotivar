#pragma once

#include <memory>
#include <queue>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <iostream>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

typedef enum class msg_type : uint8_t
{
    DownloadTrack,
    GetList,
    UploadTrack,
    NoTrackExists,
    Error404
} header_id;

enum class extension_flag : uint8_t {
    FLAC, //goes by default
    MP_3,
    WAV
};

static const int EXTENSIONS_SIZE = 3;
static std::string extensions[] = {
    ".flac",
    ".mp3",
    ".wav"
};

typedef struct track_info
{
    //char[10] name;
    size_t index;
} track_list_elem;



