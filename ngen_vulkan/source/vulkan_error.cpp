
#include "vulkan_error.h"

#define VULKAN_ERROR(id, desc)              \
    case id: return desc

namespace ngen::vulkan {
    //! \brief Converts a Vulkan error code to a user friendly string.
    //! \param result [in] - The Vulkan error code to be converted to a string.
    //! \return A string representation of the supplied Vulkan error code.
    const char* getResultString(VkResult result)
    {
        switch (result)
        {
            #include "vulkan_error_def.h"

            default:
                return "ERROR: UNKNOWN VULKAN ERROR";
        }
    }
}
