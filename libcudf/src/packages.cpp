#include <cudf/packages.h>

namespace Cudf
{

	PackageRef::PackageRef(uint32_t name, RelOp op, uint32_t version)
		: name(name)
		, op(op)
		, version(version)
	{
	}

}
