extends Control

@onready var main: RecordedScene = $"../main"

@onready var h_slider: HSlider = $VBoxContainer/HSlider
@onready var progress_bar: ProgressBar = $VBoxContainer/ProgressBar

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	if main.is_recording():
		return
	
	h_slider.max_value = main.get_total_frames()
	
	Input.mouse_mode = Input.MOUSE_MODE_VISIBLE
	
	if !dragging:
		h_slider.value = main.get_current_frame()

	else:
		main.scrub_to_frame(int(h_slider.value))

var dragging = false
func _on_h_slider_drag_ended(value_changed: bool) -> void:
	dragging = false
	main.scrub_to_frame(int(h_slider.value))

func _on_h_slider_drag_started() -> void:
	dragging = true
