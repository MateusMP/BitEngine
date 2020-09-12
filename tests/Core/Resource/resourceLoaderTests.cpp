
#include "BitEngine/Core/EngineConfiguration.h"
#include "BitEngine/Core/Resources/DevResourceLoader.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace BitEngine;
using ::testing::_;
using ::testing::Return;


class MockResourceManager : public ResourceManager {
	public:
		MOCK_METHOD0(init, bool());
		MOCK_METHOD0(update, void());
		MOCK_METHOD0(shutdown, void());
		MOCK_METHOD1(setResourceLoader, void (ResourceLoader* loader));
		MOCK_METHOD1(loadResource, BaseResource*(ResourceMeta* meta));
		MOCK_METHOD1(resourceNotInUse,  void(ResourceMeta* meta));
		MOCK_METHOD1(reloadResource,  void(BaseResource* resource));
		MOCK_METHOD1(resourceRelease,  void (ResourceMeta* meta));
        MOCK_METHOD2(loadResource, BaseResource*(ResourceMeta* meta, PropertyHolder* props));
        MOCK_CONST_METHOD0(getCurrentRamUsage, ptrsize());
		MOCK_CONST_METHOD0(getCurrentGPUMemoryUsage, u32());
};

class MockTaskManager : public BitEngine::TaskManager {
    MOCK_METHOD0(init, void());
    MOCK_METHOD0(update, void());
    MOCK_METHOD0(shutdown, void());

    MOCK_METHOD1(addTask, void(std::shared_ptr<Task> task));
    MOCK_METHOD1(scheduleToNextFrame, void(std::shared_ptr<Task> task));

    MOCK_METHOD1(waitTask, void(std::shared_ptr<Task>& task));

    MOCK_CONST_METHOD0(getTasks, const std::vector<TaskPtr>&());

    MOCK_CONST_METHOD0(verifyMainThread, void());
};

class MockResourceType1 : public BitEngine::BaseResource {
	public:
		MockResourceType1(ResourceMeta* _meta)
			: BaseResource(_meta){}
};

TEST(ResourceLoader, LoadMultipleResources)
{
	u8 memory[1024*32];
    MockTaskManager taskManager;
	MockResourceManager manager1, manager2;
	MemoryArena memoryArena;

	memoryArena.init(memory, sizeof(memory));

	DevResourceLoader devResourceLoader(&taskManager , memoryArena);

	EXPECT_CALL(manager1, setResourceLoader(&devResourceLoader));
	EXPECT_CALL(manager2, setResourceLoader(&devResourceLoader));

	devResourceLoader.registerResourceManager("TYPE1", &manager1);
	devResourceLoader.registerResourceManager("TYPE2", &manager2);

	ASSERT_TRUE(devResourceLoader.loadIndex("resources/test_resources.idx")) << "Could not load resources/test_resources.idx";

	DevResourceMeta* metaR1 = devResourceLoader.findMeta("data/someGroup/A piece of data");
	ASSERT_EQ(metaR1->package, "someGroup");
	ASSERT_EQ(metaR1->resourceName, "A piece of data");
	ASSERT_EQ(metaR1->type, "TYPE1");
	ASSERT_EQ(metaR1->properties.size(), 4);

	MockResourceType1 r1(metaR1);
	//EXPECT_CALL(manager1, loadResource(metaR1)).WillRepeatedly(Return(&r1));
    EXPECT_CALL(manager1, loadResource(((ResourceMeta*)(metaR1)), _)).WillOnce(Return(&r1));

	RR<MockResourceType1> refR1 = devResourceLoader.getResource<MockResourceType1>(std::string("data/someGroup/A piece of data"));

	ASSERT_TRUE(refR1.isValid());
	ASSERT_EQ(refR1.get(), &r1);
}
