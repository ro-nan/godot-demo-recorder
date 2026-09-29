#ifndef SCENERECORDER_H
#define SCENERECORDER_H

#include <godot_cpp/classes/node3d.hpp>
#include "main.pb.h"

namespace godot {

class SceneRecorder : public Node3D {
    GDCLASS(SceneRecorder, Node3D)

protected:
    static void _bind_methods();

public:
    SceneRecorder();
    ~SceneRecorder();

    void _process(double delta) override;
    void _ready() override;
private:
    godot::String filename;
    godot::String* ptr_filename;

    demo::Recording rec;

    void save_recording();
};

}  // namespace godot

#endif
