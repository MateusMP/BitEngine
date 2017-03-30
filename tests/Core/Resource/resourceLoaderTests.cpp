
#include "bitengine/Core/EngineConfiguration.h"
#include "bitengine/Core/Resources/DevResourceLoader.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace BitEngine;
using ::testing::_;

class MockGameEngine : public GameEngine {
	public:
		MOCK_METHOD0(getConfigurations, EngineConfiguration*());
		MOCK_METHOD0(getResourceLoader, ResourceLoader*());
		MOCK_METHOD0(getMessenger, Messenger*());
		MOCK_METHOD1(getSystem, System*(const std::string& name));
		MOCK_METHOD0(getTaskManager, TaskManager*());
		MOCK_METHOD0(getVideoDriver, VideoDriver*());
};

class MockResourceManager : public ResourceManager {
	public:
		MOCK_METHOD0(init, bool());
		MOCK_METHOD0(update, void());
		MOCK_METHOD1(setResourceLoader, void (ResourceLoader* loader));
		MOCK_METHOD1(loadResource, BaseResource*(ResourceMeta* meta));
		MOCK_METHOD1(resourceNotInUse,  void(ResourceMeta* meta));
		MOCK_METHOD1(reloadResource,  void(BaseResource* resource));
		MOCK_METHOD1(resourceRelease,  void (ResourceMeta* meta));
		MOCK_CONST_METHOD0(getCurrentRamUsage, u32());
		MOCK_CONST_METHOD0(getCurrentGPUMemoryUsage, u32());
};


TEST(ResourceLoader, LoadMultipleResources)
{
	MockGameEngine mockGE;
	MockResourceManager manager1, manager2;
	DevResourceLoader resourceLoader(&mockGE);

	EXPECT_CALL(manager1, setResourceLoader(&resourceLoader));
	EXPECT_CALL(manager2, setResourceLoader(&resourceLoader));

	resourceLoader.registerResourceManager("TYPE1", &manager1);
	resourceLoader.registerResourceManager("TYPE2", &manager2);

	//resourceLoader.loadIndex();
}
