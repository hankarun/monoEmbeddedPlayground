#include <string>
#include "Path.h"

struct Config
{
	std::string modelName;
	int sampleCount = 1;
};

Config* getConfig();
bool initialize(const Path& directory);
bool loadAssambly(const Path& filename);