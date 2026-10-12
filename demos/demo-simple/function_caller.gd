extends Node3D

@onready var main: RecordedScene = $".."

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	pass # Replace with function body.
	
func _process(delta: float) -> void:
	if !main.recording: return
	hello(delta)
	main.record_function(&"hello", get_path(), [str(delta)])

func hello(what):
	print('hello, ', what, "!")
