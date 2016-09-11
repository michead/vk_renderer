#pragma once

#include "glm\glm.hpp"

#define DEFAULT_WINDOW_WIDTH	1280
#define DEFAULT_WINDOW_HEIGHT	720
#define DEFAULT_SCENE_PATH		"data/head/head.OBJ"

struct VkEngineConfig {
public:
	glm::ivec2 resolution;
	bool fullscreen;
	std::string scenePath;
	ShadingModel shadingModel;

	void parseCmdLineArgs(int argc, char** argv)
	{
		std::vector<std::string> args = std::vector<std::string>(argv + 1, argv + argc);

		if (!args.empty())
		{
			resolution = parseResolution(args);
			fullscreen = parseFlag(args, "-f");
			scenePath = parseOption(args, "-s");
		}
		else
		{
			resolution = { DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT };
			fullscreen = false;
			scenePath = DEFAULT_SCENE_PATH;
		}

		// TODO: This needs to be parsed as well
		shadingModel = ShadingModel::DEFAULT;
	}

private:
	static glm::vec2 parseResolution(std::vector<std::string>& args)
	{
		std::vector<std::string>::iterator it;
		if ((it = std::find(args.begin(), args.end(), "-r")) == args.end())
			return glm::vec2();

		std::string sRes = "";
		if ((*it).size() > 2)
		{
			sRes.substr(2);
		}
		else
		{
			sRes = *(++it);
		}

		int sepPos = sRes.find('x');
		int width = std::atoi(sRes.substr(0, sepPos).c_str());
		int height = std::atoi(sRes.substr(sepPos).c_str());

		return glm::vec2(width, height);
	}

	static std::string parseOption(std::vector<std::string>& args, std::string opt)
	{
		std::vector<std::string>::iterator it;
		if ((it = std::find(args.begin(), args.end(), opt)) == args.end())
			return "";

		return *(++it);
	}

	static bool parseFlag(std::vector<std::string>& args, std::string flag)
	{
		return std::find(args.begin(), args.end(), flag) != args.end();
	}
};