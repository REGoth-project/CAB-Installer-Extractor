#pragma once
#include <string>

namespace GExtract
{
    /**
     * Extracts the Gothic installer given by "file" to the given target location on the file system.
     *
     * @param installerFilePath   Installer to extract
     * @param targetLocation      Location on where to put the extracted files
     * @throws std::runtime_error
     */
    void extractInstallerExecutable(const std::string& installerFilePath, const std::string& targetLocation);
    void extractInternalCABFile(const std::string& cabFilePath, const std::string& targetLocation);
}

/*int main(int argc, char** argv)
{
    GExtract::extractInstallerExecutable(argv[1], "ext");
    GExtract::extractInternalCABFile("ext/data1.cab", "ext/us");

    return 0;
}*/
