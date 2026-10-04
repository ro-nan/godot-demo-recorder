#include "recorded_scene.h"

using namespace godot;

void RecordedScene::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_recording", "recording"), &RecordedScene::set_recording);
	ClassDB::bind_method(D_METHOD("is_recording"), &RecordedScene::is_recording);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "recording"), "set_recording", "is_recording");

	ClassDB::bind_method(D_METHOD("get_current_frame"), &RecordedScene::get_current_frame);
	ClassDB::bind_method(D_METHOD("scrub_to_frame", "frame_index"), &RecordedScene::scrub_to_frame);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "current_frame"), "get_current_frame", "scrub_to_frame");

	ClassDB::bind_method(D_METHOD("get_recording_filename"), &RecordedScene::get_recording_filename);
	ClassDB::bind_method(D_METHOD("set_recording_filename", "recording_filename"), &RecordedScene::set_recording_filename);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "recording_filename"), "set_recording_filename", "get_recording_filename");
}

RecordedScene::RecordedScene() = default;

RecordedScene::~RecordedScene() = default;

String RecordedScene::get_recording_filename() const {
	return filename;
}

void RecordedScene::set_recording_filename(String recording_filename) {
	filename = recording_filename;
}

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
		recording_data.Clear();
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
	current_time += delta;
	if (recording) {
        frame = recording_data.add_frames();
		frame->set_time(current_time);
		capture_frame(this);
        if (frame_index % 5 == 0) { save_recording(); }
		return;
	}

	replay_frame();
}

std::string RecordedScene::get_path(Node* node) {
	return String(node->get_path().get_concatenated_names()).utf8().get_data();
}

void RecordedScene::capture_position(Node *child, demo::Recording_Frame *frame) {
	Node3D *node = Object::cast_to<Node3D>(child);
	if (!node) {
		return;
	}

	if (!node->has_method("get_position")){
		return;
	}

	const std::string path = get_path(node);
	const Vector3 position = node->get_global_position();
	demo::Recording_Frame_Position message;
	message.set_path(path);
	message.set_x(position.x);
	message.set_y(position.y);
	if (node->is_class("Node3D")) { message.set_z(position.z); }

	auto prev_it = previous_positions.find(path);
	const bool unchanged = prev_it != previous_positions.end() &&
		google::protobuf::util::MessageDifferencer::Equals(prev_it->second, message);
	if (!unchanged) {
		*frame->add_positions() = message;
		previous_positions[path] = message;
	}
}

void RecordedScene::capture_rotation(Node *child, demo::Recording_Frame *frame) {
	Node3D *node = Object::cast_to<Node3D>(child);
	if (!node) {
		return;
	}

	if (!node->has_method("get_rotation")){
		return;
	}

	const std::string path = get_path(node);
	const Vector3 rotation = node->get_global_rotation();
	demo::Recording_Frame_Rotation message;
	message.set_path(path);
	message.set_rx(rotation.x);
	message.set_ry(rotation.y);
	if (node->is_class("Node3D")) { message.set_rz(rotation.z); }

	auto prev_it = previous_rotations.find(path);
	const bool unchanged = prev_it != previous_rotations.end() &&
		google::protobuf::util::MessageDifferencer::Equals(prev_it->second, message);
	if (!unchanged) {
		*frame->add_rotations() = message;
		previous_rotations[path] = message;
	}
}

void RecordedScene::capture_scale(Node *child, demo::Recording_Frame *frame) {
	Node3D *node = Object::cast_to<Node3D>(child);
	if (!node) {
		return;
	}

	if (!node->has_method("get_scale")){
		return;
	}

	const std::string path = get_path(node);
	const Vector3 scale = node->get_scale();
	demo::Recording_Frame_Scale message;
	message.set_path(path);
	message.set_sx(scale.x);
	message.set_sy(scale.y);
	if (node->is_class("Node3D")) { message.set_sz(scale.z); }

	auto prev_it = previous_scales.find(path);
	const bool unchanged = prev_it != previous_scales.end() &&
		google::protobuf::util::MessageDifferencer::Equals(prev_it->second, message);
	if (!unchanged) {
		*frame->add_scales() = message;
		previous_scales[path] = message;
	}
}

