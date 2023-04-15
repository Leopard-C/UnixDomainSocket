
add_rules("mode.debug", "mode.release", "mode.releasedbg")
set_languages("c99", "cxx17")

add_cxflags("-Wreturn-type", "-Wsign-compare", "-Wunused-variable", "-Wswitch", "-Werror")
add_cxflags("-Wno-unused-result", "-Wno-deprecated-declarations", "-Wno-unused-parameter")

mode = ""
if is_mode("release") then
    mode = "release"
elseif is_mode("releasedbg") then
    mode = "releasedbg"
elseif is_mode("debug") then
    mode = "debug"
    add_defines("_DEBUG")
end

if (mode ~= "") then
    print(mode)
end

add_includedirs("src")
add_includedirs("third_party")
set_objectdir("build/obj")
add_linkdirs("lib/linux")
add_linkdirs(string.format("lib/linux/%s", mode))

target("uds_base")
    set_kind("static")
    add_files("src/uds/base/**.cpp")
    add_links("pthread")
    set_targetdir(string.format("lib/linux/%s", mode))

target("uds_json")
    set_kind("static")
    add_files("src/uds/json/**.cpp")
    add_links("jsoncpp")
    set_targetdir(string.format("lib/linux/%s", mode))

target("echo_server")
    set_kind("binary")
    add_files("example/echo_server/server.cpp")
    add_deps("uds_base")
    set_targetdir("bin")

target("echo_client")
    set_kind("binary")
    add_files("example/echo_server/client.cpp")
    add_deps("uds_base")
    set_targetdir("bin")

target("benchmark_server")
    set_kind("binary")
    add_files("example/benchmark/server.cpp")
    add_deps("uds_base")
    set_targetdir("bin")

target("benchmark_client")
    set_kind("binary")
    add_files("example/benchmark/client.cpp")
    add_deps("uds_base")
    set_targetdir("bin")

target("file_receiver")
    set_kind("binary")
    add_files("example/file_transfer/receiver.cpp")
    add_deps("uds_base")
    set_targetdir("bin")

target("file_sender")
    set_kind("binary")
    add_files("example/file_transfer/sender.cpp")
    add_deps("uds_base")
    set_targetdir("bin")

target("uds_base_cli")
    set_kind("binary")
    add_files("example/uds_base_cli/uds_base_cli.cpp")
    add_deps("uds_base")
    set_targetdir("bin")

target("uds_json_cli")
    set_kind("binary")
    add_files("example/uds_json_cli/uds_json_cli.cpp")
    add_deps("uds_json", "uds_base")
    set_targetdir("bin")

target("simple_server")
    set_kind("binary")
    add_files("example/simple_json_server/server.cpp")
    add_deps("uds_json", "uds_base")
    set_targetdir("bin")

target("simple_client")
    set_kind("binary")
    add_files("example/simple_json_server/client.cpp")
    add_deps("uds_json", "uds_base")
    set_targetdir("bin")

