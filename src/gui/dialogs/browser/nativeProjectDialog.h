#ifndef G_NATIVE_PROJECT_DIALOG_H
#define G_NATIVE_PROJECT_DIALOG_H

#include <string>

namespace giada::v
{
struct NativeProjectSelection
{
	enum class Status { SELECTED, CANCELLED, FAILED_TO_OPEN };
	Status status = Status::FAILED_TO_OPEN;
	std::string directory;
	std::string name;
};

NativeProjectSelection showNativeProjectDialog(bool save, void* owner,
    const std::string& title, const std::string& buttonLabel,
    const std::string& directory, const std::string& name,
    const std::string& nameLabel, const std::string& folderLabel);

struct NativeSampleSelection
{
	using Status = NativeProjectSelection::Status;
	Status status = Status::FAILED_TO_OPEN;
	std::string path;
};

NativeSampleSelection showNativeSampleDialog(void* owner, const std::string& title,
    const std::string& buttonLabel, const std::string& directory);
} // namespace giada::v

#endif
