#pragma once

#include <filesystem>

enum class FileType {
	Unknown,
	ImagePNG,
	ImageJPEG,
	AudioMP3,
	AudioWAV,
	MeshGLB,
	MeshGLTF,
	MeshOBJ
};

inline std::ostream& operator<<(std::ostream& os, FileType type)
{
	switch (type) {
		case FileType::Unknown:        return os << "Unknown";
		case FileType::ImagePNG:       return os << "ImagePNG";
		case FileType::ImageJPEG:      return os << "ImageJPEG";
		case FileType::AudioMP3:       return os << "AudioMP3";
		case FileType::AudioWAV:       return os << "AudioWAV";
		case FileType::MeshGLB:        return os << "MeshGLB";
		case FileType::MeshGLTF:       return os << "MeshGLTF";
		case FileType::MeshOBJ:        return os << "MeshOBJ";
		default:                       return os << "Unknown(" << static_cast<int>(type) << ")";
	}
}

class FileTypeDetector {
private:
	static bool IsTextFile(const std::filesystem::path& path);
public:
	static FileType DetectFileType(const std::filesystem::path&);
};