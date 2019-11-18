
#include "BitEngine/Core/Resources/ResourceManager.h"

// ResourceMeta

const std::string BitEngine::ResourceMeta::toString() const {
	return ("Resource Meta id: " + std::to_string(id) +
		"\n\tpackage: " + package +
		"\n\ttype: " + type +
		"\n\tfiledDir: " + resourceName +
		"\n\tprops: " + /*properties.dump() + */"\n");
}
