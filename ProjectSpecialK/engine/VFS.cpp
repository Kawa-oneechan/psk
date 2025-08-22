#include <vector>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <functional>

#include <miniz.h>
#include <format.h>
#include "VFS.h"
#include "TextUtils.h"
#include "Console.h"
#include "Texture.h"
#include "JsonUtils.h"
#include "../Game.h"

/*
Saving to an archive file breaks if the entry already exists.
The new plan:
1. Open the original file, if any, and a new temporary file.
2. Iterate through its contents.
3. If it's the entry we want to save, skip it. Else, copy it as-is to the new file.
4. Add the new entry.
5. Delete the old, rename the new.
6. This may be rather inefficient when handling a town full of villagers in a row.
Until #6 is reconsidered, use the villagers FOLDER instead.
*/

#ifdef _MSC_VER
namespace fs = std::experimental::filesystem;
#else
namespace fs = std::filesystem;
#endif

__declspec(noreturn)
extern void FatalError(const std::string& message);

namespace UI
{
	extern jsonValue json;
	extern jsonValue settings;
	extern std::string initFile;
};

namespace JSONPatch
{
	extern jsonValue& ApplyPatch(jsonValue& source, jsonValue& patch);
}

#ifdef BECKETT_EXTRASAVEDIRS
extern void GamePrepSaveDirs(const fs::path& savePath);
#endif

#ifdef _WIN32
extern "C" {
	using GUID = struct _GUID
	{
		unsigned long  Data1;
		unsigned short Data2;
		unsigned short Data3;
		unsigned char  Data4[8];
	};
	const GUID FOLDERID_SavedGames = { 0x4C5C32FF, 0xBB9D, 0x43B0, 0xB5, 0xB4, 0x2D, 0x72, 0xE5, 0x4E, 0xAA, 0xA4 };
	const GUID FOLDERID_Documents = { 0xFDD39AD0, 0x238F, 0x46AF, 0xAD, 0xB4, 0x6C, 0x85, 0x48, 0x03, 0x69, 0xC7 };
	const GUID FOLDERID_Roaming = { 0x3EB685DB, 0x65F9, 0x4CF6, 0xA0, 0x3A, 0xE3, 0xEF, 0x65, 0x72, 0x9F, 0x3D };
	int __stdcall SHGetKnownFolderPath(_In_ const GUID* rfid, _In_ unsigned long dwFlags, _In_opt_ void* hToken, _Out_ wchar_t** ppszPath);
	int __stdcall WideCharToMultiByte(_In_ unsigned int CodePage, _In_ unsigned long dwFlags, const wchar_t* lpWideCharStr, _In_ int cchWideChar, char* lpMultiByteStr, _In_ int cbMultiByte, _In_opt_ char* lpDefaultChar, _Out_opt_ bool* lpUsedDefaultChar);
	void _stdcall CoTaskMemFree(_In_opt_ void* pv);
}
#endif

namespace VFS
{
	static std::vector<Entry> entries;
	static std::vector<Source> sources;

	static fs::path savePath;

