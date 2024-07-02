#include <vector>
#include <filesystem>
#include <fstream>
#include <regex>
#include <algorithm>

#include "SpecialK.h"
#include "support/miniz.h"
#include "Console.h"

namespace fs = std::experimental::filesystem;

static std::vector<VFSEntry> entries;
static std::vector<VFSSource> sources;

static void initVFS_addEntry(VFSEntry& entry)
{
	if (entry.path.substr(entry.path.length() - 6, 6) == ".patch")
	{
		//Do NOT bother replacing, we want to patch ALL these fuckers in.
		//fmt::print("DEBUG: NOT replacing {}.\n", entry.path);
		entries.push_back(entry);
		return;
	}

	bool replaced = false;
	for (int i = 0; i < entries.size(); i++)
	{
		if (entries[i].path == entry.path)
		{
			entries[i] = entry;
			replaced = true;
			//fmt::print("DEBUG: replacing {}.\n", entry.path);
			break;
		}
	}
	if (!replaced)
		entries.push_back(entry);
}

static void initVFS_fromFolder(int source)
{
	VFSSource& src = sources[source];
	conprint(0, "VFS: from folder {}...", src.path);
	for (const auto& file : fs::recursive_directory_iterator(src.path))
	{
		if (fs::is_directory(file.status()))
			continue;
		if (file.path().filename() == "manifest.json")
			continue;

		auto storedPath = file.path().string();
		storedPath.erase(0, src.path.size() + 1);

		for (int i = 0; i < storedPath.size(); i++)
			if (storedPath[i] == '\\')
				storedPath[i] = '/';

		VFSEntry entry;
		entry.path = std::move(storedPath);
		entry.zipIndex = -1;
		entry.sourceIndex = source;

		initVFS_addEntry(entry);
	}

}

static void initVFS_fromArchive(int source)
{
	VFSSource& src = sources[source];
	conprint(0, "VFS: from {}...", src.path);
	{
		mz_zip_archive zip;
		std::memset(&zip, 0, sizeof(zip));
		mz_zip_reader_init_file(&zip, src.path.c_str(), 0);
		int zipFiles = mz_zip_reader_get_num_files(&zip);
		for (int i = 0; i < zipFiles; i++)
		{
			mz_zip_archive_file_stat fs;
			if (!mz_zip_reader_file_stat(&zip, i, &fs))
				break;
			if (fs.m_is_directory)
				continue;
			if (!_strcmpi(fs.m_filename, "manifest.json"))
				continue;

			VFSEntry entry;
			entry.path = fs.m_filename;
			entry.zipIndex = i;
			entry.sourceIndex = source;
			
			initVFS_addEntry(entry);
		}
	}
}

static void initVFS_addSource(const fs::path& path)
{
	auto newSrc = VFSSource();
	newSrc.path = path.string();
	newSrc.isZip = path.extension() == ".zip";

	std::unique_ptr<char[]> manifestData = nullptr;
	if (newSrc.isZip)
	{
		mz_zip_archive zip;
		std::memset(&zip, 0, sizeof(zip));
		mz_zip_reader_init_file(&zip, path.string().c_str(), 0);
		int zipFiles = mz_zip_reader_get_num_files(&zip);
		for (int i = 0; i < zipFiles; i++)
		{
			mz_zip_archive_file_stat fs;
			if (!mz_zip_reader_file_stat(&zip, i, &fs))
				break;
			if (fs.m_is_directory)
				continue;
			if (!_stricmp(fs.m_filename, "manifest.json"))
			{
				const size_t siz = (size_t)fs.m_uncomp_size;
				manifestData = std::make_unique<char[]>(siz + 2);
				mz_zip_reader_extract_to_mem(&zip, i, manifestData.get(), siz, 0);
				break;
			}
		}
	}
	else
	{
		auto manifestPath = path / "manifest.json";
		if (fs::exists(manifestPath))
		{
			std::ifstream file(manifestPath, std::ios::binary | std::ios::ate);
			std::streamsize fs = file.tellg();
			file.seekg(0, std::ios::beg);
			manifestData = std::make_unique<char[]>(fs + 2);
			file.read(manifestData.get(), fs);
		}
	}

	if (manifestData != nullptr)
	{
		auto manifestDoc = JSON::Parse(manifestData.get())->AsObject();

		newSrc.id = manifestDoc["id"]->AsString();
		newSrc.friendlyName = manifestDoc["friendlyName"]->IsString() ? manifestDoc["friendlyName"]->AsString() : newSrc.id;
		newSrc.author = manifestDoc["author"] != nullptr ? manifestDoc["author"]->AsString() : "Unknown";
		if (manifestDoc["namespaces"] != nullptr)
			for (const auto& s : manifestDoc["namespaces"]->AsArray())
				newSrc.namespaces.push_back(s->AsString());
		if (manifestDoc["dependencies"] != nullptr)
			for (const auto& s : manifestDoc["dependencies"]->AsArray())
				newSrc.dependencies.push_back(s->AsString());
		newSrc.priority = manifestDoc["priority"] != nullptr ? (int)manifestDoc["priority"]->AsNumber() : 1;
	}
	sources.push_back(newSrc);
}

