# Contribution Guide

This document describes some points about the contribution process for mcHF firmware and bootloader.

The maintainer of this GitHub is DF8OE.

The project is licensed under GPLv3 and is being developed within an open community. Maintainer merges pull-requests, fixes critical bugs and manages releases. Participation is appreciated.

### Getting Started

The easiest way to get started and build your own firmware is to use gitpod. 
*  Fork this repository in your own GitHub account
*  Open this fork in the browser, press the "gitpod" button and follow the instructions to get an gitpod account
*  Once you see the gitpod workspace
```
cd mchf-eclipse
make all
```
* Download the resulting binary `fw-mchf.bin`
* Now it is time to learn how to use git (with gitpod) in order to contribute your own changes.

For more information and details how to build and debug on your own machine, see the [Wiki](https://github.com/df8oe/UHSDR/wiki/Topics:-UHSDR-SW-Development)

### Pull-requests

If you fixed or added something useful to the project, you can send pull-request. It will be reviewed by coders and/or maintainer and accepted, or commented for rework, or declined.

We love pull requests. Here's a quick guide:

  * Make sure you have a [GitHub](https://www.github.com) account and read more about [pull requests](http://help.github.com/pull-requests/)
  * Fork the UHSDR repository in GitHub
  * Clone the your forked repository locally to your PC (`git clone ...`)
  * Add the main repository as "main" (`git remote add main https://github.com/df8oe/UHSDR.git`)
  * Checkout the branch you want to modify (`git checkout active-devel`)
  * Create your feature branch (`git branch my-new-feature`)
  * Commit your changes and provide good explaination in the commit message (`git commit -m`)
  * If you have multiple, consecutive commits for the same logical change, please try combining them into one commit, see for instance https://www.atlassian.com/git/tutorials/rewriting-history. Don't combine very different things in a very large, single commit, this makes it harder to identify the source of problems. 
  * If it took a while to complete the changes, consider rebasing (`git fetch main; git rebase main/active-devel`) before pushing. This is generally strongly recommended to avoid nasty surprise during integration for the main repository maintainer.
  * Push the branch to your GitHub repository (`git push` / `git push -f` if you did an rebase)
  * It is strongly recommended that you do an operational test using a mcHF before you start a pull request.
  * Create new pull request at http://www.github.com/df8oe/UHSDR . Make sure to select the right branch (active-devel) as target.
  
### Bugs
If you found an error, mistype or any other flawback in the project, please report about it using Issues. The more details you provide, the easier it can be reproduced and the faster it can be fixed.<br><br>

### Features
It you've got an idea about a new feature, it's most likely that you have do implement it on your own. If you cannot implement the feature, but it is very important, you can add a task in issues and tag it with "REQUEST:". Feature requests are discussed and may be realized shortly, later or never - there is no guarantee that requests will be accepted.<br><br>

### Coding Style Guide
The intention of these guidelines is to get easy to read, robust and maintainable code. In open source, other individuals should be able to read AND understand your code.
No one will be happy with all rules, but as long as the rules are usable, please consider following them. If you think the rules are not good, create a GitHub Issue and start the discussion.

* __Code Formatting__: Leave aside all your personal preferences and follow the style used in most UHSDR files. 
    Some files come directly from external sources, we don't reformat these.
* __Indention/Parentheses__: We use 4 spaces indent, no tabs. This allows all to see a proper indention no matter what tab size is configured in the editor. Parentheses are _always_ on a separate line, and are aligned with the statement above. If you are not following this strictly, at some point a automated code formatter will do it for you but this creates unnecessary "GitHub noise", so if possible stick to this rule.

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
	if (c == 7)
		a = 3;
	else
		b = 2;
```
	
  We use
  
```C
	if ( c == 7)
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

#### File headers
All code files should bear the appropriate license in them. I.e. if using external files as a whole (like we do with FreeDV), leave the original license header in place. For our own files, license GPLv3 is to be used.
Please use the following header:

```
/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                **
 **                                        UHSDR                                   **
 **               a powerful firmware for STM32 based SDR transceivers             **
 **                                                                                **
 **--------------------------------------------------------------------------------**
 **                                                                                **
 **  Description:   Please provide one                                             **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/
```	
You may add Author / Maintainer fields, if this is appropriate.

### Auto Formatting
The Eclipse feature could help you to follow these rules:
1. Right click on the project and hit 'Properties' or just Alt+Enter
2. Go to 'C/C++ General -> Formatter' and check 'Enable project specific settings'
<p align="center">
  <img src="https://user-images.githubusercontent.com/23377892/50575339-b2c09b00-0dcb-11e9-8e9d-2987cb7c8cd6.jpg">
</p>

3. There is 'UHSDR [K&R modified]' presettings available in list. Choose it and hit 'Apply and CLose'
<p align="center">
  <img src="https://user-images.githubusercontent.com/23377892/50575341-b2c09b00-0dcb-11e9-918b-37f97a403355.jpg">
</p>

4. If there is no 'UHSDR [K&R modified]' settings you can import it from file UHSDR_Eclipse_formatting_settings.xml in the mchf-eclipse/.settings/ folder.
<p align="center">
  <img src="https://user-images.githubusercontent.com/23377892/50575340-b2c09b00-0dcb-11e9-9372-ab9f07ddaac8.jpg">
</p>

5. And finally you can select part of your code and press Ctrl+Shift+F
<p align="center">
  <img src="https://user-images.githubusercontent.com/23377892/50575338-b2c09b00-0dcb-11e9-8bd1-86812416e632.jpg">
</p>

The selected code would been formatted accordingly the rules. Nice!

### Boy Scout Rule

The Boy Scout Rule can be summarized as: **Leave your code better than you found it.**
<p align="center">
  <img src="https://user-images.githubusercontent.com/23377892/50579382-8bd98780-0e12-11e9-940b-d78186df61e6.jpg">
</p>

Boy Scouts have a rule regarding camping, that they should leave the campground cleaner than they found it. They don’t take it upon themselves to ensure the entire campground is cleaned up, but neither do they simply trash the place but plan on coming back one week out of the year to do a proper cleanup job. By ensuring that the campground is cleaner when they leave than it was when they arrived, they can guarantee that they are doing no harm, at least when it comes to the cleanliness of the site. [read more...]()

So, let's do the same with the code in conscious and constant manner. Thank you in advance!

### WIKI
To keep slowly changing documents up-2-date it will be great if you help to set up such documents in the WIKI area. They are quick-&-easy access to newcomers and will pleasure building and reduce questions which repeat frequently. The following documents can be accessed in the WIKI area:

  * Operating manual
  * Adjustment and configuration manual
  * Modification list
  * Guides for setup of firmware development software
  * Technical specifications
