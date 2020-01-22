
os.execute("cp BitEngine/dependencies/glfw.lua BitEngine/dependencies/glfw/premake5.lua")
os.execute("cp BitEngine/dependencies/json.lua BitEngine/dependencies/json/premake5.lua")
os.execute("cp BitEngine/dependencies/assimp.lua BitEngine/dependencies/assimp/premake5.lua")
os.execute("cp BitEngine/dependencies/assimp_config.h BitEngine/dependencies/assimp/include/assimp/config.h")
os.execute("cp BitEngine/dependencies/assimp_revision.h BitEngine/dependencies/assimp/include/revision.h")
os.execute("cp BitEngine/dependencies/assimp/contrib/zlib/zconf.h.in BitEngine/dependencies/assimp/contrib/zlib/zconf.h")
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
IncludeDir["GLAD"] = "BitEngine/dependencies/glad/include"
IncludeDir["ImGui"] = "BitEngine/dependencies/imgui"
IncludeDir["glm"] = "BitEngine/dependencies/glm"
IncludeDir["json"] = "BitEngine/dependencies/json/include"
IncludeDir["stb"] = "BitEngine/dependencies/stb"
IncludeDir["winksignals"] = "BitEngine/dependencies/Wink-Signals"
IncludeDir["assimp"] = "BitEngine/dependencies/assimp/include"

--configuration "not windows"
--   prebuildcommands { "cp default.config bin/project.config" }

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
	objdir ("bin-tmp/" .. outputdir .. "/%{prj.name}")

	-- pchheader "bepch.h"
	-- pchsource "BitEngine/src/bepch.cpp"

	files
	{
		"%{prj.name}/src/BitEngine/**.h",
		"%{prj.name}/src/BitEngine/**.cpp",
		"%{prj.name}/BitEngine/dependencies/glm/glm/**.hpp",
        "%{prj.name}/BitEngine/dependencies/glm/glm/**.inl",
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
		"%{IncludeDir.GLAD}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.winksignals}",
		"%{IncludeDir.json}"
	}

	links 
	{ 
		"GLFW",
		"ImGui",
		"opengl32.lib"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"BE_PLATFORM_WINDOWS",
			"GLFW_INCLUDE_NONE"
		}

	filter "configurations:Debug"
		defines "BE_DEBUG"
		runtime "Debug"
		symbols "on"
		staticruntime "Off"

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
	objdir ("bin-tmp/" .. outputdir .. "/%{prj.name}")

	files
	{
		"samples/sample01/src/**.h",
		"samples/sample01/src/**.cpp",
		"BitEngine/src/Platform/glfw/**.cpp",
		"BitEngine/src/Platform/glfw/**.h",
		"BitEngine/src/Platform/opengl/**.cpp",
		"BitEngine/src/Platform/opengl/**.h",
		"BitEngine/dependencies/glad/src/glad.c",
	}

	includedirs
	{
		"BitEngine/src",
		"dependencies",
		"%{IncludeDir.glm}",
		"%{prj.name}/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLAD}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.winksignals}",
		"%{IncludeDir.json}",
		"%{IncludeDir.assimp}",
	}

	links
	{
		"GLFW",
		"BitEngine",
		"ImGui",
		"assimp"
	}
	debugdir "samples/sample01"

	postbuildcommands {
		"../copyfiles.sh ../bin/" .. outputdir .. "/Sample01/%{cfg.buildtarget.name} sample01/",
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"BE_PLATFORM_WINDOWS"
		}
	
	filter "system:linux"
		defines
		{
			"unix",
			"BE_PLATFORM_LINUX"
		}

		links
		{
			"X11",
			"dl",
			"pthread",
			"stdc++fs"
		}

	filter "configurations:Debug"
		defines "BE_DEBUG"
		runtime "Debug"
		symbols "on"
		debugargs { "--debug" }
		editandcontinue "On"
		staticruntime "Off"

	filter "configurations:Release"
		defines "BE_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "BE_DIST"
		runtime "Release"
		optimize "on"



project "Sample02"
	location "samples"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-tmp/" .. outputdir .. "/%{prj.name}")

	files
	{
		"samples/sample01/src/**.h",
		"samples/sample02/src/main.cpp",
		"BitEngine/src/Platform/glfw/**.cpp",
		"BitEngine/src/Platform/glfw/**.h",
		"BitEngine/src/Platform/opengl/**.cpp",
		"BitEngine/src/Platform/opengl/**.h",
		"BitEngine/dependencies/glad/src/glad.c",
	}
	
	dependson
	{
		"Sample02DLL"
	}

	includedirs
	{
		"BitEngine/src",
		"BitEngine/dependencies/json/src",
		"samples/sample01/src",
		"dependencies",
		"%{IncludeDir.glm}",
		"%{prj.name}/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLAD}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.winksignals}",
		"%{IncludeDir.json}",
		"%{IncludeDir.assimp}",
	}

	links
	{
		"BitEngine",
	}
	debugdir "samples/sample02"

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"BE_PLATFORM_WINDOWS"
		}
		
		postbuildcommands {
			"..\\copyfiles.sh ../bin/" .. outputdir .. "/Sample02/*.exe sample02/"
		}

	filter "configurations:Debug"
		defines "BE_DEBUG"
		runtime "Debug"
		symbols "on"
		debugargs { "--debug" }
		staticruntime "Off"

	filter "configurations:Release"
		defines "BE_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "BE_DIST"
		runtime "Release"
		optimize "on"

project "Sample02DLL"
	location "samples"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/Sample02")
	objdir ("bin-tmp/" .. outputdir .. "/Sample02")

	files
	{
		"samples/sample01/src/**.h",
		"samples/sample02/src/game.cpp",
	}

	includedirs
	{
		"BitEngine/src",
		"samples/sample01/src",
		"dependencies",
		"%{IncludeDir.glm}",
		"%{prj.name}/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLAD}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.winksignals}",
		"%{IncludeDir.json}",
		"%{IncludeDir.assimp}",
	}
	
	defines
	{
		BE_LIBRARY_EXPORTS
	}

	links
	{
		"BitEngine"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"BE_PLATFORM_WINDOWS",
		}
		
		postbuildcommands {
			"..\\copyfiles.sh ../bin/" .. outputdir .. "/Sample02/*.dll sample02/"
		}


	filter "configurations:Debug"
		defines "BE_DEBUG"
		runtime "Debug"
		symbols "Full"
		symbolspath "$(OutDir)$(TargetName)_$([System.DateTime]::Now.ToString(\"yyyy.MM.dd.HH.mm.ss\")).pdb"
		linkoptions { "/Zi" }
		debugargs { "--debug" }
		editandcontinue "On"
		staticruntime "Off"


	filter "configurations:Release"
		defines "BE_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "BE_DIST"
		runtime "Release"
		optimize "on"