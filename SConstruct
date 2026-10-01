#!/usr/bin/env python

env = SConscript("godot-cpp/SConstruct", {"api_version": "4.6"})

# Adjust this if your source files live in a different folder.
env.Append(CPPPATH=["src/"])
env.Append(LIBS=["protobuf"])
sources = Glob("src/*.cpp") + Glob("src/*.cc", strings=True)

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "demo/bin/libdemo-recorder.{}.{}.framework/libdemo-recorder.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
elif env["platform"] == "ios":
    if env["ios_simulator"]:
        library = env.StaticLibrary(
            "demo/bin/libdemo-recorder.{}.{}.simulator.a".format(env["platform"], env["target"]),
            source=sources,
        )
    else:
        library = env.StaticLibrary(
            "demo/bin/libdemo-recorder.{}.{}.a".format(env["platform"], env["target"]),
            source=sources,
        )
else:
    library = env.SharedLibrary(
        "demo/bin/libdemo-recorder{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

env.NoCache(library)
Default(library)
