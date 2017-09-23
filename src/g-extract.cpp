#include "g-extract.h"
#include <mspack.h>
#include <string.h>
#include <stdexcept>
#include <algorithm>
#include <unshield/libunshield.h>

#ifdef __unix__
#include <sys/stat.h>
#include <limits.h>
#include <malloc.h>

#elif _WIN32
#include <direct.h>
#endif

#ifndef NAME_MAX
#define NAME_MAX FILENAME_MAX
#endif

static const char* cab_error(struct mscab_decompressor* cd);

static bool make_sure_directory_exists(const char* directory)
{
    struct stat dir_stat;
    const char* p = directory;
    bool success = false;
    char* current = NULL;

    while (p && *p)
    {
        if ('/' == *p)
            p++;
        else if (0 == strncmp(p, "./", 2))
            p+=2;
        else if (0 == strncmp(p, "../", 3))
            p+=3;
        else
        {
            const char* slash = strchr(p, '/');
            current = strdup(directory);

            if (slash)
                current[slash-directory] = '\0';

            if (stat(current, &dir_stat) < 0)
            {
#ifdef __MINGW32__
                if (_mkdir(current) < 0)
#else
                if (mkdir(current, 0700) < 0)
#endif
                {
                    free(current);
                    current = NULL;

                    if(strlen(directory)>NAME_MAX)
                        throw std::runtime_error("Directory name must be less than " + std::to_string(NAME_MAX+1) + " characters");
                    else
                        throw std::runtime_error("Failed to create directory " + std::string(directory));


                }
            }

            p = slash;
            free(current);
            current = NULL;
        }
    }

    success = true;

    free(current);
    current = NULL;

    return success;
}
/**
 * Returns a string with an error message appropriate for the last error
 * of the CAB decompressor.
 *
 * @param  cd the CAB decompressor.
 * @return a constant string with an appropriate error message.
 */
static const char* cab_error(struct mscab_decompressor* cd)
{
    switch (cd->last_error(cd))
    {
        case MSPACK_ERR_OPEN:
        case MSPACK_ERR_READ:
        case MSPACK_ERR_WRITE:
        case MSPACK_ERR_SEEK:
            return strerror(errno);
        case MSPACK_ERR_NOMEMORY:
            return "out of memory";
        case MSPACK_ERR_SIGNATURE:
            return "bad CAB signature";
        case MSPACK_ERR_DATAFORMAT:
            return "error in CAB data format";
        case MSPACK_ERR_CHECKSUM:
            return "checksum error";
        case MSPACK_ERR_DECRUNCH:
            return "decompression error";
        default:
            return "unknown error";
    }
}


static void extractCabFile(struct mscab_decompressor* cabd,
                           struct mscabd_file* cabfile,
                           const std::string& targetLocation)
{
    std::string fileNameWithPath = cabfile->filename;
    std::replace(fileNameWithPath.begin(), fileNameWithPath.end(), '\\', '/'); // Windows accepts / as well
    std::string flatFileName;

    // Strip everything but the filename
    if(fileNameWithPath.find_last_of('/') != std::string::npos)
        flatFileName = fileNameWithPath.substr(fileNameWithPath.find_last_of('/'));
    else
        flatFileName = fileNameWithPath;

    std::string targetFile = targetLocation + "/" + flatFileName;

    int err = cabd->extract(cabd, cabfile, targetFile.c_str());
    if (MSPACK_ERR_OK != err)
    {
        throw std::runtime_error(
                "Failed to extract file: " + targetFile + " (" + std::string(cab_error(cabd)) +
                ")");
    }
}

void GExtract::extractInstallerExecutable(const std::string& installerFilePath, const std::string& targetLocation)
{
    int test;

    if (targetLocation.back() == '/' || targetLocation.back() == '\\')
        throw std::runtime_error("Invalid target location");

    MSPACK_SYS_SELFTEST(test);
    if (test != MSPACK_ERR_OK)
    {
        throw std::runtime_error("Failed self test!");
    }


    struct mscab_decompressor* cabd;
    if ((cabd = mspack_create_cab_decompressor(NULL)))
    {
        struct mscabd_cabinet* cab;

        /* search the file for cabinets */
        if (!(cab = cabd->search(cabd, installerFilePath.c_str())))
        {
            if (cabd->last_error(cabd))
            {
                throw std::runtime_error(
                        "Failed to open CAB: " + installerFilePath + " (" + std::string(cab_error(cabd)) + ")");
            } else
            {
                throw std::runtime_error("no valid cabinets found");
            }
        }

        for (struct mscabd_file* file = cab->files; file; file = file->next)
        {
            printf("Installer-File: %s\n", file->filename);
            extractCabFile(cabd, file, targetLocation);
        }
        cabd->close(cabd, cab);

        mspack_destroy_cab_decompressor(cabd);
    } else
    {
        throw std::runtime_error("Failed to create cab-decompressor!");
    }
}

void GExtract::extractInternalCABFile(const std::string& cabFilePath, const std::string& targetLocation)
{
    Unshield* us = unshield_open(cabFilePath.c_str());

    if(!us)
        throw std::runtime_error("Failed to open unshield-cab-archive: " + cabFilePath);

    int numFiles = unshield_file_count(us);

    for(int i = 0; i < numFiles; i++)
    {
        int dir = unshield_file_directory(us, i);
        const char* filename = unshield_file_name(us, i);

        std::string dirname = targetLocation + "/" + std::string(unshield_directory_name(us, dir));

        std::replace(dirname.begin(), dirname.end(), '\\', '/'); // Windows accepts / as well
        make_sure_directory_exists(dirname.c_str());

        std::string targetFile = std::string(dirname) + "/" + std::string(filename);
        unshield_file_save(us, i, targetFile.c_str());

        printf("Unshield-File: %s %s\n", targetFile.c_str());
    }

    unshield_close(us);
}

