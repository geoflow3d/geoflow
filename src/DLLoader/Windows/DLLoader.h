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

		bool DLOpenLib() override
		{
			if (!(_handle = LoadLibrary(_pathToLib.c_str()))) {
				std::cout << "Can't open and load " << _pathToLib << std::endl;
				std::cout << "ERROR: " << GetLastErrorAsString() << std::endl;
				return false;
			}

			// check header hash
			using getHeaderHash = void (*)(char *);
			auto headerHashFunc = reinterpret_cast<getHeaderHash>(
					GetProcAddress(_handle, _getHeaderHashSymbol.c_str()));
			if(!headerHashFunc) {
				std::cout << "Can't find _getHeaderHashSymbol symbol in " << _pathToLib << std::endl;
				DLCloseLib();
				return false;
			} else {
				char plugin_hash[33];
				headerHashFunc(plugin_hash);
				if(strcmp(plugin_hash, GF_SHARED_HEADERS_HASH)!=0) {
					std::cout << plugin_hash << ", geof: " << GF_SHARED_HEADERS_HASH << "\n";
					std::cout << "Plugin header hash incompatible!\n";
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
				std::cout << "Can't find allocator or deleter symbol in " << _pathToLib << std::endl;
			}

			return std::shared_ptr<T>(
				allocFunc(),
				[deleteFunc](T *p) { deleteFunc(p); });
		}

		void DLCloseLib() override
		{
			if (FreeLibrary(_handle) == 0) {
				std::cout << "Can't close " << _pathToLib << std::endl;
			}
		}

	};

}