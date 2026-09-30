# Demo Recorder
This is a game-agnostic demo recorder for Godot. It allows your builds to record demos and upload them to a server for later playback and analysis.

Watching players play your game rather than relying on general gameplay statistics is usually optimal, but watching those players over Discord is time consuming. 

This addon seeks to solve this issue by recording gameplay sessions, uploading them to a server, and sending them to you to watch at whatever time you please. In addition to this, you can pick out specific sections to watch, skip around in the replays, and gather general data from them as well. 

## Usage 
Set recording mode using `RecordedScene.set_recording` and check if the scene is recording using `RecordedScene.is_recording`.


Make sure to disable scripts that may interfere with replaying by returning during their process when `RecordedScene.is_recording`. Eg.

Instead of
```
func _physics_process(delta):
    global_rotation = Vector3.ZERO # On some frames this will lead to framefighting
```
Do
```
func _physics_process(delta):
    if RecordedScene.is_recording: return # RecordedScene being the recorded scene root node
    global_rotation = Vector3.ZERO # On some frames this will lead to framefighting

```

## Advantages
| Advantage | Compared to Streaming | Compared to Video Recordings |
| --- | --- | --- |
| **Asynchronous Review** | Review anytime without scheduling live sessions | — |
| **Playback Control** | Scrub around in the recording | — |
| **Bandwidth / File Size** | Takes up far less bandwidth | Smaller file size in most cases |
| **Visual Quality** | Perfectly lossless quality at any resolution (60 FPS, no bitrate smearing) | Perfectly lossless quality at any resolution (60 FPS, no bitrate smearing) |
| **Camera Perspective** | Multiple camera angles | Multiple camera angles |
| **Debugging Support** | Record custom debugging variables | Record custom debugging variables |
| **External Software** | No external software required | No external software required |

## Limitations 
While the file size of a simple recording (~380kb for a 12s recording) is much less than a similar quality video (High quality 1080p video @ 60fps is ~9MB, ~25x bigger), the file size of recordings with many moving objects may be larger than video. 

## AI Use Disclosure
AI is/was used for code cleaning and project set up purposes ONLY. 

Code design is mine.