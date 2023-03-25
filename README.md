# KeyLoggerApplication

This software is created for educational purposes, for security research. Supplied as is.
Build requires POCO Library and ICU libraries installed. Accordingly, the paths to these libraries must be updated in each application for a successful build.

**HookDll** - a library that is injected into the user process.

**InstallService** is an application that allows you to install and run services in a simple way.

**KeyloggerService** - Windows service that communicates with the server located in HookDll and receives data about the pressed key, the name of the application.

The KeyloggerService works with stop words and saves data only when a stop word and its stop subword have been found.
All actions with the installation must be carried out on behalf of the administrator.

# Additional info

**tree.json** - Stop word tree.

**KeyloggerService.properties** - Settings for KeyloggerService and HookDll

During installation, installer put settings to windows registry.

KeyConstants.cpp must will be save as UTF-16 encoding.