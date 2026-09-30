#include "recorded_scene.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <google/protobuf/util/message_differencer.h>
#include <string>

using namespace godot;

void RecordedScene::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_recording", "recording"), &RecordedScene::set_recording);
	ClassDB::bind_method(D_METHOD("is_recording"), &RecordedScene::is_recording);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "recording"), "set_recording", "is_recording");
}

RecordedScene::RecordedScene() = default;

RecordedScene::~RecordedScene() = default;

void RecordedScene::set_recording(bool p_recording) {
	if (recording == p_recording) {
		return;
	}

	recording = p_recording;
	if (initialized) {
		apply_mode();
	}
}

bool RecordedScene::is_recording() const {
	return recording;
}

void RecordedScene::_ready() {
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	initialized = true;
	set_process(true);
	apply_mode();
}

void RecordedScene::_exit_tree() {
	initialized = false;
}

void RecordedScene::apply_mode() {
	if (recording) {
		replay_data.Clear();
		frame_index = -1;
		replay_loaded = false;
		previous_nodes.clear();
		return;
	}

	set_process_mode(Node::PROCESS_MODE_ALWAYS);
	load_replay();
}

void RecordedScene::load_replay() {
	replay_data.Clear();
	frame_index = -1;
	replay_loaded = false;

	PackedByteArray bytes = FileAccess::get_file_as_bytes(filename);
	if (bytes.is_empty() || !replay_data.ParseFromArray(bytes.ptr(), bytes.size())) {
		print_line("RecordedScene: failed to load " + filename);
		return;
	}

	replay_loaded = true;
	print_line("RecordedScene: loaded " + String::num(replay_data.frames_size()) + " frames");
}

void RecordedScene::_process(double delta) {
    frame_index++;
	if (recording) {
        frame = recording_data.add_frames();
		capture_frame(this);
        if (frame_index % 5 == 0) { save_recording(); }
		return;
	}

	replay_frame();
}

void RecordedScene::capture_frame(godot::Node3D* parent) {
	for (int i = 0; i < parent->get_child_count(); i++) {
		Node3D *child = Object::cast_to<Node3D>(parent->get_child(i));
		if (!child) {
			continue;
		}

        const Vector3 position = child->get_global_position();
        const Vector3 rotation = child->get_global_rotation();
        const Vector3 scale = child->get_scale();

		demo::Recording_Frame_Node3D current;
        auto path = std::string(String(child->get_path().get_concatenated_names()).utf8().get_data()); // NOTE: Maybe make global for this if I keep having to use it? Not sure.
		current.set_path(path);
		current.set_x(position.x);
		current.set_y(position.y);
		current.set_z(position.z);
		current.set_rx(rotation.x);
		current.set_ry(rotation.y);
		current.set_rz(rotation.z);
		current.set_sx(scale.x);
		current.set_sy(scale.y);
		current.set_sz(scale.z);

		auto previous = previous_nodes.find(path);
		const bool unchanged = previous != previous_nodes.end() &&
			google::protobuf::util::MessageDifferencer::Equals(previous->second, current);

		if (!unchanged) {
		    *frame->add_nodes() = current;
            previous_nodes[path] = current;    
		}

        capture_frame(child);
	}
}

void RecordedScene::replay_frame() {
	if (!replay_loaded || frame_index >= replay_data.frames_size()) {
		return;
	}
	const demo::Recording_Frame &frame = replay_data.frames(frame_index);
	int node_index = 0;
	for (int i = 0; i < get_child_count() && node_index < frame.nodes_size(); i++) {
		const demo::Recording_Frame_Node3D &node = frame.nodes(node_index++);

        auto decoded_path = node.path();
        std::string selfname = String(get_name()).utf8().get_data();
        NodePath path(decoded_path.substr(decoded_path.find(selfname) + selfname.length() + 1).c_str()); // Get the nodepath relative to this node which is the second half of the nodepath after this nodes name and a / (eg. "root/self/that" -> "that")
        Node3D *child = Object::cast_to<Node3D>(get_child(i));
        if (!child) {
			continue;
		}
        
		child->set_global_position(Vector3(node.x(), node.y(), node.z()));
        child->set_global_rotation(Vector3(node.rx(), node.ry(), node.rz()));
        child->set_scale(Vector3(node.sx(), node.sy(), node.sz()));
	}
}

void RecordedScene::save_recording() {
	Ref<FileAccess> file = FileAccess::open(filename, FileAccess::WRITE);
	if (file.is_null()) {
		print_line("RecordedScene: failed to open " + filename + " for writing");
		return;
	}

	const std::string data = recording_data.SerializeAsString();
	file->store_buffer(reinterpret_cast<const uint8_t *>(data.data()), data.size());
	file->flush();
	file->close();
}