
#include "bitengine/Core/EngineConfiguration.h"
#include "bitengine/Core/Resources/DevResourceLoader.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace BitEngine;
using ::testing::_;
using ::testing::Return;

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

class MockResourceType1 : public BitEngine::BaseResource {
	public:
		MockResourceType1(ResourceMeta* _meta)
			: BaseResource(_meta){}
};

TEST(ResourceLoader, LoadMultipleResources)
{
	MockGameEngine mockGE;
	MockResourceManager manager1, manager2;
	DevResourceLoader devResourceLoader(mockGE.getMessenger(), mockGE.getTaskManager());
	ResourceLoader* resourceLoader = &devResourceLoader;;

	EXPECT_CALL(manager1, setResourceLoader(&devResourceLoader));
	EXPECT_CALL(manager2, setResourceLoader(&devResourceLoader));

	devResourceLoader.registerResourceManager("TYPE1", &manager1);
	devResourceLoader.registerResourceManager("TYPE2", &manager2);

	ASSERT_TRUE(resourceLoader->loadIndex("resources/test_resources.idx")) << "Could not load resources/test_resources.idx";

	ResourceMeta* metaR1 = resourceLoader->findMeta("data/someGroup/A piece of data");
	ASSERT_EQ(metaR1->package, "someGroup");
	ASSERT_EQ(metaR1->resourceName, "A piece of data");
	ASSERT_EQ(metaR1->type, "TYPE1");
	ASSERT_EQ(metaR1->properties.getNumberOfProperties(), 4);

	MockResourceType1 r1(metaR1);
	EXPECT_CALL(manager1, loadResource(metaR1)).WillRepeatedly(Return(&r1));

	RR<MockResourceType1> refR1 = resourceLoader->getResource<MockResourceType1>(std::string("data/someGroup/A piece of data"));

	ASSERT_TRUE(refR1.isValid());
	ASSERT_EQ(refR1.get(), &r1);
}
