/*
 * main.cpp
 *
 *  Created on: 03.03.2017
 *      Copyright 2017 Michael Speck
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <cstdlib>
#include <list>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

#include "widgets.h"
#include "parser.h"
#include "data.h"
#include "mapview.h"
#include "gui.h"
#include "lgeditor_config.h"

static bool dir_exists(const std::string &path)
{
	if (path.empty())
		return false;
#if defined(_WIN32)
	struct _stat info;
	return _stat(path.c_str(), &info) == 0 && (info.st_mode & _S_IFDIR);
#else
	struct stat info;
	return stat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode);
#endif
}

static std::string resolve_path(const char *env_var,
		const std::string &build_default,
		const std::string &install_default)
{
	if (env_var)
	{
		const char *override_path = std::getenv(env_var);
		if (override_path && *override_path)
		{
			return std::string(override_path);
		}
	}

	if (dir_exists(build_default))
		return build_default;

	return install_default;
}

std::string rcpath = resolve_path("LGEDITOR_DATADIR",
		LGEDITOR_DATADIR_BUILD,
		LGEDITOR_DATADIR_INSTALL);
std::string lgenpath = resolve_path("LGEDITOR_LGENERAL_DIR",
		LGEDITOR_LGENERAL_DIR_BUILD,
		LGEDITOR_LGENERAL_DIR_INSTALL);
Data *data = NULL; /* global for anyone to access */
GUI *gui = NULL; /* global for anyone to access */

void print_help()
{
	std::string helpstr =
		"Please read README for more detailed information.\n"
		"Usage: lgeditor FILE [WIDTH HEIGHT]\n"
		"FILE is either a map or scenario file.\n"
		"If file doesn't not exist a basic scenario with empty map (30x30)"
		"is generated.\n"
		"If it is a map, generic scenario info is added\n"
		"(20 turns, default victory conditions).\n"
		"You can edit map, flags and units. Everything else has to\n"
		"be changed by a text editor and remains unchanged on\n"
		"saving/loading in LGeneral editor.\n"
		"Tip: Copy and work a file as a user in your home and then\n"
		"copy it back to LGeneral data directory for safety.\n"
		"LGeneral needs to be 1.4.0 or newer to load it properly.";
	printf("%s\n", helpstr.c_str());
}

int main(int argc, char **argv)
{
	std::string fname;
	int w,h;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
		SDL_Log("SDL_Init failed: %s\n", SDL_GetError());
	if (TTF_Init() < 0)
		SDL_Log("TTF_Init failed: %s\n", SDL_GetError());
	
	print_help();

	/* read basic options */
	fname = "./newmap";
	w = 30;
	h = 30;
	if (argc > 1)
		fname = argv[1];
	if (argc > 2) {
		w = atoi(argv[2]);
		if (w % 2)
			w++;
		if (w > MAXMAPW)
			w = MAXMAPW;
	}
	if (argc > 3) {
		h = atoi(argv[3]);
		if (h % 2)
			h++;
		if (h > MAXMAPH)
			h = MAXMAPH;
	}

	new MainWindow("Window", 800, 600);
	data = new Data(w, h);
	gui = new GUI();
	gui->nedit->setText(fname);
	data->loadScenario(fname); /* might fail */

	gui->run();

	TTF_Quit();
	SDL_Quit();
	return 0;
}