	static jsonValue initJSON;

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
			mz_zip_archive zip{ 0 };
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
			mz_zip_reader_end(&zip);
		}
	}

	static void addSource(const fs::path& path)
	{
		if (!fs::exists(path))
		{
			conprint(2, "VFS: can't find asset source {}. Skipping.", path);
			return;
		}

		auto newSrc = Source();
		newSrc.path = path.string();
		newSrc.isZip = path.extension() == ".zip";

		std::unique_ptr<char[]> manifestData = nullptr;
		if (newSrc.isZip)
		{
			mz_zip_archive zip{ 0 };
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
			mz_zip_reader_end(&zip);
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
			auto manifestDoc = json5pp::parse5(manifestData.get());

			newSrc.id = manifestDoc["id"].as_string();
			newSrc.friendlyName = manifestDoc["friendlyName"].is_string() ? manifestDoc["friendlyName"].as_string() : newSrc.id;
			newSrc.author = !manifestDoc["author"].is_string() ? manifestDoc["author"].as_string() : "Unknown";
			if (manifestDoc["namespaces"].is_array())
				for (const auto& s : manifestDoc["namespaces"].as_array())
					newSrc.namespaces.push_back(s.as_string());
			if (manifestDoc["dependencies"].is_array())
				for (const auto& s : manifestDoc["dependencies"].as_array())
					newSrc.dependencies.push_back(s.as_string());
			newSrc.priority = manifestDoc["priority"].is_integer() ? manifestDoc["priority"].as_integer() : 1;
			sources.push_back(newSrc);
		}
		else
			conprint(0, "No manifest for {}, so mod is skipped.", newSrc.path);
	}

	static void findSaveDir()
	{
		auto initDoc = initJSON.as_object();
		if (initDoc["saves"].as_object().size() == 0)
			FatalError("No savegame paths listed.");
		auto saves = initDoc["saves"].as_object();

#ifdef _WIN32
		//Yes, we're assuming this is Vista or better. Fuck you.

		auto p = saves["win32"].as_string();

		wchar_t* wp;
		char mp[1024] = { 0 };

		auto handle = [&](const std::string& macro, const GUID* guid)
		{
			if (p.find(macro) != std::string::npos)
			{
				SHGetKnownFolderPath(guid, 0, nullptr, &wp);
				WideCharToMultiByte(65001, 0, wp, (int)wcslen(wp), mp, 1024, nullptr, nullptr);
				CoTaskMemFree(wp);
				ReplaceAll(p, macro, std::string(mp));
			}
		};

		handle("$(SavedGames)", &FOLDERID_SavedGames);
		handle("$(Documents)", &FOLDERID_Documents);
		handle("$(AppData)", &FOLDERID_Roaming);

#else
		auto p = saves["linux"]->AsString();
		//Find some interesting $() macros later on maybe.
#endif

		savePath = fs::path(p);

		fs::create_directory(savePath);
#ifdef BECKETT_EXTRASAVEDIRS
		GamePrepSaveDirs(savePath);
#endif
	}

	void Initialize()
	{
		conprint(0, "VFS: initializing...");

		if (!fs::exists(UI::initFile))
			FatalError(fmt::format("Could not load init file {}.", UI::initFile));

		std::ifstream file(UI::initFile, std::ios::binary | std::ios::ate);
		std::streamsize fsize = file.tellg();
		file.seekg(0, std::ios::beg);
		auto initData = std::make_unique<char[]>(fsize + 2);
		file.read(initData.get(), fsize);

		initJSON = json5pp::parse5(initData.get());
		if (!initJSON)
			FatalError("Init file parses as blank.");

		auto initDoc = initJSON.as_object();
		if (initDoc["sources"].as_array().size() == 0)
			FatalError("No asset sources listed.");
		for (const auto& source : initDoc["sources"].as_array())
		{
			for (const auto& mod : fs::directory_iterator(source.as_string()))
			{
				auto path = mod.path();
				auto fn = path.filename();
				if (fn.string()[0] == '.')
					continue;
				if (path.extension() == ".zip" || fs::is_directory(path))
					addSource(path);
			}
		}

		if (sources.size() == 0)
			FatalError("No asset sources found.");

		auto table = std::vector<std::string>{ "ID", "Name", "Author", "Priority" };
#ifdef DEBUG
		conprint(0, "Pre-sort:");
		for (const auto& source : sources)
		{
			table.push_back(source.id);
			table.push_back(source.friendlyName);
			table.push_back(source.author);
			table.emplace_back(std::to_string(source.priority));
		}
		Table(table, 4);
#endif

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

		debprint(0, "Post-sort:");
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

		std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b)
		{
			return a.path.compare(b.path) < 0;
		});

		conprint(0, "VFS: ended up with {} entries.", entries.size());

		findSaveDir();
	}

	std::unique_ptr<char[]> ReadData(const Entry& entry, size_t* size)
	{
		auto& source = sources[entry.sourceIndex];
		if (source.isZip)
		{
			mz_zip_archive zip{ 0 };
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
			mz_zip_reader_end(&zip);
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
		//return nullptr;
	}

	std::unique_ptr<char[]> ReadData(const std::string& path, size_t* size)
	{
		auto absPath = ResolvePath(path);
		auto it = std::find_if(entries.cbegin(), entries.cend(), [absPath](Entry e)
		{
			return e.path == absPath;
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
		auto absPath = ResolvePath(path);
		auto it = std::find_if(entries.cbegin(), entries.cend(), [absPath](Entry e)
		{
			return e.path == absPath;
		});
		if (it == entries.cend())
			return nullptr;
		return ReadString(*it);
	}

	jsonValue ReadJSON(const Entry& entry)
	{
		try
		{
			auto vfsData = ReadString(entry.path);
			auto doc = json5pp::parse5(vfsData);

			std::string ppath = entry.path + ".patch";
			for (const auto& pents : entries)
			{
				if (pents.path == ppath)
				{
					auto pdata = ReadString(pents.path);
					auto pdoc = json5pp::parse5(pdata);
					auto patched = JSONPatch::ApplyPatch(doc, pdoc);
					doc = std::move(patched);
				}
			}

			return doc;
		}
		catch (std::exception& e)
		{
			FatalError(e.what());
		}
	}

	jsonValue ReadJSON(const std::string& path)
	{
		auto absPath = ResolvePath(path);
		auto it = std::find_if(entries.cbegin(), entries.cend(), [absPath](Entry e)
		{
			return e.path == absPath;
		});
		if (it == entries.cend())
			return jsonValue(nullptr);
		return ReadJSON(*it);
	}

	std::vector<Entry> Enumerate(const std::string& path)
	{
		std::vector<Entry> r;
		std::string p = path;
		ReplaceAll(p, "\\", "/");
		p = ResolvePath(p);

		const auto splatp = p.find('*');

		if (splatp == p.npos)
		{
			for (const auto& entry : entries)
			{
				if (entry.path == p)
				{
					r.push_back(entry);
					break;
				}
			}
			return r;
		}

		const auto prefix = p.substr(0, splatp);
		const auto suffix = p.substr(splatp + 1);
		const auto prelen = prefix.length();
		const auto suflen = suffix.length();
		const auto plen = p.length();
		auto anything = false;
		for (const auto& entry : entries)
		{
			if (entry.path[0] != p[0])
			{
				if (anything)
					return r;
				continue;
			}

			const auto ep = entry.path;
			const auto eplen = ep.length();
			if (eplen < plen || ep.substr(0, prelen) != prefix)
			{
				if (anything)
					return r;
				continue;
			}
			if (ep.substr(eplen - suflen) != suffix)
				continue;

			r.push_back(entry);
			anything = true;
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

		entries.shrink_to_fit();

		debprint(0, "ForgetVFS: went from {} to {} items, forgetting {}.", start, entries.size(), start - entries.size());
	}

	std::string mangle(const std::string& path)
	{
		std::string ret = path;
		ReplaceAll(ret, "/", "\\");
		ReplaceAll(ret, ":", "_");
		return ret;
	}

	std::unique_ptr<char[]> ReadSaveData(const std::string& archive, const std::string& path, size_t* size)
	{
		std::string p2 = mangle(path);

		auto p = savePath / archive;
		auto p3 = p.generic_string();

		mz_zip_archive zip{ 0 };
		mz_zip_reader_init_file(&zip, p3.c_str(), 0);
		if (zip.m_zip_type == MZ_ZIP_TYPE_INVALID)
			return nullptr;
		int zipFiles = mz_zip_reader_get_num_files(&zip);
		mz_zip_archive_file_stat zfs = { 0 };
		for (int i = 0; i < zipFiles; i++)
		{
			if (!mz_zip_reader_file_stat(&zip, i, &zfs))
				break;
			if (zfs.m_is_directory)
				continue;
			if (!_strcmpi(zfs.m_filename, p2.c_str()))
				continue;
		}

		const size_t fsize = (size_t)zfs.m_uncomp_size;
		if (size != nullptr)
			*size = fsize;
		auto ret = std::make_unique<char[]>(fsize + 2);
		mz_zip_reader_extract_to_mem(&zip, zfs.m_file_index, ret.get(), fsize, 0);
		mz_zip_reader_end(&zip);
		return ret;
	}

	std::string ReadSaveString(const std::string& archive, const std::string& path)
	{
		auto data = ReadSaveData(archive, path, nullptr);
		if (!data)
			return "";
		return std::string(data.get());
	}

	jsonValue ReadSaveJSON(const std::string& archive, const std::string& path)
	{
		auto data = ReadSaveString(archive, path);
		if (data.empty())
			throw std::runtime_error("Blank JSON"); //return nullptr;
		auto doc = json5pp::parse5(data);
		return doc;
	}

	bool WriteSaveData(const std::string& archive, const std::string& path, char data[], size_t size)
	{
		std::string p2 = mangle(path);

		auto p = savePath / archive;
		auto p3 = p.generic_string();
		auto ret = mz_zip_add_mem_to_archive_file_in_place(p2.c_str(), p2.c_str(), data, size, nullptr, 0, MZ_BEST_COMPRESSION);
		if (!ret)
			throw std::exception("Couldn't save to archive.");
		return true;
	}

	bool WriteSaveString(const std::string& archive, const std::string& path, const std::string& data)
	{
		std::string p2 = mangle(path);

		auto p = savePath / archive;
		auto p3 = p.generic_string();
		auto ret = mz_zip_add_mem_to_archive_file_in_place(p3.c_str(), p2.c_str(), data.c_str(), data.length(), nullptr, 0, MZ_BEST_COMPRESSION);
		if (!ret)
			throw std::exception("Couldn't save to archive.");
		//TODO: make this stronger.
		return true;

	}

	bool WriteSaveJSON(const std::string& archive, const std::string& path, jsonValue& data)
	{
		return WriteSaveString(archive, path, data.stringify5(json5pp::rule::tab_indent<>()));
	}

	std::unique_ptr<char[]> ReadSaveData(const std::string& path, size_t* size)
	{
		std::string p2 = mangle(path);

		std::ifstream file(savePath / p2, std::ios::binary | std::ios::ate);
		if (!file.good())
			throw std::runtime_error("Couldn't open file.");
		std::streamsize fs = file.tellg();
		file.seekg(0, std::ios::beg);
		if (size != nullptr)
			*size = fs;
		auto ret = std::make_unique<char[]>(fs + 2);
		file.read(ret.get(), fs);
		return ret;
	}

	size_t ReadSaveData(void* ret, const std::string& path)
	{
		std::string p2 = mangle(path);

		std::ifstream file(savePath / p2, std::ios::binary | std::ios::ate);
		if (!file.good())
			throw std::runtime_error("Couldn't open file.");
		std::streamsize fs = file.tellg();
		file.seekg(0, std::ios::beg);
		file.read((char*)ret, fs);
		return fs;
	}

	std::string ReadSaveString(const std::string& path)
	{
		return std::string(ReadSaveData(path, nullptr).get());
	}

	jsonValue ReadSaveJSON(const std::string& path)
	{
		auto data = ReadSaveString(path);
		auto doc = json5pp::parse5(data);
		return doc;
	}

	bool WriteSaveData(const std::string& path, void* data, size_t size)
	{
		std::string p2 = mangle(path);

		std::ofstream file(savePath / p2, std::ios::trunc | std::ios::binary);
		if (!file.good())
			throw std::runtime_error("Couldn't open file.");
		file.write((char*)data, size);
		file.close();
		return true;
	}

	bool WriteSaveString(const std::string& path, const std::string& data)
	{
		return WriteSaveData(path, (void*)data.c_str(), data.length());
	}

	bool WriteSaveJSON(const std::string& path, jsonValue& data)
	{
		return WriteSaveString(path, data.stringify5(json5pp::rule::tab_indent<>()));
	}

	std::string GetPathPart(const std::string& path)
	{
		if (path.empty()) return path;
		if (path.find('/') != std::string::npos)
			return path.substr(0, path.find_last_of('/'));
		return "";
	}

	std::string GetFilePart(const std::string& path)
	{
		if (path.empty()) return path;
		if (path.find('/') != std::string::npos)
			return path.substr(path.find_last_of('/') + 1);
		return "";
	}

	std::string ChangeExtension(const std::string& path, const std::string& ext)
	{
		if (path.empty()) return path;
		if (path.find('.') != std::string::npos)
			return path.substr(0, path.find_last_of('.') + 1) + ext;
		return path;
	}

	std::string GoUpPath(const std::string& path)
	{
		if (path.empty()) return path;
		if (path.find('/') != std::string::npos)
			return path.substr(0, path.rfind('/'));
		return "";
	}


	std::string ClimbDown(const std::string& path, const std::string& fallback)
	{
		auto haveFile = [path](const std::string& p)
		{
			auto it = std::find_if(entries.cbegin(), entries.cend(), [p](Entry e)
			{
				return e.path == p;
			});
			return it != entries.cend();
		};

		auto dir = GetPathPart(path);
		auto file = GetFilePart(path);
		while (!dir.empty())
		{
			auto findThis = fmt::format("{}/{}", dir, file);
			if (haveFile(findThis))
				return findThis;
			findThis = fmt::format("{}/{}", dir, fallback);
			if (haveFile(findThis))
				return findThis;

			dir = GoUpPath(dir);
		}

		//Final attempt in the root
		if (haveFile(file))
			return file;

		return fallback;
	}
}
