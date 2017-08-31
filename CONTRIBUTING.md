# Contribution Guide

This document describes some points about the contribution process for mcHF firmware and bootloader.

The maintainer of this GitHub is DF8OE.

The project is licensed under GPLv3 and is being developed within an open community. Maintainer merges pull-requests, fixes critical bugs and manages releases. Participation is appreciated.

### Getting Started

Soon we'll point here to a document describing how to setup an environment to compile, upload and debug firmware. 

Meanwhile you may try the available .bin files and follow the instructions in the 
mchf-eclipse/bootloader/readme.txt how to upload these. 

### Pull-requests

If you fixed or added something useful to the project, you can send pull-request. It will be reviewed by coders and/or maintainer and accepted, or commented for rework, or declined.

We love pull requests. Here's a quick guide:

  * Make sure you have a [GitHub](https://www.github.com) account and read more about [pull requests](http://help.github.com/pull-requests/)
  * Fork it in GitHub
  * Clone it locally to your PC (`github clone ...`)
  * Add the main repository as "main" (`git remote add main https://github.com/df8oe/mchf-github.git`)
  * Checkout the branch you want to modify (`git checkout active-devel`)
  * Create your feature branch (`git branch my-new-feature`)
  * Commit your changes and provide good explaination in the commit message (`git commit -m`)
  * If it took a while to complete the changes, consider rebasing (`git fetch main; git rebase main/active-devel`) before pushing
  * Push the branch to your GitHub repository (`git push`)
  * It is strongly recommended that you do an operational test using a mcHF before you start a pull request.
  * Create new pull request at http://www.github.com/df8oe/mchf-github . Make sure to select the right branch (active-devel) as target.
  
### Bugs
If you found an error, mistype or any other flawback in the project, please report about it using Issues. The more details you provide, the easier it can be reproduced and the faster it can be fixed.<br><br>

### Features
It you've got an idea about a new feature, it's most likely that you have do implement it on your own. If you cannot implement the feature, but it is very important, you can add a task in issues and tag it with "REQUEST:". Feature requests are discussed and may be realized shortly, later or never - there is no guarantee that requests will be accepted.<br><br>

### Coding Style Guide
The intention of these guidelines is to get easy to read, robust and maintainable code. In open source, other individuals should be able to read AND understand your code.
No one will be happy with all rules, but as long as the rules are usable, please consider following them. If you think the rules are not good, create a GitHub Issue and start the discussion.

  * __Code Formatting__: Leave aside all your personal preferences and follow the style used in most UHSDR files. 
    Some files come directly from external sources, we don't reformat these.
  *__Indention/Parentheses__: We use 4 spaces indent, no tabs. This allows all to see a proper indention no matter what tab size is configured in the editor. Parentheses are _always_ on a separate line, and are aligned with the statement above. If you are not following this strictly, at some point a automated code formatter will do it for you but this creates unnecessary "GitHub noise", so if possible stick to this rule. 

```C
 	bool foo(int in)
	{ 
		bool retval = 0;
		if (in = 0)
		{
			retval = 1;
		}
		return retval;
	}
```    
  * __No warnings__: Compiler should not issue any warning when it compiles UHSDR code.  	
  * __Single Return__: A function has only one *return* statement. Use a designated return value variable (retval) if necessary.
```C
	bool foo(int in)
	{ 
	
		if (in = 0)
		{
			return in;
		}
		else
		{
			return in*2;
		}
	}
```
 	We use
```C
 	bool foo(int in)
	{ 
		bool retval;
		if (in = 0)
		{
			retval = in;
		}
		else
		{
			retval = in*2;
		}
		return retval;
	}
```
 	
  * __Single Line Conditional statements__: please, always use parentheses. C is not Python and indention is easily broken, makes code very hard to debug. 
   
```C
	if 
		a = 3;
	else
		b = 2;
```
	
  We use
  
```C
	if 
	{
		a = 3;
	}
	else
	{
		b = 2;
	}
```
	
  * __Pointer Type Casting__: should be avoided or commented on: Do not remove type warnings just by using a cast operator! Identify the root cause. Most cases there will be a coding problem. If such a pointer cast is necessary, please provide a comment.  
```C
	void foo_with_float_pointer(float32_t* fp)
	{
		fp = sqrtf(*fp);
	}

    void bar_wrong(int32_t* value_p)
	{
	    // NOT WORKING CORRECTLY, BUT COMPILER WILL NOT COMPLAIN
	    foo_with_float_pointer((float32_t*)value_p);
	}
	
	void bar_correct(int32_t* value_p)
	{
	    // CORRECT
	    float32_t value_float = *value_p; 
	    foo_with_float_pointer(value_float);
	    *value_p = value_float; 
	}
```
	

### WIKI
To keep slowly changing documents up-2-date it will be great if you help to set up such documents in the WIKI area. They are quick-&-easy access to newcomers and will pleasure building and reduce questions which repeat frequently. The following documents can be accessed in the WIKI area:

  * Operating manual
  * Adjustment and configuration manual
  * Modification list
  * Guides for setup of firmware development software
  * Technical specifications
