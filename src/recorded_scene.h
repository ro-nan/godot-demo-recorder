#ifndef RECORDED_SCENE_H
#define RECORDED_SCENE_H

#include <godot_cpp/classes/node3d.hpp>
#include "main.pb.h"

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

private:
	bool recording = true;
	bool initialized = false;
	bool replay_loaded = false;
	bool restore_pause_on_exit = false;
	bool tree_was_paused = false;
	int replay_frame_index = 0;
	String filename = "rec.bin";
	demo::Recording recording_data;
	demo::Recording replay_data;
    demo::Recording_Frame *frame;

	void apply_mode();
	void load_replay();
	void capture_frame(godot::Node3D* parent);
	void replay_frame();
	void restore_tree_pause();
	void save_recording();
};

} // namespace godot

#endif