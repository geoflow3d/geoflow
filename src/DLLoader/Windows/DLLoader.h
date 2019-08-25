#pragma once

#include <iostream>
#include "../IDLLoader.h"
#include "Windows.h"

namespace dlloader
{
	template <class T>
	class DLLoader : public IDLLoader<T>
	{

	private:
		HMODULE			_handle;
		std::string		_pathToLib;
		std::string		_allocClassSymbol;
		std::string		_deleteClassSymbol;
		std::string		_getHeaderHashSymbol;

	public:
		DLLoader(std::string const &pathToLib,
			std::string const &allocClassSymbol = "allocator",
			std::string const &deleteClassSymbol = "deleter",
			std::string const &getHeaderHashSymbol = "get_shared_headers_hash")
			:
			_handle(nullptr), _pathToLib(pathToLib),
			_allocClassSymbol(allocClassSymbol), 
			_deleteClassSymbol(deleteClassSymbol),
			_getHeaderHashSymbol(getHeaderHashSymbol)
		{}

		~DLLoader() = default;

		bool DLOpenLib() override
		{
			if (!(_handle = LoadLibrary(_pathToLib.c_str()))) {
				std::cerr << "Can't open and load " << _pathToLib << std::endl;
				return false;
			}

			// check header hash
			using getHeaderHash = void (*)(char *);
			auto headerHashFunc = reinterpret_cast<getHeaderHash>(
					GetProcAddress(_handle, _getHeaderHashSymbol.c_str()));
			if(!headerHashFunc) {
				std::cerr << "Can't find _getHeaderHashSymbol symbol in " << _pathToLib << std::endl;
				DLCloseLib();
				return false;
			} else {
				char plugin_hash[33];
				headerHashFunc(plugin_hash);
				if(strcmp(plugin_hash, GF_SHARED_HEADERS_HASH)!=0) {
					std::cerr << plugin_hash << ", geof: " << GF_SHARED_HEADERS_HASH << "\n";
					std::cerr << "Plugin header hash incompatible!\n";
					DLCloseLib();
					return false;
				}
			}
			return true;
		}

		std::shared_ptr<T> DLGetInstance() override
		{
			using allocClass = T * (*)();
			using deleteClass = void(*)(T *);

			auto allocFunc = reinterpret_cast<allocClass>(
				GetProcAddress(_handle, _allocClassSymbol.c_str()));
			auto deleteFunc = reinterpret_cast<deleteClass>(
				GetProcAddress(_handle, _deleteClassSymbol.c_str()));

			if (!allocFunc || !deleteFunc) {
				DLCloseLib();
				std::cerr << "Can't find allocator or deleter symbol in " << _pathToLib << std::endl;
			}

			return std::shared_ptr<T>(
				allocFunc(),
				[deleteFunc](T *p) { deleteFunc(p); });
		}

		void DLCloseLib() override
		{
			if (FreeLibrary(_handle) == 0) {
				std::cerr << "Can't close " << _pathToLib << std::endl;
			}
		}

	};

}