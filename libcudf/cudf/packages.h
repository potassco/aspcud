#pragma once

#include <stdint.h>
#include <vector>
#include <memory>
#include <limits>
#include <boost/shared_ptr.hpp>
#include <boost/functional/hash.hpp>

namespace Cudf
{
	struct PackageRef
	{
		enum RelOp { GE=0, LE, EQ, NEQ };

		uint32_t name;
		RelOp    op;
		uint32_t version;

		PackageRef(uint32_t name = 0, RelOp op = GE, uint32_t version = 0);
	};

	struct PkgList : public std::vector<PackageRef>
	{
	};

	struct PkgFormula : public std::vector<PkgList>
	{
	};

	struct Package
	{
		enum Keep { VERSION, PACKAGE, FEATURE, NONE };

		Package(uint32_t name = std::numeric_limits<uint32_t>::max(), uint32_t version = 0)
			: name(name)
			, version(version)
			, keep(NONE)
			, installed(false)
		{ }

		bool initialized() { return name != std::numeric_limits<uint32_t>::max(); }

		uint32_t      name;
		uint32_t      version;
		PkgList       conflicts;  // default empty
		PkgFormula    depends;    // default true
		PkgFormula    recommends; // default empty
		PkgList       provides;   // default empty
		Keep          keep;       // default NONE
		bool          installed;  // default false
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

}
