extends Node3D
@onready var hello: RecordedScene = $".."

func _process(delta: float) -> void:
	if hello.get_current_frame() == 300:
		hello.scrub_to_frame(100)
	
	print(hello.get_current_frame())
