#include <vector>
#include <filesystem>
#include <fstream>
#include <regex>
#include <algorithm>

#include "SpecialK.h"
#include "support/miniz.h"
#include "Console.h"

namespace JSONPatch
{
	extern JSONValue* ApplyPatch(JSONValue& source, JSONValue& patch);
}

#ifdef _WIN32
extern "C" {
	typedef struct _GUID
	{
		unsigned long  Data1;
		unsigned short Data2;
		unsigned short Data3;
		unsigned char  Data4[8];
	} GUID;
	const GUID FOLDERID_SavedGames = { 0x4c5c32ff, 0xbb9d, 0x43b0, 0xb5, 0xb4, 0x2d, 0x72, 0xe5, 0x4e, 0xaa, 0xa4 };
	int __stdcall SHGetKnownFolderPath(_In_ const GUID* rfid, _In_ unsigned long dwFlags, _In_opt_ void* hToken, _Out_ wchar_t** ppszPath);
	int __stdcall WideCharToMultiByte(_In_ unsigned int CodePage, _In_ unsigned long dwFlags, const wchar_t* lpWideCharStr, _In_ int cchWideChar, char* lpMultiByteStr, _In_ int cbMultiByte, _In_opt_ char* lpDefaultChar, _Out_opt_ bool* lpUsedDefaultChar);
	void _stdcall CoTaskMemFree(_In_opt_ void* pv);
}
#endif

namespace VFS
{
#ifdef _MSC_VER
	namespace fs = std::experimental::filesystem;
#else
	namespace fs = std::filesystem;
#endif

	static std::vector<Entry> entries;
	static std::vector<Source> sources;

	static fs::path savePath;

