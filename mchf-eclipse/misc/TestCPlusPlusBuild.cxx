/*
 * Test2.cpp
 *
 *  Created on: 22.07.2016
 *      Author: danilo
 */

#include "TestCPlusPlusBuild.h"
#include "TestCPlusPlusInterface.h"

#ifdef TESTCPLUSPLUS

#include <string>


Test2 t2;

void test_call_cpp()
{
    t2.doIt();
}

void Test2::doIt()
{
    done = true;
}

Test2::Test2():  stringTest("TestOK"), done(false)
{
    // TODO Auto-generated constructor stub
}

Test2::~Test2()
{
    // TODO Auto-generated destructor stub
}

#endif
