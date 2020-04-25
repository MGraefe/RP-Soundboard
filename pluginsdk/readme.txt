TeamSpeak 3 plugin development kit
Copyright (c) 2008-2016 TeamSpeak Systems GmbH

Contents
- Demo plugin sourcecode as template for developing own plugins
- Required header files
- Clientlib documentation

bin\plugins\
  Target build directory for the sample project. Instead you might want to configure your output directory
  to the plugins directory within your TeamSpeak 3 client installation.
docs\
  Clientlib documentation 
include\
  Required header files
src\
  Sourcecode of our test plugin

We strongly recommend to build upon the test plugin. Plugins are required to export some special functions,
which already exist in the test plugin. Optional functions may be removed if not used. See code comments
for details.


For questions please visit our forums at http://forum.teamspeak.com
