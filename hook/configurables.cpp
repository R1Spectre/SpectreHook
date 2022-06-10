#include <string>
#include "pch.h"
#include "configurables.h"

std::string GetSpectrePrefix()
{
	return SPECTRE_FOLDER_PREFIX;
}

void ParseConfigurables()
{

	char* clachar = strstr(GetCommandLineA(), "-profile=");
	if (clachar)
	{
		std::string cla = std::string(clachar);
		if (strncmp(cla.substr(9, 1).c_str(), "\"", 1))
		{
			int space = cla.find(" ");
			std::string dirname = cla.substr(9, space - 9);
			Log::Info("Found profile in command line arguments: %s", dirname.c_str());
			SPECTRE_FOLDER_PREFIX = dirname;
		}
		else
		{
			std::string quote = "\"";
			int quote1 = cla.find(quote);
			int quote2 = (cla.substr(quote1 + 1)).find(quote);
			std::string dirname = cla.substr(quote1 + 1, quote2);
			Log::Info("Found profile in command line arguments: %s", dirname.c_str());
			SPECTRE_FOLDER_PREFIX = dirname;
		}
	}
	else
	{
		SPECTRE_FOLDER_PREFIX = "R1Spectre";
		Log::Info("Profile was not found in command line arguments. Using default: %s", SPECTRE_FOLDER_PREFIX.c_str());
	}
}
