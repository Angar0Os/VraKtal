#ifndef EDITOR_COMMAND_FILECOMMANDS_H
#define EDITOR_COMMAND_FILECOMMANDS_H
#pragma once

#include <vector>
#include <filesystem>

#include "command.h"

namespace command
{
	class DeleteFileCommand : public Command
	{
	private:
		std::vector<std::filesystem::path> m_paths;
		std::vector<std::filesystem::path> m_backupPaths;
		bool m_executed = false;

	public:
		explicit DeleteFileCommand(const std::vector<std::filesystem::path>& paths);

		void Execute() override;
		void Undo() override;
		std::string GetDescription() const override;
	};

	class RenameFileCommand : public Command
	{
	private:
		std::filesystem::path m_oldPath;
		std::filesystem::path m_newPath;

	public:
		RenameFileCommand(const std::filesystem::path& oldPath, const std::filesystem::path& newPath);

		void Execute() override;
		void Undo() override;
		std::string GetDescription() const override;
	};

	class CreateFolderCommand : public Command
	{
	private:
		std::filesystem::path m_folderPath;

	public:
		explicit CreateFolderCommand(const std::filesystem::path& folderPath);

		void Execute() override;
		void Undo() override;
		std::string GetDescription() const override;
	};

	class CopyFileCommand : public Command
	{
	private:
		std::vector<std::filesystem::path> m_sourcePaths;
		std::vector<std::filesystem::path> m_destPaths;

	public:
		CopyFileCommand(const std::vector<std::filesystem::path>& sourcePaths, const std::filesystem::path& destinationFolder);

		void Execute() override;
		void Undo() override;
		std::string GetDescription() const override;
	};

	class MoveFileCommand : public Command
	{
	private:
		std::vector<std::filesystem::path> m_sourcePaths;
		std::vector<std::filesystem::path> m_destPaths;

	public:
		MoveFileCommand(const std::vector<std::filesystem::path>& sourcePaths, const std::filesystem::path& destinationFolder);

		void Execute() override;
		void Undo() override;
		std::string GetDescription() const override;
	};

	class ImportFileCommand : public Command
	{
	private:
		std::vector<std::filesystem::path> m_sourcePaths;
		std::vector<std::filesystem::path> m_destPaths;

	public:
		ImportFileCommand(const std::vector<std::filesystem::path>& sourcePaths, const std::filesystem::path& destinationFolder);

		void Execute() override;
		void Undo() override;
		std::string GetDescription() const override;
	};
}

#endif // EDITOR_COMMAND_FILECOMMANDS_H
