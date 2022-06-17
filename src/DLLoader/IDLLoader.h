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

#include <memory>
#include <string>
#include <geoflow/gfSharedHeadersHash.h>

struct ImGuiContext;

namespace dlloader
{

	/*
	** Interface for Dynamic Library Loading (DLLoader)
	** API for Unix and Windows. Handling of open, close, validity-check.
	*/
	template <class T>
	class IDLLoader
	{

	public:

		virtual ~IDLLoader() = default;

		/*
		**
		*/
		virtual bool DLOpenLib(bool verbose=false) = 0;

		/*
		** Return a shared pointer on an instance of class loaded through
		** a dynamic library.
		*/
		virtual std::shared_ptr<T>	DLGetInstance() = 0;

		/*
		** Correctly delete the instance of the "dynamically loaded" class.
		*/
		virtual void DLCloseLib(bool verbose=false) = 0;

	};
}