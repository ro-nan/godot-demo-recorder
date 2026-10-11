extends Node

# RecordedSignals are tiny objects that are stored in the reocrdedS_signals array that recieve callbacks
# There is one for each signal with a name of that signal
# When they get called via the signal they call a function on their owner of name callback, passing their name (the name of the signal), and the arguments of the signal

var recorded_signals : Array[RecordedSignal] = []
var callbacks : Array[Callable] = []
func _ready() -> void:
	for i in get_signal_list():
		var r = RecordedSignal.new()
		r.name = i.name
		r.owner = self
		recorded_signals.append(r)
		var c = Callable(r, "callback")
		callbacks.append(c)
		connect(i.name, c)
		
func callback(name : StringName, args : Array):
	print("CALLBACK ", name, args)

signal test
signal test_two(one, two, three)

var frame_num = 0
func _process(delta: float) -> void:
	test.emit()
	
	frame_num += 1
	if frame_num % 3 == 0:
		test_two.emit("where", "is", "it")
	
