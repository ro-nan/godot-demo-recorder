#!/usr/bin/env python

import os
import shutil

paths = [
    "demos/demo-simple",
    # Add more project directories here if you want to deploy the same build to multiple Godot projects.
    # "demos/demo-other",
]

if not paths:
    raise ValueError("At least one project path must be configured in paths[]")

build_path = os.path.abspath(paths[0])

env = SConscript("godot-cpp/SConstruct", {"api_version": "4.6"})

# Adjust this if your source files live in a different folder.
env.Append(CPPPATH=["src/"])
env.Append(LIBS=["protobuf"])
sources = Glob("src/*.cpp") + Glob("src/*.cc", strings=True)


def build_library_path():
    if env["platform"] == "macos":
        return os.path.join(
            build_path,
            "bin",
            "libdemo-recorder.{}.{}.framework/libdemo-recorder.{}.{}".format(
                env["platform"], env["target"], env["platform"], env["target"]
            ),
        )
    if env["platform"] == "ios":
        if env["ios_simulator"]:
            return os.path.join(
                build_path,
                "bin",
                "libdemo-recorder.{}.{}.simulator.a".format(env["platform"], env["target"]),
            )
        return os.path.join(
            build_path,
            "bin",
            "libdemo-recorder.{}.{}.a".format(env["platform"], env["target"]),
        )
    return os.path.join(
        build_path,
        "bin",
        "libdemo-recorder{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
    )


def deploy_projects(target, source, env):
    first_path = os.path.abspath(paths[0])
    first_path = os.path.normpath(first_path)
    for path in paths[1:]:
        target_path = os.path.abspath(path)
        target_path = os.path.normpath(target_path)

        if os.path.abspath(target_path) == os.path.abspath(first_path):
            continue

        if os.path.exists(target_path):
            shutil.rmtree(target_path)

        shutil.copytree(first_path, target_path, dirs_exist_ok=True)

    return 0


library = env.SharedLibrary(build_library_path(), source=sources) if env["platform"] not in ["macos", "ios"] else (
    env.SharedLibrary(
        build_library_path(),
        source=sources,
    )
    if env["platform"] == "macos"
    else env.StaticLibrary(
        build_library_path(),
        source=sources,
    )
)

# Ensure the project is copied to each configured target project after the first build completes.
env.AddPostAction(library, deploy_projects)

env.NoCache(library)
Default(library)
