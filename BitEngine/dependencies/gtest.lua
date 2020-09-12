project "GoogleTest"
    kind "StaticLib"
    files {
        "googletest/googletest/src/gtest-all.cc",
        "googletest/googlemock/src/gmock-all.cc"
    }
    includedirs { 
        "googletest/googletest/include", 
        "googletest/googletest",
        "googletest/googlemock/include", 
        "googletest/googlemock"
    }

    runtime "Debug"
    symbols "on"
    staticruntime "Off"