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
			std::string const &pluginTargetName)
			:
			_handle(nullptr), _pathToLib(pathToLib),
			_allocClassSymbol("allocator_"+pluginTargetName), 
			_deleteClassSymbol("deleter_"+pluginTargetName), 
			_getHeaderHashSymbol("get_shared_headers_hash_"+pluginTargetName)
		{}

		~DLLoader() = default;

    
    //Returns the last Win32 error, in string format. Returns an empty string if there is no error.
    std::string GetLastErrorAsString()
    {
        //Get the error message, if any.
        DWORD errorMessageID = ::GetLastError();
        if(errorMessageID == 0)
            return std::string(); //No error message has been recorded

        LPSTR messageBuffer = nullptr;
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                    NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

        std::string message(messageBuffer, size);

        //Free the buffer.
        LocalFree(messageBuffer);

        return message;
    }

		bool DLOpenLib(bool verbose) override
		{
			if (!(_handle = LoadLibrary(_pathToLib.c_str()))) {
				if (verbose) std::cout << "Can't open and load " << _pathToLib << std::endl;
				if (verbose) std::cout << "-> ERROR: " << GetLastErrorAsString() << std::endl;
				return false;
			}

			// check header hash
			using getHeaderHash = void (*)(char *);
			auto headerHashFunc = reinterpret_cast<getHeaderHash>(
					GetProcAddress(_handle, _getHeaderHashSymbol.c_str()));
			if(!headerHashFunc) {
				if (verbose) std::cout << "Can't open and load " << _pathToLib << std::endl;
				if (verbose) std::cout << "-> Can't find _getHeaderHashSymbol" << std::endl;
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
			using allocClass = T * (*)();
			using deleteClass = void(*)(T *);

			auto allocFunc = reinterpret_cast<allocClass>(
				GetProcAddress(_handle, _allocClassSymbol.c_str()));
			auto deleteFunc = reinterpret_cast<deleteClass>(
				GetProcAddress(_handle, _deleteClassSymbol.c_str()));

			if (!allocFunc || !deleteFunc) {
				DLCloseLib(true);
				std::cout << "Can't find allocator or deleter symbol in " << _pathToLib << std::endl;
			}

			return std::shared_ptr<T>(
				allocFunc(),
				[deleteFunc](T *p) { deleteFunc(p); });
		}

		void DLCloseLib(bool verbose) override
		{
			if (FreeLibrary(_handle) == 0) {
				if (verbose) std::cout << "Can't close " << _pathToLib << std::endl;
			}
		}

	};

}