# Contribution Guide

This document describes some points about the contribution process for mcHF firmware and bootloader.

The maintainer of this GitHub is DF8OE.

The project is free for non-commercial use and is being developed within an open community. Maintainer merges pull-requests, fixes critical bugs and manages releases. Participation is appreciated.

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
  * Checkout the branch you want to modify (`git checkout devel-DF8OE`)
  * Create your feature branch (`git branch my-new-feature`)
  * Commit your changes and provide good explaination in the commit message (`git commit -m`)
  * If it took a while to complete the changes, consider rebasing (`git fetch main; git rebase main/devel-DF8OE`) before pushing
  * Push the branch to your GitHub repository (`git push`)
  * It is strongly recommended that you do an operational test using a mcHF before you start a pull request.
  * Create new pull request at http://www.github.com/df8oe/mchf-github . Make sure to select the right branch (devel-DF8OE) as target.
  
### Bugs
If you found an error, mistype or any other flawback in the project, please report about it using Issues. The more details you provide, the easier it can be reproduced and the faster it can be fixed.<br><br>

### Features
It you've got an idea about a new feature, it's most likely that you have do implement it on your own. If you cannot implement the feature, but it is very important, you can add a task in issues and tag it with "REQUEST:". Feature requests are discussed and may be realized shortly, later or never - there is no guarantee that requests will be accepted.<br><br>

### WIKI
To keep slowly changing documents up-2-date it will be great if you help to set up such documents in the WIKI area. They are quick-&-easy access to newcomers and will pleasure building and reduce questions which repeat frequently. The following documents can be accessed in the WIKI area:

  * Operating manual
  * Adjustment and configuration manual
  * Modification list
  * Guides for setup of firmware development software
  * Technical specifications
