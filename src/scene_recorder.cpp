#include "scene_recorder.h"

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/file_access.hpp>

using namespace godot;

void SceneRecorder::_bind_methods() {
}

SceneRecorder::SceneRecorder() {
    
}

SceneRecorder::~SceneRecorder() {
}

void SceneRecorder::_ready() {
   GOOGLE_PROTOBUF_VERIFY_VERSION;
   auto* frame = rec.add_frames();
   auto* node = frame->add_nodes();
   node->set_x(1.0);
   node->set_y(2.0);
   node->set_z(3.0);

   Ref<FileAccess> f = FileAccess::open("rec.bin", FileAccess::WRITE);
   if (f.is_null()) {
		return;
	}
    auto data = rec.SerializeAsString();
    f->store_buffer((const uint8_t*)data.data(), data.size());
    f->flush();
    f->close();

    demo::Recording replay;
    PackedByteArray rdata = FileAccess::get_file_as_bytes("rec.bin");
    std::string serialized;
    if (!rdata.is_empty()) {
        serialized.assign(reinterpret_cast<const char *>(rdata.ptr()), rdata.size());
    }
    if (replay.ParseFromString(serialized)) {
        print_line("LOADED");
    } else {
        print_line("FAILED TO LOAD");
    }
    print_line("Replay has " + String::num(replay.frames_size()) + " frames");
    if (replay.frames_size() > 0 && replay.frames(0).nodes_size() > 0) {
        const auto& node = replay.frames(0).nodes(0);
        print_line(node.x());
        print_line(node.y());
        print_line(node.z());
    }
}

void SceneRecorder::_process(double delta) {
   
    
}
