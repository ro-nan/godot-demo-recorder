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
}

void SceneRecorder::_process(double delta) {
    auto* frame = rec.add_frames();
    for(int i = 0; i < get_child_count(); i++) {
        auto child = Object::cast_to<Node3D>(get_child(i));
        if (child->is_class("Node3D")) {
            auto* node = frame->add_nodes();
            auto position = child->get_position();
            node->set_x(position.x);
            node->set_y(position.y);
            node->set_z(position.z);

            print_line(node->x());
        }
    }
    save_recording();
}

void SceneRecorder::save_recording() {
    Ref<FileAccess> f = FileAccess::open("rec.bin", FileAccess::WRITE);
    if (f.is_null()) {
        return;
    }
    auto data = rec.SerializeAsString();
    f->store_buffer((const uint8_t*)data.data(), data.size());
    f->flush();
    f->close();
}