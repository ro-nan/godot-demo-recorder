#ifndef RECORDED_SCENE_H
#define RECORDED_SCENE_H

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/control.hpp>
#include "main.pb.h"
#include <string>
#include <unordered_map>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <google/protobuf/util/message_differencer.h>
#include <string>

namespace godot {

class RecordedScene : public Node3D {
	GDCLASS(RecordedScene, Node3D)

protected:
	static void _bind_methods();

public:
	RecordedScene();
	~RecordedScene();

	void _process(double delta) override;
	void _ready() override;
	void _exit_tree() override;

	void set_recording(bool p_recording);
	bool is_recording() const;

    int get_current_frame() const;
    void scrub_to_frame(int p_frame_index);

	String get_recording_filename() const;
	void set_recording_filename(String recording_filename);

    std::unordered_map<std::string, demo::Recording_Frame_Position> previous_positions;
    std::unordered_map<std::string, demo::Recording_Frame_Rotation> previous_rotations;
    std::unordered_map<std::string, demo::Recording_Frame_Scale> previous_scales;
    std::unordered_map<std::string, demo::Recording_Frame_Visibility> previous_visibilities;

private:
	bool recording = true;
	bool initialized = false;
	bool replay_loaded = false;
	bool restore_pause_on_exit = false;
	bool tree_was_paused = false;
	int frame_index = -1;
	double current_time = 0.0;
	int current_replay_frame_index = 0;
	String filename = "rec.dem";
	demo::Recording recording_data;
	demo::Recording replay_data;
	demo::Recording_Frame *frame = nullptr;
	demo::Recording scrub_frame_cache;
    
	void apply_mode();
	void load_replay();
	void capture_frame(godot::Node3D* parent);
	void capture_position(Node *child, demo::Recording_Frame *frame);
	void capture_rotation(Node *child, demo::Recording_Frame *frame);
	void capture_scale(Node *child, demo::Recording_Frame *frame);
	void capture_visibility(Node *child, demo::Recording_Frame *frame);
	void replay_frame();
	void save_recording();

    std::string get_path(Node* node);
};

} // namespace godot

#endif