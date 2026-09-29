#ifndef SCENERECORDER_H
#define SCENERECORDER_H

#include <godot_cpp/classes/node.hpp>
#include "main.pb.h"

namespace godot {

class SceneRecorder : public Node {
    GDCLASS(SceneRecorder, Node)

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
};

}  // namespace godot

#endif
