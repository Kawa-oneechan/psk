#pragma once

#include <string>
#include "support/JSON/JSONValue.h"

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
	extern JSONValue* ReadJSON(const Entry& entry);
	//Returns a JSON Document, with optional patches, given by name
	extern JSONValue* ReadJSON(const std::string& path);
	//Returns a list of VFSEntries matching a particular file pattern.
	extern std::vector<Entry> Enumerate(const std::string& path);
	extern void Forget(const std::vector<Entry>& entries);

}
