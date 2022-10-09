#pragma once
#include <filesystem>

class Path
{
    Path(const std::filesystem::path& p)
        : path(p) {}
public:
    Path(const char* value)
    {
        path = std::filesystem::u8path(value);
    }

    Path append(const char* value) const
    {
        return Path(path / value);
    }

    std::string toString() const
    {
        return path.string();
    }

    static Path fromWorkingDir(const char* path)
    {
        auto workingDir = std::filesystem::current_path();
        Path p(workingDir);
        p = p.append(path);
        return p;
    }

private:
    std::filesystem::path path;
};