	static void addFileEntry(Entry& entry)
	{
		if (entry.path.substr(entry.path.length() - 6, 6) == ".patch")
		{
			//Do NOT bother replacing, we want to patch ALL these fuckers in.
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
				break;
			}
		}
		if (!replaced)
			entries.push_back(entry);
	}

	static void addFromFolder(int source)
	{
		Source& src = sources[source];
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

			Entry entry;
			entry.path = std::move(storedPath);
			entry.zipIndex = -1;
			entry.sourceIndex = source;

			addFileEntry(entry);
		}

	}

	static void addFromArchive(int source)
	{
		Source& src = sources[source];
		conprint(0, "VFS: from {}...", src.path);
		{
			mz_zip_archive zip;
			std::memset(&zip, 0, sizeof(zip));
			mz_zip_reader_init_file(&zip, src.path.c_str(), 0);
			int zipFiles = mz_zip_reader_get_num_files(&zip);
			for (int i = 0; i < zipFiles; i++)
			{
				mz_zip_archive_file_stat zfs;
				if (!mz_zip_reader_file_stat(&zip, i, &zfs))
					break;
				if (zfs.m_is_directory)
					continue;
				if (!_strcmpi(zfs.m_filename, "manifest.json"))
					continue;

				Entry entry;
				entry.path = zfs.m_filename;
				entry.zipIndex = i;
				entry.sourceIndex = source;

				addFileEntry(entry);
			}
		}
	}

	static void addSource(const fs::path& path)
	{
		auto newSrc = Source();
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
				mz_zip_archive_file_stat zfs;
				if (!mz_zip_reader_file_stat(&zip, i, &zfs))
					break;
				if (zfs.m_is_directory)
					continue;
				if (!_stricmp(zfs.m_filename, "manifest.json"))
				{
					const size_t siz = (size_t)zfs.m_uncomp_size;
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
				std::streamsize fsize = file.tellg();
				file.seekg(0, std::ios::beg);
				manifestData = std::make_unique<char[]>(fsize + 2);
				file.read(manifestData.get(), fsize);
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
			sources.push_back(newSrc);
		}
		else
			conprint(0, "No manifest for {}, so mod is skipped.", newSrc.path);
	}

	static void findSaveDir()
	{
#ifdef _WIN32
		//Yes, we're assuming this is Vista or better. Fuck you.
		wchar_t* wp;
		char mp[1024] = { 0 };
		SHGetKnownFolderPath(&FOLDERID_SavedGames, 0, nullptr, &wp);
		WideCharToMultiByte(65001, 0, wp, (int)wcslen(wp), mp, 1024, nullptr, nullptr);
		CoTaskMemFree(wp);
		savePath = fs::path(std::string(mp)) / "Project Special K";
#else
		//TODO: find a good save path for non-Windows systems.
#endif
		fs::create_directory(savePath);
	}

	void Initialize()
	{
		conprint(0, "VFS: initializing...");

		addSource("data");
		for (const auto& mod : fs::directory_iterator("mods"))
		{
			addSource(mod.path());
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

		std::sort(sources.begin(), sources.end(), [](const Source& a, const Source& b)
		{
			return (a.priority < b.priority);
		});


		//Resolve dependencies
		{
			std::vector<Source> originalSet;
			std::vector<Source> workingSet;

			std::function<void(const Source&)> depWorker;
			depWorker = [&](const Source& source)
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
				addFromArchive(i);
			else
				addFromFolder(i);
		}

		conprint(0, "VFS: ended up with {} entries.", entries.size());

		findSaveDir();
	}

	std::unique_ptr<char[]> ReadData(const Entry& entry, size_t* size)
	{
		auto& source = sources[entry.sourceIndex];
		if (source.isZip)
		{
			conprint(2, "DEBUG: getting {} from {}.", entry.path, source.path);
			mz_zip_archive zip;
			std::memset(&zip, 0, sizeof(zip));
			mz_zip_reader_init_file(&zip, source.path.c_str(), 0);
			mz_zip_archive_file_stat zfs;
			if (!mz_zip_reader_file_stat(&zip, entry.zipIndex, &zfs))
			{
				conprint(1, "ReadVFS: couldn't read {}?", entry.path);
				return nullptr;
			}
			const size_t fsize = (size_t)zfs.m_uncomp_size;
			if (size != nullptr)
				*size = fsize;
			auto ret = std::make_unique<char[]>(fsize + 2);
			mz_zip_reader_extract_to_mem(&zip, entry.zipIndex, ret.get(), fsize, 0);
			return ret;
		}
		else
		{
			std::string path = (fs::path(source.path) / entry.path).string();
			std::ifstream file(path, std::ios::binary | std::ios::ate);
			std::streamsize fsize = file.tellg();
			file.seekg(0, std::ios::beg);
			if (size != nullptr)
				*size = fsize;
			auto ret = std::make_unique<char[]>(fsize + 2);
			file.read(ret.get(), fsize);
			return ret;
		}
		return nullptr;
	}

	std::unique_ptr<char[]> ReadData(const std::string& path, size_t* size)
	{
		auto it = std::find_if(entries.cbegin(), entries.cend(), [path](Entry e)
		{
			return e.path == path;
		});
		if (it == entries.cend())
			return nullptr;
		return ReadData(*it, size);
	}

	std::string ReadString(const Entry& entry)
	{
		return std::string(ReadData(entry, nullptr).get());
	}

	std::string ReadString(const std::string& path)
	{
		auto it = std::find_if(entries.cbegin(), entries.cend(), [path](Entry e)
		{
			return e.path == path;
		});
		if (it == entries.cend())
			return nullptr;
		return ReadString(*it);
	}

	JSONValue* ReadJSON(const Entry& entry)
	{
		try
		{
			auto vfsData = ReadString(entry.path);
			auto doc = JSON::Parse(vfsData.c_str());

			std::string ppath = entry.path + ".patch";
			for (const auto& pents : entries)
			{
				if (pents.path == ppath)
				{
					auto pdata = ReadString(pents.path);
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
		auto it = std::find_if(entries.cbegin(), entries.cend(), [path](Entry e)
		{
			return e.path == path;
		});
		if (it == entries.cend())
			return nullptr;
		return ReadJSON(*it);
	}

	std::vector<Entry> Enumerate(const std::string& path)
	{
		std::vector<Entry> r;

		//Turn "species\\*.json" into "species/(.*?)\.json"
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

	void Forget(const std::vector<Entry>& forget)
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

	std::unique_ptr<char[]> ReadSaveData(const std::string& archive, const std::string& path, size_t* size)
	{
		auto p = savePath / archive;
		auto p2 = p.generic_string();

		mz_zip_archive zip;
		std::memset(&zip, 0, sizeof(zip));
		mz_zip_reader_init_file(&zip, p2.c_str(), 0);
		if (zip.m_zip_type == MZ_ZIP_TYPE_INVALID)
			return nullptr;
		int zipFiles = mz_zip_reader_get_num_files(&zip);
		mz_zip_archive_file_stat zfs;
		for (int i = 0; i < zipFiles; i++)
		{
			if (!mz_zip_reader_file_stat(&zip, i, &zfs))
				break;
			if (zfs.m_is_directory)
				continue;
			if (!_strcmpi(zfs.m_filename, path.c_str()))
				continue;
		}

		const size_t fsize = (size_t)zfs.m_uncomp_size;
		if (size != nullptr)
			*size = fsize;
		auto ret = std::make_unique<char[]>(fsize + 2);
		mz_zip_reader_extract_to_mem(&zip, zfs.m_file_index, ret.get(), fsize, 0);

		return ret;
	}

	std::string ReadSaveString(const std::string& archive, const std::string& path)
	{
		auto data = ReadSaveData(archive, path, nullptr);
		if (!data)
			return "";
		return std::string(data.get());
	}

	JSONValue* ReadSaveJSON(const std::string& archive, const std::string& path)
	{
		auto data = ReadSaveString(archive, path);
		if (data.empty())
			throw std::runtime_error("Blank JSON"); //return nullptr;
		auto doc = JSON::Parse(data.c_str());
		return doc;
	}

	bool WriteSaveData(const std::string& archive, const std::string& path, char data[], size_t size)
	{
		auto p = savePath / archive;
		auto p2 = p.generic_string();
		auto ret = mz_zip_add_mem_to_archive_file_in_place(p2.c_str(), path.c_str(), data, size, nullptr, 0, MZ_BEST_COMPRESSION);
		if (!ret)
			throw std::exception("Couldn't save to archive.");
		//TODO: make this stronger.
		return true;
	}

	bool WriteSaveString(const std::string& archive, const std::string& path, const std::string& data)
	{
		auto p = savePath / archive;
		auto p2 = p.generic_string();
		auto ret = mz_zip_add_mem_to_archive_file_in_place(p2.c_str(), path.c_str(), data.c_str(), data.length(), nullptr, 0, MZ_BEST_COMPRESSION);
		if (!ret)
			throw std::exception("Couldn't save to archive.");
		//TODO: make this stronger.
		return true;

	}

	bool WriteSaveJSON(const std::string& archive, const std::string& path, JSONValue* data)
	{
		return WriteSaveString(archive, path, JSON::Stringify(data));
	}

	std::unique_ptr<char[]> ReadSaveData(const std::string& path, size_t* size)
	{
		std::ifstream file(savePath / path, std::ios::binary | std::ios::ate);
		if (!file.good())
			throw std::exception("Couldn't open file.");
		std::streamsize fs = file.tellg();
		file.seekg(0, std::ios::beg);
		if (size != nullptr)
			*size = fs;
		auto ret = std::make_unique<char[]>(fs + 2);
		file.read(ret.get(), fs);
		return ret;
	}

	std::string ReadSaveString(const std::string& path)
	{
		return std::string(ReadSaveData(path, nullptr).get());
	}

	JSONValue* ReadSaveJSON(const std::string& path)
	{
		auto data = ReadSaveString(path);
		auto doc = JSON::Parse(data.c_str());
		return doc;
	}

	bool WriteSaveData(const std::string& path, char data[], size_t size)
	{
		size;
		std::ofstream file(savePath / path, std::ios::trunc | std::ios::binary);
		if (!file.good())
			throw std::exception("Couldn't open file.");
		file << data; //eeugh
		file.close();
		return true;
	}

	bool WriteSaveString(const std::string& path, const std::string& data)
	{
		std::ofstream file(savePath / path, std::ios::trunc | std::ios::binary);
		if (!file.good())
			throw std::exception("Couldn't open file.");
		file << data; //eeugh
		file.close();
		return true;
	}

	bool WriteSaveJSON(const std::string& path, JSONValue* data)
	{
		return WriteSaveString(path, JSON::Stringify(data));
	}
}
