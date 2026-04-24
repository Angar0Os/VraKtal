#include "utils/fileTypeDetector.h"
#include <fstream>
#include <vector>
#include <string>

FileType FileTypeDetector::DetectFileType(const std::filesystem::path& path) 
{
	
	std::error_code ec;

	if (std::filesystem::is_directory(path, ec))
	{
		return FileType::Folder; // à ajouter dans ton enum
	}

	if (ec)
	{
		return FileType::Unknown;
	}

	std::ifstream file(path, std::ios::binary);
	std::vector<unsigned char> buffer(12, 0);
	file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
	size_t bytesRead = file.gcount();

	if (!file.is_open())
	{
		return FileType::Unknown;
	}

	if (bytesRead < 2) {
		return FileType::Unknown;
	}

	if (bytesRead >= 4 && buffer[0] == 0x89 && buffer[1] == 0x50 && buffer[2] == 0x4E && buffer[3] == 0x47) {
		return FileType::ImagePNG;
	}

	if (bytesRead >= 3 && buffer[0] == 0xFF && buffer[1] == 0xD8 && buffer[2] == 0xFF) {
		return FileType::ImageJPEG;
	}

	if (bytesRead >= 12 && buffer[0] == 0x52 && buffer[1] == 0x49 && buffer[2] == 0x46 && buffer[3] == 0x46 &&
		buffer[8] == 0x57 && buffer[9] == 0x41 && buffer[10] == 0x56 && buffer[11] == 0x45) {
		return FileType::AudioWAV;
	}

	if (bytesRead >= 3) {
		if (buffer[0] == 0x49 && buffer[1] == 0x44 && buffer[2] == 0x33) {
			return FileType::AudioMP3;
		}
		if (buffer[0] == 0xFF && (buffer[1] == 0xFB || buffer[1] == 0xF3 || buffer[1] == 0xF2)) {
			return FileType::AudioMP3;
		}
	}

	if (bytesRead >= 4 && buffer[0] == 0x67 && buffer[1] == 0x6C && buffer[2] == 0x54 && buffer[3] == 0x46) {
		return FileType::MeshGLB;
	}

	if (IsTextFile(path)) {
		file.seekg(0);
		std::vector<char> textBuffer(512);
		file.read(textBuffer.data(), textBuffer.size());
		size_t textBytes = file.gcount();
		std::string content(textBuffer.data(), textBytes);

		// glTF JSON: starts with { and contains "asset" field
		if (content[0] == '{' && content.find("\"asset\"") != std::string::npos) {
			return FileType::MeshGLTF;
		}

		// Common patterns: v (vertex), vt (texture coord), vn (normal), f (face)
		if (content.find("v ") == 0 || content.find("vt ") == 0 || content.find("vn ") == 0 ||
			content.find("f ") == 0 || content.find("o ") == 0 || content.find("g ") == 0 ||
			content.find("# Blender") != std::string::npos || content.find("# Wavefront") != std::string::npos) {
			return FileType::MeshOBJ;
		}

		return FileType::Unknown;
	}

	return FileType::Unknown;
}

bool FileTypeDetector::IsTextFile(const std::filesystem::path& path)
{
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open()) {
		return false;
	}

	std::vector<char> buffer(512);
	file.read(buffer.data(), buffer.size());
	size_t bytesRead = file.gcount();

	// Check if all bytes are printable ASCII or common whitespace
	for (size_t i = 0; i < bytesRead; ++i) {
		unsigned char c = static_cast<unsigned char>(buffer[i]);
		if (c < 0x20 && c != '\n' && c != '\r' && c != '\t') {
			return false;
		}
		if (c > 0x7E && c < 0x80) { // Extended ASCII that's not UTF-8
			return false;
		}
	}

	return true;
}