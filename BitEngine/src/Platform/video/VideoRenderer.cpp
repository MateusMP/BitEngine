
#include "Platform/video/VideoRenderer.h"

namespace BitEngine {

	DataUseMode DataUseMode::fromString(const std::string& str)
	{
		if (str.compare("VERTEX") == 0) {
			return DataUseMode::Vertex;
		}
		else if (str.compare("UNIFORM") == 0) {
			return DataUseMode::Uniform;
		}

		return DataUseMode::TotalModes;
	}

	DataType DataType::fromString(const std::string& str)
	{
		if (str.compare("TEXTURE_1D") == 0) {
			return DataType::TEXTURE_1D;
		}
		else if (str.compare("TEXTURE_2D") == 0) {
			return DataType::TEXTURE_2D;
		}
		else if (str.compare("TEXTURE_3D") == 0) {
			return DataType::TEXTURE_3D;
		}
		else if (str.compare("LONG") == 0) {
			return DataType::LONG;
		}
		else if (str.compare("FLOAT") == 0) {
			return DataType::FLOAT;
		}
		else if (str.compare("VEC2") == 0) {
			return DataType::VEC2;
		}
		else if (str.compare("VEC3") == 0) {
			return DataType::VEC3;
		}
		else if (str.compare("VEC4") == 0) {
			return DataType::VEC4;
		}
		else if (str.compare("MAT3") == 0) {
			return DataType::MAT3;
		}
		else if (str.compare("MAT4") == 0) {
			return DataType::MAT4;
		}

		return DataType::INVALID_DATA_TYPE;
	}
}