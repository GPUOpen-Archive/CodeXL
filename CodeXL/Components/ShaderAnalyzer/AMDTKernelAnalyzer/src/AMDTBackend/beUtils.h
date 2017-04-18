#ifndef beUtils_h__
#define beUtils_h__

// Device info.
#include <DeviceInfo.h>
#include <AMDTBackend/beDataTypes.h>
#include <AMDTBaseTools/Include/gtString.h>

class beUtils
{
public:

    // Convert HW generation to its corresponding numerical value.
    static bool GdtHwGenToNumericValue(GDT_HW_GENERATION hwGen, size_t& gfxIp);

    // Convert HW generation to its corresponding string representation.
    static bool GdtHwGenToString(GDT_HW_GENERATION hwGen, std::string& hwGenAsStr);

    // Predicate to be used for sorting HW devices.
    static bool GfxCardInfoSortPredicate(const GDT_GfxCardInfo& a, const GDT_GfxCardInfo& b);

    // Deletes the physical files from the file system.
    static void DeleteOutputFiles(const beProgramPipeline& outputFilePaths);

    // Deletes a physical file from the file system.
    static void DeleteFile(const gtString& filePath);

private:
    // No instances for this class, as this is a static utility class.
    beUtils();
    beUtils(const beUtils& other);
    ~beUtils();
};

#endif // beUtils_h__
