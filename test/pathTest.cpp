#include "util/path.hpp"
#include <assert.h>
#include <gtest/gtest.h>
#include <iostream>

TEST(path, init) {
  path p = "./a/b/c";
  EXPECT_EQ(p.length(), 3);
  EXPECT_EQ(p[0], "a");
  EXPECT_EQ(p[1], "b");
  EXPECT_EQ(p[2], "c");
}
TEST(path, join) {
  path result = path::join({"/a/b/c", "../.././b"});
  EXPECT_EQ(result.string(), "/a/b");
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}