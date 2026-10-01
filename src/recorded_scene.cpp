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
		previous_positions.clear();
		previous_rotations.clear();
		previous_scales.clear();
		previous_visibilities.clear();
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
		
		auto path = std::string(String(child->get_path().get_concatenated_names()).utf8().get_data()); // NOTE: Maybe make global for this if I keep having to use it? Not sure.

        const Vector3 position = child->get_global_position();
		const bool has_position = child->has_method("get_position");

        const Vector3 rotation = child->get_global_rotation();
		const bool has_rotation = child->has_method("get_rotation");

        const Vector3 scale = child->get_scale();
		const bool has_scale = child->has_method("get_scale");

		const bool has_visibility = child->has_method("is_visible");
		
		if (has_position) {
			demo::Recording_Frame_Position current_position;
			current_position.set_path(path);
			current_position.set_x(position.x);
			current_position.set_y(position.y);
			current_position.set_z(position.z);

			auto prev_p = previous_positions.find(path);
			const bool p_unchanged = prev_p != previous_positions.end() &&
				google::protobuf::util::MessageDifferencer::Equals(prev_p->second, current_position);

			if (!p_unchanged) {
				*frame->add_positions() = current_position;
				previous_positions[path] = current_position;    
			}
		}
		
		if (has_rotation) {
			demo::Recording_Frame_Rotation current_rotation;
			current_rotation.set_path(path);
			current_rotation.set_rx(rotation.x);
			current_rotation.set_ry(rotation.y);
			current_rotation.set_rz(rotation.z);

			auto prev_r = previous_rotations.find(path);
			const bool r_unchanged = prev_r != previous_rotations.end() &&
				google::protobuf::util::MessageDifferencer::Equals(prev_r->second, current_rotation);

			if (!r_unchanged) {
			    *frame->add_rotations() = current_rotation;
				previous_rotations[path] = current_rotation;    
			}
		}

		if (has_scale) {
			demo::Recording_Frame_Scale current_scale;
			current_scale.set_path(path);
			current_scale.set_sx(scale.x);
			current_scale.set_sy(scale.y);
			current_scale.set_sz(scale.z);

			auto prev_s = previous_scales.find(path);
			const bool s_unchanged = prev_s != previous_scales.end() &&
				google::protobuf::util::MessageDifferencer::Equals(prev_s->second, current_scale);

			if (!s_unchanged) {
			    *frame->add_scales() = current_scale;
				previous_scales[path] = current_scale;    
			}
		}

		if (has_visibility) {
			demo::Recording_Frame_Visibility current_visibility;
			current_visibility.set_path(path);
			current_visibility.set_visible(child->is_visible());

			auto prev_v = previous_visibilities.find(path);
			const bool v_unchanged = prev_v != previous_visibilities.end() &&
				google::protobuf::util::MessageDifferencer::Equals(prev_v->second, current_visibility);

			if (!v_unchanged) {
			    *frame->add_visibilities() = current_visibility;
				previous_visibilities[path] = current_visibility;    
			}
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
	for (int i = 0; i < frame.positions_size() && node_index < frame.positions_size(); i++) {
		const demo::Recording_Frame_Position &position = frame.positions(i);

        auto decoded_path = position.path();
        std::string selfname = String(get_name()).utf8().get_data();
        const godot::NodePath path = NodePath(decoded_path.substr(decoded_path.find(selfname) + selfname.length() + 1).c_str());

		Node3D *child = get_node<Node3D>(path);
        if (!child) {
			continue;
		}
        
		child->set_global_position(Vector3(position.x(), position.y(), position.z()));
	}
	for (int i = 0; i < frame.rotations_size() && node_index < frame.rotations_size(); i++) {
		const demo::Recording_Frame_Rotation &rotation = frame.rotations(i);

		auto decoded_path = rotation.path();
		std::string selfname = String(get_name()).utf8().get_data();
		const godot::NodePath path = NodePath(decoded_path.substr(decoded_path.find(selfname) + selfname.length() + 1).c_str());

		Node3D *child = get_node<Node3D>(path);
		if (!child) {
			continue;
		}
		
		child->set_global_rotation(Vector3(rotation.rx(), rotation.ry(), rotation.rz()));
	}
	for (int i = 0; i < frame.scales_size() && node_index < frame.scales_size(); i++) {
		const demo::Recording_Frame_Scale &scale = frame.scales(i);

		auto decoded_path = scale.path();
		std::string selfname = String(get_name()).utf8().get_data();
		const godot::NodePath path = NodePath(decoded_path.substr(decoded_path.find(selfname) + selfname.length() + 1).c_str());

		Node3D *child = get_node<Node3D>(path);
		if (!child) {
			continue;
		}
		
		child->set_scale(Vector3(scale.sx(), scale.sy(), scale.sz()));
	}
	for (int i = 0; i < frame.visibilities_size() && node_index < frame.visibilities_size(); i++) {
		const demo::Recording_Frame_Visibility &visibility = frame.visibilities(i);

		auto decoded_path = visibility.path();
		std::string selfname = String(get_name()).utf8().get_data();
		const godot::NodePath path = NodePath(decoded_path.substr(decoded_path.find(selfname) + selfname.length() + 1).c_str());

		Node3D *child = get_node<Node3D>(path);
		if (!child) {
			continue;
		}

		child->set_visible(visibility.visible());
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