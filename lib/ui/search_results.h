#pragma once

#include "session.h"

class search_result {
public:
	search_result(tracklist_t by_track_metadata, tracklist_t by_track_tags):
		by_track_metadata_(by_track_metadata), by_track_tags_(by_track_tags) {}
	tracklist_t by_track_metadata_;
	tracklist_t by_track_tags_;
};

