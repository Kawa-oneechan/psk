#pragma once
#include <memory>
#include <string>
#include "JsonUtils.h"

namespace VFS
{
	struct Entry
	{
		std::string path; //Relative path of the file.
		int zipIndex; //If the file is from an archive, the file's offset inside that archive.
		int sourceIndex; //Which Source the file is from.
	};

	struct Source
	{
		std::string path;
		bool isZip;
		std::string id;
		std::string friendlyName;
		std::string author;
		int priority;
		std::vector<std::string> namespaces;
		std::vector<std::string> dependencies;

		inline bool operator== (const struct Source& r) { return this->id == r.id; }
	};

	//Initializes the Virtual File System.
	extern void Initialize();
	//Returns the contents of a file given by a predetermined Entry, as a string.
	extern std::string ReadString(const Entry& entry);
	//Returns the contents of a file given by name, as a string.
	extern std::string ReadString(const std::string& path);
	//Returns the contents of a file given by a predetermined Entry.
	extern std::unique_ptr<char[]> ReadData(const Entry& entry, size_t* size);
	//Returns the contents of a file given by name.
	extern std::unique_ptr<char[]> ReadData(const std::string& path, size_t* size);
	//Returns a JSON Document, with optional patches, given by a predetermined Entry.
	extern jsonValue ReadJSON(const Entry& entry);
	//Returns a JSON Document, with optional patches, given by name
	extern jsonValue ReadJSON(const std::string& path);
	//Returns a list of VFSEntries matching a particular file pattern.
	extern std::vector<Entry> Enumerate(const std::string& path);
	extern void Forget(const std::vector<Entry>& entries);

	extern std::unique_ptr<char[]> ReadSaveData(const std::string& archive, const std::string& path, size_t* size);
	extern std::string ReadSaveString(const std::string& archive, const std::string& path);
	extern jsonValue ReadSaveJSON(const std::string& archive, const std::string& path);
	extern bool WriteSaveData(const std::string& archive, const std::string& path, char data[], size_t size);
	extern bool WriteSaveString(const std::string& archive, const std::string& path, const std::string& data);
	extern bool WriteSaveJSON(const std::string& archive, const std::string& path, const jsonValue& data);

	//Returns the contents of a file from the saved game folder.
	extern std::unique_ptr<char[]> ReadSaveData(const std::string& path, size_t* size);
	//Returns the contents of a file from the saved game folder.
	extern size_t ReadSaveData(void* ret, const std::string& path);
	//Returns the contents of a file from the saved game folder, as a string.
	extern std::string ReadSaveString(const std::string& path);
	//Returns the contents of a file from the saved game folder, as a JSON Document.
	extern jsonValue ReadSaveJSON(const std::string& path);
	//Writes data to a file in the saved game folder.
	extern bool WriteSaveData(const std::string& path, const void* data, size_t size);
	//Writes a string to a file in the saved game folder.
	extern bool WriteSaveString(const std::string& path, const std::string& data);
	//Writes a JSON Document to a file in the saved game folder.
	extern bool WriteSaveJSON(const std::string& path, const jsonValue& data);

	//Creates a folder in the saved game folder. Non-existent parts inbetween are handled.
	extern void MakeSaveDir(const std::string& path);

	//Given "foo/bar/baz.txt", returns "foo/bar".
	extern std::string GetPathPart(const std::string& path);
	//Given "foo/bar/baz.txt", returns "baz.txt". Given "baz.txt", returns the empty string.
	extern std::string GetFilePart(const std::string& path);
	//Given "foo.txt", returns "txt".
	extern std::string GetExtension(const std::string& path);
	//Given "foo/bar/baz.txt" and "doc", returns "foo/bar/baz.doc".
	extern std::string ChangeExtension(const std::string& path, const std::string& ext);
	//Given "a/b/c", returns "a/b". Given "a", returns the empty string.
	extern std::string GoUpPath(const std::string& path);
	//Given "foo/bar/baz.txt" and "fallback.txt", finds the highest existing "baz.txt" or "fallback.txt", or the empty string.
	extern std::string ClimbDown(const std::string& path, const std::string& fallback);
}
