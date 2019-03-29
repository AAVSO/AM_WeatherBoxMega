#ifndef _JAMES_SYNGE_TEST_H_
#define _JAMES_SYNGE_TEST_H_

#include <Arduino.h>

namespace {
void AssertFailedAt(const char* file, unsigned long line) {
  Serial.println();
  Serial.print("Assertion failed at ");
  Serial.print(file);
  Serial.print(":");
  Serial.println(line);
}
}

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define ASSERT_FAILED_AT AssertFailedAt(__FILE__, __LINE__)
  // Serial.println(); \
  // AssertFailedAt
  //  Serial.println("Assertion failed at " TOSTRING(__LINE__) " in file " __FILE__)

#define EXPECT_TRUE(exp) if (exp) ; else {ASSERT_FAILED_AT; Serial.println("NOT TRUE: " #exp); }
#define EXPECT_FALSE(exp) if (!exp) ; else {ASSERT_FAILED_AT; Serial.println("NOT FALSE: " #exp); }
#define EXPECT_EQ(a,b) if (a == b) ; else {ASSERT_FAILED_AT; \
  Serial.println("NOT TRUE: " #a " == " #b); \
  Serial.print(#a " is: "); \
  Serial.println(a); \
  Serial.print(#b " is: "); \
  Serial.println(b); \
}

#define ASSERT_TRUE(exp) if (exp) ; else {ASSERT_FAILED_AT; Serial.println("NOT TRUE: " #exp); return; }
#define ASSERT_FALSE(exp) if (!exp) ; else {ASSERT_FAILED_AT; Serial.println("NOT FALSE: " #exp); return; }
#define ASSERT_EQ(a, b) if (a == b) ; else {ASSERT_FAILED_AT; \
  Serial.println("NOT TRUE: " #a " == " #b); \
  Serial.print(#a " is: "); \
  Serial.println(a); \
  Serial.print(#b " is: "); \
  Serial.println(b); \
  return; \
}
#define ASSERT_NE(a, b) if (!(a == b)) ; else {ASSERT_FAILED_AT; \
  Serial.println("NOT TRUE: !(" #a " == " #b ")"); \
  Serial.print(#a " is: "); \
  Serial.println(a); \
  Serial.print(#b " is: "); \
  Serial.println(b); \
  return; \
}

#endif  // _JAMES_SYNGE_TEST_H_
