#include "scene_replayer.h"

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/file_access.hpp>

using namespace godot;

void SceneReplayer::_bind_methods() {
}

SceneReplayer::SceneReplayer() {
    
}

SceneReplayer::~SceneReplayer() {
}

void SceneReplayer::_ready() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    set_process_mode(ProcessMode::PROCESS_MODE_ALWAYS);
    get_tree()->set_pause(true);

    PackedByteArray rdata = FileAccess::get_file_as_bytes("rec.bin");
    std::string serialized;
    if (!rdata.is_empty()) {
        serialized.assign(reinterpret_cast<const char*>(rdata.ptr()), rdata.size());
    }
    if (replay.ParseFromString(serialized)) {
        print_line("LOADED");
    } else {
        print_line("FAILED TO LOAD");
    }
    print_line("Replay has " + String::num(replay.frames_size()) + " frames");
    if (replay.frames_size() > 0 && replay.frames(0).nodes_size() > 0) {
        const auto& node = replay.frames(0).nodes(0);
        print_line(node.path())        print_line(node.x());
        print_line(node.y());
        print_line(node.z());
    }
}

void SceneReplayer::_process(double delta) {
    if (frame < 0 || frame >= replay.frames_size()) {
        return;
    }

    const auto& recorded_frame = replay.frames(frame);

    for (int i = 0; i < recorded_frame.nodes_size(); i++) {
        const auto& node = recorded_frame.nodes(i);
        Node* child_node = get_child(i);
        auto child = Object::cast_to<Node3D>(child_node);
        child->set_position(Vector3(node.x(), node.y(), node.z()));
        child->set_rotation(Vector3(node.rx(), node.ry(), node.rz()));
        child->set_scale(Vector3(node.sx(), node.sy(), node.sz()));
    }

    frame++;
}