void InitVFS()
{
	conprint(0, "VFS: initializing...");

	initVFS_addSource("data");
	for (const auto& mod : fs::directory_iterator("mods"))
	{
		initVFS_addSource(mod.path());
	}
	
	conprint(0, "Pre-sort:");
	auto table = std::vector<std::string>{ "ID", "Name", "Author", "Priority" };
	for (const auto& source : sources)
	{
		table.push_back(source.id);
		table.push_back(source.friendlyName);
		table.push_back(source.author);
		table.emplace_back(std::to_string(source.priority));
	}
	Table(table, 4);

	std::sort(sources.begin(), sources.end(), [](const VFSSource& a, const VFSSource& b)
	{
		return (a.priority < b.priority);
	});
	

	//Dependencies?
	{
		std::vector<VFSSource> originalSet;
		std::vector<VFSSource> workingSet;

		std::function<void(const VFSSource&)> depWorker;
		depWorker = [&](const VFSSource& source)
		{
			if (std::find(workingSet.begin(), workingSet.end(), source) != workingSet.end())
				throw std::runtime_error(fmt::format("Asset source \"{}\" dependencies form a cycle.", source.id));

			if (std::find(sources.begin(), sources.end(), source) != sources.end())
				return;

			for (const auto& dep : source.dependencies)
			{
				bool found = false;
				for (auto& s : originalSet)
				{
					if (s.id == dep)
					{
						found = true;
						depWorker(s);
					}
				}
				if (!found)
					throw std::runtime_error(fmt::format("Asset source \"{}\" cannot resolve dependency on \"{}\".", source.id, dep));
			}

			auto it = std::find(workingSet.begin(), workingSet.end(), source);
			if (it != workingSet.end())
				workingSet.erase(it);

			sources.emplace_back(source);
		};

		for (const auto& source : sources)
			originalSet.emplace_back(source);
		sources.clear();

		for (const auto& source : originalSet)
			depWorker(source);
	}

	conprint(0, "Post-sort:");
	table = std::vector<std::string>{ "ID", "Name", "Author", "Priority" };
	for (const auto& source : sources)
	{
		table.push_back(source.id);
		table.push_back(source.friendlyName);
		table.push_back(source.author);
		table.emplace_back(std::to_string(source.priority));
	}
	Table(table, 4);

	for (int i = 0; i < sources.size(); i++)
	{
		auto& source = sources[i];
		if (source.isZip)
			initVFS_fromArchive(i);
		else
			initVFS_fromFolder(i);
	}

	conprint(0, "VFS: ended up with {} entries.", entries.size());
}