void RecordedScene::capture_visibility(Node *child, demo::Recording_Frame *frame) {
	Node3D *node = Object::cast_to<Node3D>(child);
	if (!node) {
		return;
	}

	if (!node->has_method("is_visible")){
		return;
	}

	const std::string path = get_path(node);
	demo::Recording_Frame_Visibility message;
	message.set_path(path);
	message.set_visible(node->is_visible());

	auto prev_it = previous_visibilities.find(path);
	const bool unchanged = prev_it != previous_visibilities.end() &&
		google::protobuf::util::MessageDifferencer::Equals(prev_it->second, message);
	if (!unchanged) {
		*frame->add_visibilities() = message;
		previous_visibilities[path] = message;
	}
}

void RecordedScene::capture_frame(godot::Node3D* parent) {
	for (int i = 0; i < parent->get_child_count(); i++) {
		Node3D *child = Object::cast_to<Node3D>(parent->get_child(i));
		if (!child) {
			continue;
		}

		capture_position(child, frame);
		capture_rotation(child, frame);
		capture_scale(child, frame);
		capture_visibility(child, frame);
		// TODO: Add more things to capture

		capture_frame(child);
	}
}

void RecordedScene::replay_frame() {
	if (!replay_loaded || current_replay_frame_index >= replay_data.frames_size()) {
		return;
	}
	if (current_time < replay_data.frames(current_replay_frame_index + 1).time()) {
		return;
	}
	const demo::Recording_Frame &frame = replay_data.frames(current_replay_frame_index);
	int node_index = 0;
	for (int i = 0; i < frame.positions_size() && node_index < frame.positions_size(); i++) {
		const demo::Recording_Frame_Position &position = frame.positions(i);

        auto decoded_path = position.path();
        std::string selfname = String(get_name()).utf8().get_data();
        const godot::NodePath path = NodePath(decoded_path.substr(decoded_path.find(selfname) + selfname.length() + 1).c_str());

		Node *child = get_node<Node>(path);
        if (!child) {
			continue;
		}
        
		if (child->is_class("Node3D")) { Object::cast_to<Node3D>(child)->set_global_position(Vector3(position.x(), position.y(), position.z())); }
		if (child->is_class("Node2D")) { Object::cast_to<Node2D>(child)->set_global_position(Vector2(position.x(), position.y())); }
		if (child->is_class("Control")) { Object::cast_to<Control>(child)->set_global_position(Vector2(position.x(), position.y())); }
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
		
		if (child->is_class("Node3D")) { Object::cast_to<Node3D>(child)->set_global_rotation(Vector3(rotation.rx(), rotation.ry(), rotation.rz())); }
		if (child->is_class("Node2D")) { Object::cast_to<Node2D>(child)->set_global_rotation(rotation.rz()); }
		if (child->is_class("Control")) { Object::cast_to<Control>(child)->set_rotation(rotation.rz()); }
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
		
		if (child->is_class("Node3D")) { Object::cast_to<Node3D>(child)->set_scale(Vector3(scale.sx(), scale.sy(), scale.sz())); }
		if (child->is_class("Node2D")) { Object::cast_to<Node2D>(child)->set_scale(Vector2(scale.sx(), scale.sy())); }
		if (child->is_class("Control")) { Object::cast_to<Control>(child)->set_scale(Vector2(scale.sx(), scale.sy())); }
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
	current_replay_frame_index++;
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

int RecordedScene::get_current_frame() const {
	return current_replay_frame_index;
}

void RecordedScene::scrub_to_frame(int p_frame_index) {
	if (!replay_loaded || p_frame_index < 0 || p_frame_index >= replay_data.frames_size()) {
		return;
	}

	int frame_delta = p_frame_index - current_replay_frame_index;
	if (frame_delta == 0) {
		return;
	}
	if (frame_delta > 0) {
		for (int i = 0; i < frame_delta; i++) {
			frame_index++;
			replay_frame();
		}
	}
	else {
		frame_index = 0;
		for (int i = 0; i < p_frame_index; i++) {
			frame_index++;
			replay_frame();
		}
	}
}