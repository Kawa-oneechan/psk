#pragma once

#include <string>

#include "support/JSON/JSON.h"

typedef struct
{
	std::string path; //Relative path of the file.
	int zipIndex; //If the file is from an archive, the file's offset inside that archive.
	int sourceIndex; //Which VFSSource the file is from.
} VFSEntry;

//Initializes the Virtual File System.
extern void InitVFS();
//Returns the contents of a file given by a predetermined VFSEntry.
extern char* ReadVFS(const VFSEntry& entry, size_t* size);
//Returns the contents of a file given by name.
extern char* ReadVFS(const std::string& path, size_t* size);
//Returns a JSON Document, with optional patches, given by a predetermined VFSEntry.
extern JSONValue* ReadJSON(const VFSEntry& entry);
//Returns a JSON Document, with optional patches, given by name
extern JSONValue* ReadJSON(const std::string& path);
//Returns a list of VFSEntries matching a particular file pattern.
extern std::vector<VFSEntry> EnumerateVFS(const std::string& path);
