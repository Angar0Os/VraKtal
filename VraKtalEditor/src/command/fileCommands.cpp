#include "command/fileCommands.h"
#include <iostream>

namespace command {
	void ClearBackupDirectory()
	{
		std::filesystem::path backupDir = GetBackupDirectory();
		if (std::filesystem::exists(backupDir)) {
			try {
				std::filesystem::remove_all(backupDir);
			}
			catch (const std::exception& e) {
				std::cerr << "Failed to clear backup directory: " << e.what() << std::endl;
			}
		}
	}

	// DeleteFileCommand

	DeleteFileCommand::DeleteFileCommand(const std::vector<std::filesystem::path>& paths) : m_paths(paths)
	{
		std::filesystem::path backupDir = GetBackupDirectory();
		std::filesystem::create_directories(backupDir);

		for (const auto& path : m_paths) {
			m_backupPaths.push_back(backupDir / path.filename());
		}
	}

	void DeleteFileCommand::Execute()
	{
		if (m_executed) {
			return;
		}

		try	{
			for (size_t i = 0; i < m_paths.size(); ++i)	{
				if (std::filesystem::exists(m_paths[i])) {
					if (std::filesystem::is_directory(m_paths[i])) {
						std::filesystem::copy(m_paths[i], m_backupPaths[i], std::filesystem::copy_options::recursive);
					}
					else {
						std::filesystem::copy_file(m_paths[i], m_backupPaths[i], std::filesystem::copy_options::overwrite_existing);
					}

					std::filesystem::remove_all(m_paths[i]);
				}
			}

			m_executed = true;
		}
		catch (const std::exception& err) {
			std::cerr << "Delete command failed: " << err.what() << std::endl;
		}
	}

	void DeleteFileCommand::Undo()
	{
		if (!m_executed) {
			return;
		}

		try {
			for (size_t i = 0; i < m_backupPaths.size(); ++i) {
				if (std::filesystem::exists(m_backupPaths[i])) {
					if (std::filesystem::is_directory(m_backupPaths[i])) {
						std::filesystem::copy(m_backupPaths[i], m_paths[i], std::filesystem::copy_options::recursive);
					} else {
						std::filesystem::copy_file(m_backupPaths[i], m_paths[i]);
					}

					std::filesystem::remove_all(m_backupPaths[i]);
				}
			}

			m_executed = false;
		}
		catch (const std::exception& err) {
			std::cerr << "Undo delete command failed: " << err.what() << std::endl;
		}
	}

	std::string DeleteFileCommand::GetDescription() const
	{
		return "Delete " + std::to_string(m_paths.size()) + " item(s)";
	}

	// RenameFileCommand

	RenameFileCommand::RenameFileCommand(const std::filesystem::path& oldPath, const std::filesystem::path& newPath)
		: m_oldPath(oldPath), m_newPath(newPath)
	{
	}

	void RenameFileCommand::Execute()
	{
		try {
			std::filesystem::rename(m_oldPath, m_newPath);
		}
		catch (const std::exception& err) {
			std::cerr << "Rename command failed: " << err.what() << std::endl;
		}
	}

	void RenameFileCommand::Undo()
	{
		try {
			std::filesystem::rename(m_newPath, m_oldPath);
		}
		catch (const std::exception& err) {
			std::cerr << "Undo rename command failed: " << err.what() << std::endl;
		}
	}

	std::string RenameFileCommand::GetDescription() const
	{
		return "Rename " + m_oldPath.filename().string() + " to " + m_newPath.filename().string();
	}

	// CreateFolderCommand

	CreateFolderCommand::CreateFolderCommand(const std::filesystem::path& folderPath) : m_folderPath(folderPath)
	{
	}

	void CreateFolderCommand::Execute()
	{
		try {
			std::filesystem::create_directory(m_folderPath);
		}
		catch (const std::exception& err) {
			std::cerr << "Create folder command failed: " << err.what() << std::endl;
		}
	}

	void CreateFolderCommand::Undo()
	{
		try {
			if (std::filesystem::exists(m_folderPath)) {
				std::filesystem::remove(m_folderPath);
			}
		}
		catch (const std::exception& err) {
			std::cerr << "Undo create folder command failed: " << err.what() << std::endl;
		}
	}

