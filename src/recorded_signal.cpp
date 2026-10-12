#include "recorded_signal.h"

using namespace godot;

void RecordedSignal::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_name"), &RecordedSignal::get_name);
	ClassDB::bind_method(D_METHOD("set_name", "name"), &RecordedSignal::set_name);
	ClassDB::bind_method(D_METHOD("get_owner"), &RecordedSignal::get_owner);
	ClassDB::bind_method(D_METHOD("set_owner", "owner"), &RecordedSignal::set_owner);

	MethodInfo mi;
	mi.name = "callback";
	ClassDB::bind_vararg_method(METHOD_FLAGS_DEFAULT, "callback", &RecordedSignal::callback, mi);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "name"), "set_name", "get_name");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "owner"), "set_owner", "get_owner");
}

StringName RecordedSignal::get_name() const {
	return name;
}

void RecordedSignal::set_name(const StringName &p_name) {
	name = p_name;
}

void RecordedSignal::callback(const Variant **p_args, GDExtensionInt p_argcount, GDExtensionCallError &r_error) {
	Array args;
	for (int i = 0; i < p_argcount; ++i) {
		if (p_args != nullptr && p_args[i] != nullptr) {
			args.append(*p_args[i]);
		}
	}

	if (owner == nullptr) {
		print_line("Callback on ", name, " with no owner");
		r_error.error = GDEXTENSION_CALL_OK;
		return;
	}
	
	owner->call("callback", name, args);
	r_error.error = GDEXTENSION_CALL_OK;
}

Node *RecordedSignal::get_owner() const {
	return owner;
}

void RecordedSignal::set_owner(Node *p_owner) {
	owner = p_owner;
}