std::unique_ptr<char[]> ReadVFS(const VFSEntry& entry, size_t* size)
{
	auto& source = sources[entry.sourceIndex];
	if (source.isZip)
	{
		conprint(2, "DEBUG: getting {} from {}.", entry.path, source.path);
		mz_zip_archive zip;
		std::memset(&zip, 0, sizeof(zip));
		mz_zip_reader_init_file(&zip, source.path.c_str(), 0);
		mz_zip_archive_file_stat fs;
		if (!mz_zip_reader_file_stat(&zip, entry.zipIndex, &fs))
		{
			conprint(1, "ReadVFS: couldn't read {}?", entry.path);
			return nullptr;
		}
		const size_t siz = (size_t)fs.m_uncomp_size;
		if (size != nullptr)
			*size = siz;
		auto ret = std::make_unique<char[]>(siz + 2);
		mz_zip_reader_extract_to_mem(&zip, entry.zipIndex, ret.get(), siz, 0);
		return ret; //return std::make_unique<char*>(ret);
	}
	else
	{
		std::string path = (fs::path(source.path) / entry.path).string();
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		std::streamsize fs = file.tellg();
		file.seekg(0, std::ios::beg);
		if (size != nullptr)
			*size = fs;
		auto ret = std::make_unique<char[]>(fs + 2);
		file.read(ret.get(), fs);
		return ret; //return std::make_unique<char*>(ret);
	}
	return nullptr;
}

std::unique_ptr<char[]> ReadVFS(const std::string& path, size_t* size)
{
	auto it = std::find_if(entries.cbegin(), entries.cend(), [path](VFSEntry e)
	{
		return e.path == path;
	});
	if (it == entries.cend())
		return nullptr;
	return ReadVFS(*it, size);
}

const std::string ReadVFSString(const VFSEntry& entry)
{
	return std::string(ReadVFS(entry, nullptr).get());
}

const std::string ReadVFSString(const std::string& path)
{
	auto it = std::find_if(entries.cbegin(), entries.cend(), [path](VFSEntry e)
	{
		return e.path == path;
	});
	if (it == entries.cend())
		return nullptr;
	return ReadVFSString(*it);
}

namespace JSONPatch
{
	extern JSONValue* ApplyPatch(JSONValue& source, JSONValue& patch);
}

JSONValue* ReadJSON(const VFSEntry& entry)
{
	try
	{
		auto vfsData = ReadVFSString(entry.path);
		auto doc = JSON::Parse(vfsData.c_str());

		std::string ppath = entry.path + ".patch";
		for (const auto& pents : entries)
		{
			if (pents.path == ppath)
			{
				auto pdata = ReadVFSString(pents.path);
				auto pdoc = JSON::Parse(pdata.c_str());
				auto patched = JSONPatch::ApplyPatch(*doc, *pdoc);
				doc = patched;
			}
		}

		return doc;
	}
	catch (std::exception& e)
	{
		FatalError(e.what());
	}
}

JSONValue* ReadJSON(const std::string& path)
{
	auto it = std::find_if(entries.cbegin(), entries.cend(), [path](VFSEntry e)
	{
		return e.path == path;
	});
	if (it == entries.cend())
		return nullptr;
	return ReadJSON(*it);
}

std::vector<VFSEntry> EnumerateVFS(const std::string& path)
{
	std::vector<VFSEntry> r;

	//Turn "species\\*.json" into "species\\\\(.*?)\.json"
	std::string pathAsPattern;
	for (const auto& ch : path)
	{
		switch (ch)
		{
		case '\\': pathAsPattern += '/'; break;
		case '*': pathAsPattern += "(.*?)"; break;
		default: pathAsPattern += ch; break;
		}
	}
	pathAsPattern += '$';

	std::regex pattern(pathAsPattern, std::regex_constants::icase);

	for (const auto& entry : entries)
	{
		std::string ep(entry.path);
		if (ep.length() < path.length())
			continue;
		if (std::regex_search(ep, pattern))
			r.push_back(entry);
	}
	return r;
}

void ForgetVFS(const std::vector<VFSEntry>& forget)
{
	auto start = entries.size();

	for (const auto& f : forget)
	{
		auto e = entries.begin();
		while (e != entries.end())
		{
			if (f.path == e->path)
			{
				entries.erase(e);
				break;
			}
			++e;
		}
	}

	conprint(0, "ForgetVFS: went from {} to {} items, forgetting {}.", start, entries.size(), start - entries.size());
}
