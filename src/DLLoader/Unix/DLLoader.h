// This file is part of Geoflow
// Copyright (C) 2018-2022 Ravi Peters

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

		bool DLOpenLib(bool verbose) override
		{
			if (!(_handle = dlopen(_pathToLib.c_str(), RTLD_LAZY | RTLD_GLOBAL))) {
				if (verbose) std::cout << "Can't open and load " << _pathToLib << std::endl;
				if (verbose) std::cout << "->" << dlerror() << std::endl;
				return false;
			}

			// check header hash
			using getHeaderHash = void (*)(char *);
			auto headerHashFunc = reinterpret_cast<getHeaderHash>(
					dlsym(_handle, _getHeaderHashSymbol.c_str()));
			if(!headerHashFunc) {
				if (verbose) std::cout << "Can't open and load " << _pathToLib << std::endl;
				if (verbose) std::cout << "->" << dlerror() << std::endl;
				DLCloseLib(verbose);
				return false;
			} else {
				char plugin_hash[33];
				headerHashFunc(plugin_hash);
				if(strcmp(plugin_hash, GF_SHARED_HEADERS_HASH)!=0) {
					if (verbose) std::cout << "Can't open and load " << _pathToLib << std::endl;
					if (verbose) std::cout << "-> Plugin header hash incompatible!\n";
					// std::cout << plugin_hash << ", geof: " << GF_SHARED_HEADERS_HASH << "\n";
					DLCloseLib(verbose);
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
				DLCloseLib(true);
			}

			return std::shared_ptr<T>(
					allocFunc(),
					[deleteFunc](T *p){ deleteFunc(p); });
		}

		void DLCloseLib(bool verbose) override
		{
			if (dlclose(_handle) != 0) {
				if (verbose) std::cout << dlerror() << std::endl;
			}
		}

	};
}
