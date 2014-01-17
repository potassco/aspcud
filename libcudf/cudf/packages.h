//
// Copyright (c) 2010, Roland Kaminski <kaminski@cs.uni-potsdam.de>
//
// This file is part of aspcud.
//
// aspcud is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// aspcud is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with aspcud.  If not, see <http://www.gnu.org/licenses/>.
//

#pragma once

#include <stdint.h>
#include <vector>
#include <memory>
#include <limits>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/functional/hash.hpp>
#include <boost/any.hpp>

namespace Cudf
{
	struct PackageRef
	{
		enum RelOp { GE=0, LE, EQ, NEQ };

		uint32_t  name;
		uint32_t  op;
		int32_t   version;

		PackageRef(uint32_t name = 0, RelOp op = GE, int32_t version = 0);
	};

	typedef std::vector<PackageRef> PkgList;
	typedef std::vector<PkgList> PkgFormula;

	struct Package
	{
		typedef std::map<uint32_t, int32_t> IntPropMap;
		typedef std::map<uint32_t, uint32_t> StringPropMap;
		enum Keep { VERSION, PACKAGE, FEATURE, NONE };

		Package(uint32_t name = std::numeric_limits<uint32_t>::max(), int32_t version = 0)
			: name(name)
			, version(version)
			, keep(NONE)
			, installed(false)
		{ }

		bool initialized() { return name != std::numeric_limits<uint32_t>::max(); }

		uint32_t      name;
		int32_t       version;
		PkgList       conflicts;   // default empty
		PkgFormula    depends;     // default true
		PkgFormula    recommends;  // default empty
		PkgList       provides;    // default empty
		Keep          keep;        // default NONE
		bool          installed;   // default false
		IntPropMap    intProps;    // default empty
		StringPropMap stringProps; // default empty
	};

	struct Request
	{
		PkgList install;
		PkgList upgrade;
		PkgList remove;
	};

	struct Document
	{
		typedef std::vector<Package> Packages;
		Packages packages;
		Request  request;
	};

	typedef boost::any Value;
}
