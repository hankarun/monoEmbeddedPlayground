#include <string>

struct Config
{
	std::string modelName;
	int sampleCount = 1;
};

Config* getConfig();
bool initialize(const char*);