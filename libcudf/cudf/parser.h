//
// Copyright (c) 2010, Roland Kaminski <kaminski@cs.uni-potsdam.de>
//
// This file is part of aspcud.
//
// gringo is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// gringo is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with gringo.  If not, see <http://www.gnu.org/licenses/>.
//

#pragma once

#include <cudf/lexer_impl.h>
#include <utility>
#include <vector>
#include <stack>
#include <map>
#include <cudf/dependency.h>
#include <boost/lexical_cast.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/foreach.hpp>

class Parser : public LexerImpl
{
public:
	enum RelOp
	{
		GE  = Cudf::PackageRef::GE,
		LE  = Cudf::PackageRef::LE,
		EQ  = Cudf::PackageRef::EQ,
		NEQ = Cudf::PackageRef::NEQ,
		LT,
		GT
	};
	struct Token
	{
		uint32_t index;
	};
	struct Type
	{
		Type(uint32_t type);
		bool intType() const;

		uint32_t    type;
		Cudf::Value value;
	};

public:
	Parser(Dependency &dep);
	int lex();
	std::string errorToken();
	void syntaxError();
	void parseError();
	void parse(std::istream &sin);
	~Parser();

	void parseType(uint32_t index);
	int lexString();
	void parseString()
	{
		lexString_ = true;
	}
	Cudf::Value &addType(uint32_t name, uint32_t type)
	{
		std::pair<TypeMap::iterator, bool> res = typeMap_.insert(TypeMap::value_type(name, Type(type)));
		if (!res.second) { throw std::runtime_error("duplicate type"); }
		return res.first->second.value;
	}
	bool mapBool(uint32_t index)
	{
		assert(index == falseStr_ || index == trueStr_);
		return index == trueStr_;
	}
	int32_t mapInt(uint32_t index)
	{
		return boost::lexical_cast<int32_t>(dep_.string(index));
	}
	void setPkgRef(uint32_t name, uint32_t op = Cudf::PackageRef::GE, uint32_t version = std::numeric_limits<uint32_t>::max())
	{
		pkgRef.name    = name;
		pkgRef.version = (version == std::numeric_limits<uint32_t>::max()) ? 0 : boost::lexical_cast<int32_t>(dep_.string(version));
		if (op == GT)
		{
			pkgRef.version++;
			pkgRef.op = Cudf::PackageRef::GE;
		}
		else if (op == LT)
		{
			pkgRef.version--;
			pkgRef.op = Cudf::PackageRef::LE;
		}
		else { pkgRef.op = op; }
	}
	void pushPkgList()
	{
		pkgFormula.push_back(Cudf::PkgList());
		std::swap(pkgFormula.back(), pkgList);
	}
	template <class T>
	void setProperty(uint32_t name, T &value)
	{
		std::pair<PropMap::iterator, bool> res = propMap_.insert(PropMap::value_type(name, T()));
		if(!res.second)
		{
			throw std::runtime_error("duplicate property");
		}
		std::swap(boost::any_cast<T&>(res.first->second), value);
	}
	template <class T>
	void setProperty(uint32_t name, const T &value)
	{
		if(!propMap_.insert(PropMap::value_type(name, value)).second)
		{
			throw std::runtime_error("duplicate property");
		}
	}
	template <class T>
	void getProp(uint32_t name, T &dst)
	{
		PropMap::iterator it = propMap_.find(name);
		if (it != propMap_.end())
		{
			dst = boost::any_cast<T&>(it->second);
		}
		else
		{
			TypeMap::iterator it = typeMap_.find(name);
			assert(it != typeMap_.end());
			if (!it->second.value.empty())
			{
				dst = boost::any_cast<T&>(it->second.value);
			}
			else { throw std::runtime_error("required attribute missing"); }
		}
	}
	void addPreamble()
	{
		propMap_.clear();
	}
	void addPackage(uint32_t name)
	{
		doc_->packages.push_back(Cudf::Package(name));
		Cudf::Package &pkg = doc_->packages.back();
		getProp(versionStr_,    pkg.version);
		getProp(conflictsStr_,  pkg.conflicts);
		getProp(dependsStr_,    pkg.depends);
		if (typeMap_.find(recommendsStr_) != typeMap_.end()) { getProp(recommendsStr_, pkg.recommends); }
		getProp(providesStr_,   pkg.provides);
		getProp(installedStr_,  pkg.installed);

		uint32_t keep;
		getProp(keepStr_, keep);

		if (keep == packageStr_)      { pkg.keep = Cudf::Package::PACKAGE; }
		else if (keep == featureStr_) { pkg.keep = Cudf::Package::FEATURE; }
		else if (keep == versionStr_) { pkg.keep = Cudf::Package::VERSION; }
		else if (keep == noneStr_)    { pkg.keep = Cudf::Package::NONE; }
		else                          { throw std::runtime_error("invalid keep value"); }

		if (!dep_.addAll())
		{
			BOOST_FOREACH (uint32_t name, optSize_)
			{
				int32_t value;
				getProp(name, value);
				pkg.intProps.insert(Package::IntPropMap::value_type(name, value));
			}
		}
		else
		{
			BOOST_FOREACH (TypeMap::value_type &val, typeMap_)
			{
				if (val.second.intType())
				{
					int32_t value;
					getProp(val.first, value);
					pkg.intProps.insert(Package::IntPropMap::value_type(val.first, value));
				}
			}
		}
		propMap_.clear();
	}
	void addRequest()
	{
		getProp(installStr_, doc_->request.install);
		getProp(removeStr_,  doc_->request.remove);
		getProp(upgradeStr_, doc_->request.upgrade);
		propMap_.clear();
	}

private:
	typedef boost::unordered_map<uint32_t, Type>        TypeMap;
	typedef boost::unordered_map<uint32_t, Cudf::Value> PropMap;
	typedef std::vector<uint32_t>                       EnumValues;
	typedef std::vector<uint32_t>                       OptSizeVec;

	Dependency      &dep_;
	Cudf::Document  *doc_;
	void            *parser_;
	Token            token_;
	bool             lexString_;
	uint32_t         shiftToken_;

	OptSizeVec       optSize_;
	TypeMap          typeMap_;
	PropMap          propMap_;

	uint32_t         versionStr_;
	uint32_t         conflictsStr_;
	uint32_t         dependsStr_;
	uint32_t         recommendsStr_;
	uint32_t         providesStr_;
	uint32_t         keepStr_;
	uint32_t         installedStr_;
	uint32_t         installStr_;
	uint32_t         removeStr_;
	uint32_t         upgradeStr_;
	uint32_t         packageStr_;
	uint32_t         featureStr_;
	uint32_t         noneStr_;
	uint32_t         trueStr_;
	uint32_t         falseStr_;

public:
	Cudf::PackageRef pkgRef;
	Cudf::PkgList    pkgList;
	Cudf::PkgFormula pkgFormula;
	EnumValues       identList;
};
