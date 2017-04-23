
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "bitengine/Core/Logger.h"
#include "bitengine/Core/EngineConfiguration.h"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

    const char* argvs[] = {BE_PARAM_DEBUG,  BE_PARAM_DEBUG_FILE_ONLY};
	BitEngine::LoggerSetup::Setup(2, argvs);

  return RUN_ALL_TESTS();
}