	std::string CreateFolderCommand::GetDescription() const
	{
		return "Create folder " + m_folderPath.filename().string();
	}

	// CopyFileCommand

	CopyFileCommand::CopyFileCommand(const std::vector<std::filesystem::path>& sourcePaths, const std::filesystem::path& destinationFolder)
		: m_sourcePaths(sourcePaths)
	{
		for (const auto& sourcePath : sourcePaths) {
			m_destPaths.push_back(destinationFolder / sourcePath.filename());
		}
	}

	void CopyFileCommand::Execute()
	{
		try {
			for (size_t i = 0; i < m_sourcePaths.size(); ++i) {
				if (std::filesystem::is_directory(m_sourcePaths[i])) {
					std::filesystem::copy(m_sourcePaths[i], m_destPaths[i], std::filesystem::copy_options::recursive);
				} else {
					std::filesystem::copy_file(m_sourcePaths[i], m_destPaths[i]);
				}
			}
		}
		catch (const std::exception& err) {
			std::cerr << "Copy command failed: " << err.what() << std::endl;
		}
	}

	void CopyFileCommand::Undo()
	{
		try {
			for (const auto& destPath : m_destPaths) {
				if (std::filesystem::exists(destPath)) {
					std::filesystem::remove_all(destPath);
				}
			}
		}
		catch (const std::exception& err) {
			std::cerr << "Undo copy command failed: " << err.what() << std::endl;
		}
	}

	std::string CopyFileCommand::GetDescription() const
	{
		return "Copy " + std::to_string(m_sourcePaths.size()) + " item(s)";
	}

	// MoveFileCommand

	MoveFileCommand::MoveFileCommand(const std::vector<std::filesystem::path>& sourcePaths, const std::filesystem::path& destinationFolder)
		: m_sourcePaths(sourcePaths)
	{
		for (const auto& sourcePath : sourcePaths) {
			m_destPaths.push_back(destinationFolder / sourcePath.filename());
		}
	}

	void MoveFileCommand::Execute()
	{
		try {
			for (size_t i = 0; i < m_sourcePaths.size(); ++i) {
				std::filesystem::rename(m_sourcePaths[i], m_destPaths[i]);
			}
		}
		catch (const std::exception& err) {
			std::cerr << "Move command failed: " << err.what() << std::endl;
		}
	}

	void MoveFileCommand::Undo()
	{
		try {
			for (size_t i = 0; i < m_destPaths.size(); ++i) {
				std::filesystem::rename(m_destPaths[i], m_sourcePaths[i]);
			}
		}
		catch (const std::exception& err) {
			std::cerr << "Undo move command failed: " << err.what() << std::endl;
		}
	}

	std::string MoveFileCommand::GetDescription() const
	{
		return "Move " + std::to_string(m_sourcePaths.size()) + " item(s)";
	}

	// ImportFileCommand

	ImportFileCommand::ImportFileCommand(const std::vector<std::filesystem::path>& sourcePaths, const std::filesystem::path& destinationFolder)
		: m_sourcePaths(sourcePaths)
	{
		for (const auto& sourcePath : sourcePaths) {
			m_destPaths.push_back(destinationFolder / sourcePath.filename());
		}
	}

	void ImportFileCommand::Execute()
	{
		try {
			for (size_t i = 0; i < m_sourcePaths.size(); ++i) {
				std::filesystem::copy_file(m_sourcePaths[i], m_destPaths[i]);
			}
		}
		catch (const std::exception& err) {
			std::cerr << "Import command failed: " << err.what() << std::endl;
		}
	}

	void ImportFileCommand::Undo()
	{
		try {
			for (const auto& destPath : m_destPaths) {
				if (std::filesystem::exists(destPath)) {
					std::filesystem::remove(destPath);
				}
			}
		}
		catch (const std::exception& err) {
			std::cerr << "Undo import command failed: " << err.what() << std::endl;
		}
	}

	std::string ImportFileCommand::GetDescription() const
	{
		return "Import " + std::to_string(m_sourcePaths.size()) + " file(s)";
	}
}