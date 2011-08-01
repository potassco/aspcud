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

class Parser : public LexerImpl
{
public:
	enum RelOp { GE=0, GT, LE, LT, EQ, NEQ };
	struct Token
	{
		uint32_t index;
	};

public:
	Parser(Dependency &dep);
	int lex();
	int lexIgnore();
	int lexInt();
	std::string errorToken();
	void syntaxError();
	void parseError();
	void parse(std::istream &sin);
	~Parser();

	// package
	void newPackage(uint32_t name)
	{
		doc_->packages.push_back(Cudf::Package(name));
		package_ = &doc_->packages.back();
	}
	void setVersion(uint32_t version)
	{
		if(package_) { package_->version = boost::lexical_cast<uint32_t>(dep_.string(version)); }
	}
	void setProvides()
	{
		if(package_) { package_->provides.swap(pkgList_); }
		pkgList_.clear();
	}
	void setInstalled(uint32_t value)
	{
		if(package_) { package_->installed = boolMap_[value]; }
	}
	void setKeep(uint32_t value)
	{
		if(package_) { package_->keep = keepMap_[value]; }
	}
	void setDepends()
	{
		if(package_) { package_->depends.swap(pkgFormula_); }
		pkgFormula_.clear();
	}
	void setConflicts()
	{
		if(package_) { package_->conflicts.swap(pkgList_); }
		pkgList_.clear();
	}
	void setRecommends()
	{
		if(package_) { package_->recommends.swap(pkgFormula_); }
		pkgFormula_.clear();
	}

	void setIntProp(uint32_t key, uint32_t val)
	{
		if (package_) { package_->intProps.push_back(Cudf::Package::IntPropMap::value_type(key, boost::lexical_cast<uint32_t>(dep_.string(val)))); }
	}

	// package references
	void addPkg(uint32_t name, uint32_t op = Cudf::PackageRef::GE, uint32_t version = std::numeric_limits<uint32_t>::max())
	{
		packageRef_.name = name;
		version = (version == std::numeric_limits<uint32_t>::max()) ? 0 : boost::lexical_cast<uint32_t>(dep_.string(version));
		switch(op)
		{
		case GE:
			packageRef_.op      = Cudf::PackageRef::GE;
			packageRef_.version = version;
			break;
		case GT:
			packageRef_.op      = Cudf::PackageRef::GE;
			packageRef_.version = version + 1;
			break;
		case LE:
			packageRef_.op      = Cudf::PackageRef::LE;
			packageRef_.version = version;
			break;
		case LT:
			packageRef_.op      = Cudf::PackageRef::LE;
			packageRef_.version = version - 1;
			break;
		case EQ:
			packageRef_.op      = Cudf::PackageRef::EQ;
			packageRef_.version = version;
			break;
		case NEQ:
			packageRef_.op      = Cudf::PackageRef::NEQ;
			packageRef_.version = version;
			break;
		}
	}

	// formulas
	void setConstant(bool value)
	{
		pkgFormula_.clear();
		if(!value) { pkgFormula_.resize(1); }
	}
	void addClause()
	{
		pkgFormula_.resize(pkgFormula_.size() + 1);
		pkgFormula_.back().swap(pkgList_);
	}
	void addToClause() {
		pkgList_.push_back(packageRef_);
	}

	// lists
	void addToList() { pkgList_.push_back(packageRef_); }

	// requests
	void newRequest()
	{
		package_ = 0;
		request_ = true;
	}
	void setInstall()
	{
		if(request_) { pkgList_.swap(doc_->request.install); }
		pkgList_.clear();
	}
	void setRemove()
	{
		if(request_) { pkgList_.swap(doc_->request.remove); }
		pkgList_.clear();
	}
	void setUpgrade()
	{
		if(request_) { pkgList_.swap(doc_->request.upgrade); }
		pkgList_.clear();
	}

private:
	typedef std::map<uint32_t, bool>                BoolMap;
	typedef std::map<uint32_t, Cudf::Package::Keep> KeepMap;

	void            *parser_;
	Token            token_;
	bool             lexIgnore_;
	bool             lexInt_;
	bool             request_;
	Dependency      &dep_;
	Cudf::Document  *doc_;
	Cudf::PackageRef packageRef_;
	Cudf::Package   *package_;
	Cudf::PkgList    pkgList_;
	Cudf::PkgFormula pkgFormula_;
	KeepMap          keepMap_;
	BoolMap          boolMap_;
};
