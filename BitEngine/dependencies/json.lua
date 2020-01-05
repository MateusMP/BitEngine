project "JSON"
	kind "StaticLib"
	language "C"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-tmp/" .. outputdir .. "/%{prj.name}")

	files
	{
		"single_include/nlohmann/json.hpp",
	}
		
	systemversion "latest"
	staticruntime "On"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
