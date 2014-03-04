//////////////////// Copyright //////////////////////// {{{1

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

//////////////////// Preamble ///////////////////////// {{{1

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
#include <set>

struct Entity;
struct Package;
struct Feature;
class Dependency;
class ConflictGraph;

typedef std::vector<Entity*>    EntityList;
typedef std::vector<EntityList> EntityFormula;
typedef std::vector<Feature*>   FeatureList;
typedef std::vector<Package*>   PackageList;

//////////////////// Criterion //////////////////////// {{{1

struct Criterion
{
    enum Selector { SOLUTION, CHANGED, NEW, REMOVED, UP, DOWN, INSTALLREQUEST, UPGRADEREQUEST, REQUEST };
    enum Measurement { COUNT, SUM, NOTUPTODATE, UNSAT_RECOMMENDS, ALIGNED };
    typedef boost::unordered_map<uint32_t, std::set<uint32_t> > AlignedMap;

    bool optimize;
    Measurement measurement;
    Selector selector;
    std::string attr1;
    std::string attr2;
    uint32_t attrUid1;
    uint32_t attrUid2;
    AlignedMap optAligned;
};

//////////////////// Criteria ///////////////////////// {{{1

struct Criteria {
    typedef std::vector<uint32_t> OptProps;
    typedef std::vector<Criterion> CritVec;

    Criteria();
    void init(Dependency *dep, CritVec &vec);

    CritVec criteria;
    OptProps optProps;
};

//////////////////// Entity /////////////////////////// {{{1

struct Entity {
    Entity(uint32_t name, int32_t version, bool installed = false);
    bool operator<(const Entity &ent) const;
    bool operator==(const Entity &ent) const;
    void remove(Dependency *dep);
    void add(Dependency *dep);

    uint32_t addClause();
    virtual void doAdd(Dependency *dep) = 0;
    virtual void dumpAsFacts(Dependency *dep, std::ostream &out) = 0;
    virtual void addToClause(PackageList &clause, Package *self = 0) = 0;
    virtual void addConflictEdges(ConflictGraph &g) = 0;
    bool allVersions() const;

    virtual ~Entity() = 0;

    uint32_t name;
    int32_t  version;
    bool     visited;
    bool     installed;
    bool     remove_;

protected:
    virtual void doRemove(Dependency *dep) = 0;

};

//////////////////// Package ////////////////////////// {{{1

struct Package : public Entity {
    typedef Cudf::Package::Keep Keep;
    typedef std::map<uint32_t, int32_t> IntPropMap;
    typedef std::map<uint32_t, uint32_t> StringPropMap;
    enum Relevant { 
        RELEVANT_NONE = 0,       // reason set is empty
        RELEVANT_SELF = 1,       // reason set is the current package itself
        RELEVANT_EQUAL = 2,      // reason set includes all packages with the same name as the current package
        RELEVANT_RECOMMENDED = 4 // reason set includes all recommended packages of current package
    };

    Package(const Cudf::Package &pkg);
    void dumpAsFacts(Dependency *dep, std::ostream &out);
    void dumpAttrs(Dependency *dep, std::ostream &out);
    void dumpAttr(Dependency *dep, std::ostream &out, unsigned uid);
    void addToClause(PackageList &clause, Package *self = 0);
    void addConflictEdges(ConflictGraph &g);
    bool satisfies(Criterion::Selector sel);
    Relevant relevant(bool optimize, Criterion::Selector sel);
    unsigned relevant(Criterion &crit);
    uint32_t getProp(uint32_t uid) const;
    void doAdd(Dependency *dep);

    EntityList    conflicts;
    EntityFormula depends;
    EntityFormula recommends;
    FeatureList   provides;
    Keep          keep;
    IntPropMap    intProps;
    StringPropMap stringProps;
    // inferred attributes 
    bool optInstalled;
    bool optGtMaxInstalled;
    bool optLtMinInstalled;
    bool optMaxVersion;
    bool optInInstall;
    bool optInUpgrade;
    bool dfsVisited;

protected:
    void doRemove(Dependency *dep);
};

//////////////////// Feature ////////////////////////// {{{1

struct Feature : public Entity {
    Feature(const Cudf::PackageRef &ftr);
    void dumpAsFacts(Dependency *dep, std::ostream &out);
    void addToClause(PackageList &clause, Package *self = 0);
    void doAdd(Dependency *dep);
    void addConflictEdges(ConflictGraph &g);

    PackageList providedBy;

protected:
    void doRemove(Dependency *dep);
};

size_t hash_value(const Feature &ftr);

//////////////////// Request ////////////////////////// {{{1

struct Request {
    Request(uint32_t name);
    void add(Dependency *dep);

    uint32_t   name;
    EntityList requests;
};

//////////////////// ConflictGraph //////////////////// {{{1

class ConflictGraph {
public:
    void addEdges(Package *a, PackageList const &neighbors);
    void init(bool verbose);
    void dump(Dependency *dep, std::ostream &out);
private:
    void components_(bool verbose);
    void cliques_(bool verbose);
private:
    struct PkgCmp {
        bool operator()(Package *a, Package *b) const;
    };
    struct PkgHash {
        size_t operator()(Package *pkg) const;
    };
    bool edgeSort(Package *a, Package *b);
    typedef boost::unordered_map<Package*, PackageList, ConflictGraph::PkgHash> Edges;
    typedef boost::unordered_set<std::pair<Package*, Package*> > EdgeSet;
    EdgeSet edgeSet_;
    Edges edges_;
public:
    typedef std::vector<PackageList> Components;
    Components components;
    Components cliques;
};

//////////////////// Dependency /////////////////////// {{{1

class Dependency {
public:
    typedef boost::unordered_map<uint32_t, EntityList>  EntityMap;
    typedef std::vector<Request>                        RequestList;
    typedef boost::unordered_map<PackageList, uint32_t> ClauseMap;
    friend class Package;
private:
    typedef boost::ptr_vector<Package>    PackageSet;
    typedef boost::unordered_set<Feature> FeatureSet;
    typedef boost::multi_index::multi_index_container<
        std::string, boost::multi_index::indexed_by<
            boost::multi_index::random_access<>,
            boost::multi_index::hashed_unique<boost::multi_index::identity<std::string> >
        >
    > StringSet;

public:
    Dependency(Criteria::CritVec &crits, bool addAll, bool verbose = true);
    uint32_t index(const std::string &s);
    uint32_t index(const char *s);
    const std::string &string(uint32_t index);
    void init(const Cudf::Document &doc);
    void closure();
    void conflicts();
    void add(Entity *ent);
    uint32_t addClause(PackageList &list, std::ostream &out);
    void dumpAsFacts(std::ostream &out);
    bool addAll() const;

    // WARNING: for testing the implementation of this is highly inefficient
    bool test_contains(std::string const &name, int32_t version);

private:
    void initClosure();
    void rewriteRequests();

public:
    Criteria    criteria;

private:
    StringSet     strings_;
    PackageSet    packages_;
    FeatureSet    features_;
    EntityMap     entityMap_;
    EntityList    remove_;
    RequestList   install_;
    RequestList   upgrade_;
    EntityList    closure_;
    ClauseMap     clauses_;
    ConflictGraph conflictGraph_;
    bool          verbose_;
    bool          addAll_;
};
