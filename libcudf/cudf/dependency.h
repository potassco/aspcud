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

#include <cudf/packages.h>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <iostream>
#include <map>

#define CUDF_VERSION "0.9.0"

struct Entity;
struct Package;
struct Feature;
class Dependency;

typedef std::vector<Entity*>    EntityList;
typedef std::vector<EntityList> EntityFormula;
typedef std::vector<Feature*>   FeatureList;
typedef std::vector<Package*>   PackageList;

struct Entity
{
	Entity(uint32_t name, uint32_t version, bool installed = false);
	bool operator<(const Entity &ent) const;
	bool operator==(const Entity &ent) const;
	void remove(Dependency *dep);
	void add(Dependency *dep);

	uint32_t addClause();
	virtual void doAdd(Dependency *dep) = 0;
	virtual void dumpAsFacts(Dependency *dep, std::ostream &out) = 0;
	virtual void addToClause(PackageList &clause, Package *self = 0) = 0;
	bool allVersions() const;

	virtual ~Entity() = 0;

	uint32_t name;
	uint32_t version;
	bool     visited;
	bool     installed;

protected:
	virtual void doRemove(Dependency *dep) = 0;

	bool remove_;
};

struct Package : public Entity
{
	typedef Cudf::Package::Keep Keep;
	typedef Cudf::Package::IntPropMap IntPropMap;

	Package(const Cudf::Package &pkg);
	void dumpAsFacts(Dependency *dep, std::ostream &out);
	void addToClause(PackageList &clause, Package *self = 0);
	void doAdd(Dependency *dep);

	EntityList    conflicts;
	EntityFormula depends;
	EntityFormula recommends;
	FeatureList   provides;
	Keep          keep;
	IntPropMap    intProps;

protected:
	void doRemove(Dependency *dep);

};

struct Feature : public Entity
{
	Feature(const Cudf::PackageRef &ftr);
	void dumpAsFacts(Dependency *dep, std::ostream &out);
	void addToClause(PackageList &clause, Package *self = 0);
	void doAdd(Dependency *dep);

	PackageList providedBy;

protected:
	void doRemove(Dependency *dep);
};

size_t hash_value(const Feature &ftr);

struct Request
{
	Request(uint32_t name);
	void add(Dependency *dep);

	uint32_t   name;
	EntityList requests;

};

class Dependency
{
public:
	typedef boost::unordered_map<uint32_t, EntityList>  EntityMap;
	typedef std::vector<Request>                        RequestList;
	typedef boost::unordered_map<PackageList, uint32_t> ClauseMap;
	struct Criteria
	{
		typedef std::map<std::string, int> OptSizeMap;

		Criteria();

		int        removed;
		int        newpkg;
		int        changed;
		int        unsat_recommends;
		int        notuptodate;
		OptSizeMap optSize;
	};

private:
	typedef boost::ptr_vector<Package>    PackageSet;
	typedef boost::unordered_set<Feature> FeatureSet;
	typedef boost::multi_index::multi_index_container
	<
		std::string, boost::multi_index::indexed_by
		<
			boost::multi_index::random_access<>,
			boost::multi_index::hashed_unique<boost::multi_index::identity<std::string> >
		>
	> StringSet;

public:
	Dependency(const Criteria &criteria, bool verbose = true);
	uint32_t index(const std::string &s);
	uint32_t index(const char *s);
	const std::string &string(uint32_t index);
	void init(const Cudf::Document &doc);
	void closure(bool addAll);
	void add(Entity *ent);
	void addMaxVersion(uint32_t name, Package *reason = 0);
	uint32_t addClause(PackageList &list, std::ostream &out);
	void dumpAsFacts(std::ostream &out);

private:
	void initClosure();
	void rewriteRequests();

public:
	Criteria    criteria;

private:
	StringSet   strings_;
	PackageSet  packages_;
	FeatureSet  features_;
	EntityMap   entityMap_;
	EntityList  remove_;
	RequestList install_;
	RequestList upgrade_;
	EntityList  closure_;
	ClauseMap   clauses_;
	bool        verbose_;
};
