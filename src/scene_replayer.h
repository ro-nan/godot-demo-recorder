#ifndef SCENEREPLAYER_H
#define SCENEREPLAYER_H

#include <godot_cpp/classes/node3d.hpp>
#include "main.pb.h"

namespace godot {

class SceneReplayer : public Node3D {
    GDCLASS(SceneReplayer, Node3D)

protected:
    static void _bind_methods();

public:
    SceneReplayer();
    ~SceneReplayer();

    void _process(double delta) override;
    void _ready() override;
private:
    godot::String filename;
    godot::String* ptr_filename;

    demo::Recording replay;

    int frame = 0;
};

}  // namespace godot

#endif
