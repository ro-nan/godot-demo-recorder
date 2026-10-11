#ifndef RECORDED_SIGNAL_H
#define RECORDED_SIGNAL_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/node.hpp>

namespace godot {

class RecordedSignal : public RefCounted {
	GDCLASS(RecordedSignal, RefCounted)

protected:
	static void _bind_methods();

public:
	StringName get_name() const;
	Node *get_owner() const;
	void set_name(const StringName &p_name);
	void set_owner(Node *p_owner);
	void callback(const Variant **p_args, GDExtensionInt p_argcount, GDExtensionCallError &r_error);

private:
	StringName name;
	Node *owner = nullptr;
};

} // namespace godot

#endif