
os.execute("cp BitEngine/dependencies/glfw.lua BitEngine/dependencies/glfw/premake5.lua")
os.execute("cp BitEngine/dependencies/json.lua BitEngine/dependencies/json/premake5.lua")
os.execute("cp BitEngine/dependencies/assimp.lua BitEngine/dependencies/assimp/premake5.lua")
os.execute("cp BitEngine/dependencies/imgui.lua BitEngine/dependencies/imgui/premake5.lua")

workspace "BitEngine"
	architecture "x64"
	startproject "Sample01"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}
	
	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "BitEngine/dependencies/glfw/include"
IncludeDir["GLEW"] = "BitEngine/dependencies/glew-2.1.0/include"
IncludeDir["ImGui"] = "BitEngine/dependencies/imgui"
IncludeDir["glm"] = "BitEngine/dependencies/glm"
IncludeDir["json"] = "BitEngine/dependencies/json/src"
IncludeDir["stb"] = "BitEngine/dependencies/stb"
IncludeDir["winksignals"] = "BitEngine/dependencies/Wink-Signals"
IncludeDir["assimp"] = "BitEngine/dependencies/assimp/include"

configuration "not windows"
   prebuildcommands { "cp default.config bin/project.config" }

group "Dependencies"
	include "BitEngine/dependencies/glfw/"
	include "BitEngine/dependencies/imgui/"
	include "BitEngine/dependencies/json/"
	include "BitEngine/dependencies/assimp/"

group ""

project "BitEngine"
	location "BitEngine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	-- pchheader "hzpch.h"
	-- pchsource "BitEngine/src/hzpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/BitEngine/dependencies/glm/glm/**.hpp",
        "%{prj.name}/BitEngine/dependencies/glm/glm/**.inl",
        "%{prj.name}/BitEngine/dependencies/json/src/json.hpp",
		"%{prj.name}/BitEngine/dependencies/stb/src/stb_image.h",
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLEW}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.winksignals}",
		"%{IncludeDir.json}"
	}

	links 
	{ 
		"BitEngine/dependencies/glew-2.1.0/lib/Release/x64/glew32s.lib",
		"GLFW",
		"ImGui",
		"opengl32.lib"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"BE_PLATFORM_WINDOWS",
			"GLEW_STATIC",
			"GLFW_INCLUDE_NONE"
		}

	filter "configurations:Debug"
		defines "BE_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "BE_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "BE_DIST"
		runtime "Release"
		optimize "on"

project "Sample01"
	location "samples"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"samples/sample01/src/**.h",
		"samples/sample01/src/**.cpp",
	}

	includedirs
	{
		"BitEngine/src",
		"BitEngine/dependencies/json/src",
		"dependencies",
		"%{IncludeDir.glm}",
		"%{prj.name}/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLEW}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.winksignals}",
		"%{IncludeDir.json}/src/",
		"%{IncludeDir.assimp}",
	}

	links
	{
		"BitEngine",
	}
	debugdir "samples/sample01"

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"BE_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "BE_DEBUG"
		runtime "Debug"
		symbols "on"
		debugargs { "--debug" }

	filter "configurations:Release"
		defines "BE_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "BE_DIST"
		runtime "Release"
		optimize "on"
