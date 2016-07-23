/*
 * Test2.h
 *
 *  Created on: 22.07.2016
 *      Author: danilo
 */

#ifndef MISC_TESTCPLUSPLUSBUILD_H_
#define MISC_TESTCPLUSPLUSBUILD_H_

#include "TestCPlusPlusInterface.h"

#ifdef TESTCPLUSPLUS

#include <string>

class Test2
{
    std::string stringTest;
    bool done;
public:
    void doIt();
    Test2();
    virtual ~Test2();
};

#endif // TESTCPLUSPLUS

#endif /* MISC_TESTCPLUSPLUSBUILD_H_ */
