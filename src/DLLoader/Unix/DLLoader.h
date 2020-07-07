#pragma once

#include <iostream>
#include <string.h>
#include <dlfcn.h>
#include "../IDLLoader.h"

namespace dlloader
{
	template <class T>
	class DLLoader : public IDLLoader<T>
	{

	private:

		void			*_handle;
		std::string		_pathToLib;
		std::string		_allocClassSymbol;
		std::string		_deleteClassSymbol;
		std::string		_getHeaderHashSymbol;

	public:

		DLLoader(std::string const &pathToLib,
			std::string const &pluginTargetName)
			:
			_handle(nullptr), _pathToLib(pathToLib),
			_allocClassSymbol("allocator_"+pluginTargetName), 
			_deleteClassSymbol("deleter_"+pluginTargetName), 
			_getHeaderHashSymbol("get_shared_headers_hash_"+pluginTargetName)
		{
		}

		~DLLoader() = default;

		bool DLOpenLib() override
		{
			if (!(_handle = dlopen(_pathToLib.c_str(), RTLD_LAZY | RTLD_GLOBAL))) {
				std::cout << dlerror() << std::endl;
				return false;
			}

			// check header hash
			using getHeaderHash = void (*)(char *);
			auto headerHashFunc = reinterpret_cast<getHeaderHash>(
					dlsym(_handle, _getHeaderHashSymbol.c_str()));
			if(!headerHashFunc) {
				std::cout << dlerror() << std::endl;
				DLCloseLib();
				return false;
			} else {
				char plugin_hash[33];
				headerHashFunc(plugin_hash);
				if(strcmp(plugin_hash, GF_SHARED_HEADERS_HASH)!=0) {
					std::cout << "Plugin header hash incompatible!\n";
					DLCloseLib();
					return false;
				}
			}
			return true;
		}

		std::shared_ptr<T> DLGetInstance() override
		{
			using allocClass = T *(*)();
			using deleteClass = void (*)(T *);


			auto allocFunc = reinterpret_cast<allocClass>(
				dlsym(_handle, _allocClassSymbol.c_str()));
			if(!allocFunc)
				std::cout << dlerror() << std::endl;
			auto deleteFunc = reinterpret_cast<deleteClass>(
				dlsym(_handle, _deleteClassSymbol.c_str()));
			if(!deleteFunc)
				std::cout << dlerror() << std::endl;

			if (!allocFunc || !deleteFunc) {
				DLCloseLib();
			}

			return std::shared_ptr<T>(
					allocFunc(),
					[deleteFunc](T *p){ deleteFunc(p); });
		}

		void DLCloseLib() override
		{
			if (dlclose(_handle) != 0) {
				std::cout << dlerror() << std::endl;
			}
		}

	};
}
