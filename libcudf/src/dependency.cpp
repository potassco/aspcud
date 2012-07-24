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

#include <cudf/dependency.h>
#include <boost/lambda/lambda.hpp>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm/unique.hpp>
#include <boost/range/algorithm/find.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <typeinfo>
#include <map>

//////////////////// Helper ////////////////////

namespace
{
    template <class T>
    void sort_uniq_ptr(T &t)
    {
        t.resize(boost::range::unique(boost::range::sort(t, *boost::lambda::_1 < *boost::lambda::_2), *boost::lambda::_1 == *boost::lambda::_2).size());
    }

    template <class T>
    void sort_uniq(T &t)
    {
        t.resize(boost::range::unique(boost::range::sort(t)).size());
    }

    struct CudfPackageRefFilter
    {
        CudfPackageRefFilter(const Cudf::PackageRef &ref) : ref(ref) { }
        bool operator()(const Entity *entity)
        {
            switch(ref.op)
            {
            case Cudf::PackageRef::EQ:
                return (entity->version == ref.version || entity->allVersions()) && ref.version != 0;
            case Cudf::PackageRef::NEQ:
                return  entity->version != ref.version || entity->allVersions();
            case Cudf::PackageRef::LE:
                return (entity->version <= ref.version || entity->allVersions()) && ref.version != 0;
            case Cudf::PackageRef::GE:
                return  entity->version >= ref.version || entity->allVersions();
            }
            return true;
        }
        const Cudf::PackageRef &ref;
    };

#define refRange(map, ref) ( map[ref.name] | boost::adaptors::filtered(CudfPackageRefFilter(ref)) )

    void unroll(Dependency::EntityMap &map, const Cudf::PkgList &clause, EntityList &list)
    {
        foreach(const Cudf::PackageRef &ref, clause)
        {
            boost::range::push_back(list, refRange(map, ref));
        }
        sort_uniq_ptr(list);
    }

    void unroll(Dependency::EntityMap &map, const Cudf::PkgFormula &formula, EntityFormula &list)
    {
        foreach(const Cudf::PkgList &clause, formula)
        {
            list.push_back(EntityList());
            unroll(map, clause, list.back());
        }
    }

    void unroll(Dependency::EntityMap &map, const Cudf::PkgList &formula, Dependency::RequestList &requests)
    {
        foreach(const Cudf::PackageRef &clause, formula)
        {
            requests.push_back(Request(clause.name));
            boost::range::push_back(requests.back().requests, refRange(map, clause));
            sort_uniq_ptr(requests.back().requests);
        }
    }

#undef refRange
}

//////////////////// Entity ////////////////////

Entity::Entity(uint32_t name, int32_t version, bool installed)
    : name(name)
    , version(version)
    , visited(false)
    , installed(installed)
    , remove_(false)
{
}

bool Entity::operator<(const Entity &ent) const
{
    // NOTE: will sort the maximum version to the end
    //       see Feature::allVersions()
    if (version != ent.version) { return version < ent.version; }
    else if (name != ent.name)  { return name    < ent.name; }
    else                       { return typeid(this).before(typeid(&ent)); }
}

bool Entity::operator==(const Entity &ent) const
{
    return
        typeid(this) == typeid(&ent) &&
        name         == ent.name &&
        version      == ent.version;
}

void Entity::remove(Dependency *dep)
{
    if (!remove_)
    {
        remove_ = true;
        doRemove(dep);
    }
}

void Entity::add(Dependency *dep)
{
    if (!visited && !remove_)
    {
        visited = true;
        dep->add(this);
    }
}

bool Entity::allVersions() const
{
    return version == std::numeric_limits<int32_t>::max();
}

Entity::~Entity()
{
}

//////////////////// Package ////////////////////

Package::Package(const Cudf::Package &pkg)
    : Entity(pkg.name, pkg.version, pkg.installed)
    , keep(pkg.keep)
    , intProps(pkg.intProps)
    , stringProps(pkg.stringProps)
{
}

void Package::doRemove(Dependency *)
{
}

bool Package::satisfies(EntityList &list, Criterion::Selector sel)
{
    // Note: check if installing the package would satisfy the selector
    switch(sel)
    {
        case Criterion::SOLUTION: { return true; }
        case Criterion::CHANGED:  { return !installed; }
        case Criterion::NEW:
        {
            foreach (Entity *ent, list)
            {
                if (dynamic_cast<Package*>(ent) && ent->installed) { return false; }
            }
            return true;
        }
        case Criterion::REMOVED: { return false; }
        case Criterion::UP:
        {
            foreach (Entity *ent, list)
            {
                if (dynamic_cast<Package*>(ent) && ent->version >= version && ent->installed)
                {
                    return false;
                }
            }
            return true; 
        }
        case Criterion::DOWN:    
        {
            foreach (Entity *ent, list)
            {
                if (dynamic_cast<Package*>(ent) && ent->version <= version && ent->installed)
                {
                    return false;
                }
            }
            return true; 
        }
    }
}

bool Package::satisfies(EntityList &list, bool optimize, Criterion::Selector sel)
{
    switch(sel)
    {
        case Criterion::SOLUTION: { return optimize; }
        case Criterion::CHANGED:  { return optimize != installed; }
        case Criterion::NEW:
        {
            if (optimize) { return false; }
            foreach (Entity *ent, list)
            {
                if (dynamic_cast<Package*>(ent) && ent->installed) { return false; }
            }
            return true;
        }
        case Criterion::REMOVED: { return optimize && installed; }
        case Criterion::UP:
        {
            if (!optimize) { return false; }
            foreach (Entity *ent, list)
            {
                if (dynamic_cast<Package*>(ent) && ent->version >= version && ent->installed)
                {
                    return false;
                }
            }
            return true; 
        }
        case Criterion::DOWN:    
        {
            if (!optimize) { return false; }
            foreach (Entity *ent, list)
            {
                if (dynamic_cast<Package*>(ent) && ent->version <= version && ent->installed)
                {
                    return false;
                }
            }
            return true; 
        }
    }
}

bool Package::satisfies(Dependency *dep, EntityList &list, Criterion &crit)
{
    if (remove_) { return false; }
    switch (crit.measurement)
    {
        case Criterion::COUNT: { return satisfies(list, crit.optimize, crit.selector); }
        case Criterion::SUM:
        {
            int attr = intProps[crit.attrUid1];
            if (attr > 0 && !crit.optimize || attr < 0 && crit.optimize)
            {
                return satisfies(list, false, crit.selector);
            }
            else if (attr < 0 && !crit.optimize || attr > 0 && crit.optimize)
            {
                return satisfies(list, true, crit.selector);
            }
            return false;
        }
        case Criterion::NOTUPTODATE:
        {
            if (!satisfies(list, crit.optimize, crit.selector)) { return false; }
            foreach (Entity *ent, list)
            {
                if (dynamic_cast<Package*>(ent) && ent->version > version)
                {
                    return false;
                }
            }
            return true;
        }
        case Criterion::UNSAT_RECOMMENDS:
        {
            return !recommends.empty() && satisfies(list, crit.optimize, crit.selector);
        }
        case Criterion::ALIGNED:
        {
            return dep->aligned_[std::make_pair(std::make_pair(crit.attrUid1, crit.attrUid2), getProp(crit.attrUid1))].size() > 1 && satisfies(list, crit.optimize, crit.selector);
        }
    }
}

void Package::doAdd(Dependency *dep)
{
    if (!dep->addAll())
    {
        foreach(EntityList &clause, depends)
        {
            foreach(Entity *ent, clause) { ent->add(dep); }
        }
        foreach (Criterion &crit, dep->criteria.criteria)
        {
            if (crit.measurement == Criterion::UNSAT_RECOMMENDS && !crit.optimize && satisfies(dep->entityMap_[name], crit.selector))
            {
                foreach(EntityList &clause, recommends)
                {
                    foreach(Entity *ent, clause) { ent->add(dep); }
                }
            }
        }
    }
}

uint32_t Package::getProp(uint32_t uid) const
{
    IntPropMap::const_iterator it = intProps.find(uid);
    if (it != intProps.end()) { return it->second; }
    else
    {
        StringPropMap::const_iterator jt = stringProps.find(uid);
        if (jt != stringProps.end()) { return jt->second; }
    }
    return 0;
}

void Package::dumpAsFacts(Dependency *dep, std::ostream &out)
{
    // unit(VP)
    out << "unit(\"" << dep->string(name) << "\"," << version << ").\n";
    // satisfies(VP,D)
    // depends(VP,D)
    foreach(EntityList &clause, depends)
    {
        PackageList pkgClause;
        foreach(Entity *ent, clause) { ent->addToClause(pkgClause); }
        uint32_t condition = dep->addClause(pkgClause, out);
        out << "depends(\"" << dep->string(name) << "\"," << version << "," << condition << ").\n";
    }
    // conflicts(VP, D)
    if (!conflicts.empty()) {
        PackageList pkgClause;
        foreach(Entity *ent, conflicts) { ent->addToClause(pkgClause, this); }
        uint32_t condition = dep->addClause(pkgClause, out);
        out << "conflict(\"" << dep->string(name) << "\"," << version << "," << condition << ").\n";
    }
}

void Package::dumpAttr(Dependency *dep, std::ostream &out, unsigned uid)
{
    out << "attribute(\"" << dep->string(name) << "\"," << version << ",\"" << dep->string(uid) << "\",";
    IntPropMap::const_iterator it = intProps.find(uid);
    if (it != intProps.end()) { out << it->second; }
    else
    {
        StringPropMap::const_iterator jt = stringProps.find(uid);
        // Note: we do not care for the value at all
        if (jt != stringProps.end()) { out << jt->second; }
    }
    out << ").\n";
}

void Package::addToClause(PackageList &clause, Package *self)
{
    if (!remove_ && this != self) { clause.push_back(this); }
}

//////////////////// Feature ////////////////////

Feature::Feature(const Cudf::PackageRef &ftr)
    : Entity(ftr.name, ftr.version == 0 ? std::numeric_limits<int32_t>::max() : ftr.version, false)
{
}

void Feature::doRemove(Dependency *dep)
{
    foreach(Package *pkg, providedBy) { pkg->remove(dep); }
}

void Feature::doAdd(Dependency *dep)
{
    foreach(Package *pkg, providedBy) { pkg->add(dep); }
}

void Feature::dumpAsFacts(Dependency *, std::ostream &)
{
}

size_t hash_value(const Feature &ftr)
{
    size_t seed = 0;
    boost::hash_combine(seed, ftr.name);
    boost::hash_combine(seed, ftr.version);
    return seed;
}

void Feature::addToClause(PackageList &clause, Package *self)
{
    if (!remove_)
    {
        foreach(Package *pkg, providedBy)
        {
            pkg->addToClause(clause, self);
        }
    }
}

//////////////////// Request ////////////////////

Request::Request(uint32_t name)
    : name(name)
{
}

void Request::add(Dependency *dep)
{
    foreach(Entity *ent, requests) { ent->add(dep); }
}

//////////////////// Criteria ////////////////////
    
Criteria::Criteria()
{
}

void Criteria::init(Dependency *dep, CritVec &vec)
{
    std::swap(criteria, vec);
    foreach(Criterion &crit, criteria)
    {
        if (!crit.attr1.empty()) 
        {
            uint32_t uid = dep->index(crit.attr1);
            optProps.push_back(uid); 
        }
        if (!crit.attr2.empty())
        {
            uint32_t uid = dep->index(crit.attr2);
            optProps.push_back(uid);
        }
    }
    sort_uniq(optProps);
}

//////////////////// Dependency ////////////////////

Dependency::Dependency(Criteria::CritVec &crits, bool addAll, bool verbose)
    : verbose_(verbose)
    , addAll_(addAll)
{
    criteria.init(this, crits);
}

uint32_t Dependency::index(const std::string &s)
{
    StringSet::iterator it = strings_.push_back(s).first;
    return it - strings_.begin();
}

uint32_t Dependency::index(const char *s)
{
    return index(std::string(s));
}

const std::string &Dependency::string(uint32_t index)
{
    return strings_.at(index);
}

void Dependency::init(const Cudf::Document &doc)
{
    // first pass: add packages and features
    foreach(const Cudf::Package &cudfPkg, doc.packages)
    {
        packages_.push_back(new Package(cudfPkg));
        Package *pkg = &packages_.back();
        entityMap_[pkg->name].push_back(pkg);
        foreach(const Cudf::PackageRef &provided, cudfPkg.provides)
        {
            // NOTE: version might be zero here, which is than mapped to the maximum integer value
            std::pair<FeatureSet::iterator, bool> res = features_.insert(Feature(provided));
            Feature *ftr = const_cast<Feature*>(&*res.first);
            if (pkg->installed) { ftr->installed = true; }
            pkg->provides.push_back(ftr);
            ftr->providedBy.push_back(pkg);
            if (res.second) { entityMap_[ftr->name].push_back(ftr); }
        }
        sort_uniq_ptr(pkg->provides);
    }
    // second pass: roll out dependencies
    PackageSet::iterator current = packages_.begin();
    foreach(const Cudf::Package &cudfPkg, doc.packages)
    {
        Package *pkg = &*current++;
        unroll(entityMap_, cudfPkg.conflicts, pkg->conflicts);
        unroll(entityMap_, cudfPkg.depends, pkg->depends);
        unroll(entityMap_, cudfPkg.recommends, pkg->recommends);
    }
    unroll(entityMap_, doc.request.remove,  remove_);
    unroll(entityMap_, doc.request.install, install_);
    unroll(entityMap_, doc.request.upgrade, upgrade_);
}

void Dependency::add(Entity *ent)
{
    closure_.push_back(ent);
}

void Dependency::rewriteRequests()
{
    foreach(Entity *ent, remove_) { ent->remove(this); }
    foreach(Request &request, upgrade_)
    {
        foreach(Entity *ent, request.requests) { ent->visited = true; }
        int32_t removeVersion = 0;
        foreach(Entity *ent, entityMap_[request.name])
        {
            // if some version is installed then a >= version must be installed
            if (ent->installed && removeVersion < ent->version) { removeVersion = ent->version; }
        }
        foreach(Entity *ent, entityMap_[request.name])
        {
            // no unrequested or smaller version may be installed
            if (!ent->visited || (removeVersion != 0 && (ent->version < removeVersion || ent->allVersions())))
            {
                ent->remove(this);
            }
        }
        foreach(Entity *ent, request.requests) { ent->visited = false; }
    }
    // rewrite keep flags into installs
    foreach(Package &pkg, packages_)
    {
        if (pkg.installed)
        {
            switch(pkg.keep)
            {
                case Cudf::Package::FEATURE:
                {
                    foreach(Feature *ftr, pkg.provides)
                    {
                        install_.push_back(Request(ftr->name));
                        install_.back().requests.push_back(ftr);
                    }
                    break;
                }
                case Cudf::Package::VERSION:
                {
                    install_.push_back(Request(pkg.name));
                    install_.back().requests.push_back(&pkg);
                    break;
                }
                case Cudf::Package::PACKAGE:
                {
                    install_.push_back(Request(pkg.name));
                    foreach(Entity *ent, entityMap_[pkg.name])
                    {
                        if (dynamic_cast<Package*>(ent))
                        {
                            install_.back().requests.push_back(ent);
                        }
                    }
                    break;
                }
                case Cudf::Package::NONE: { break; }
            }
        }
    }
}

void Dependency::initClosure()
{
    foreach(Request &request, upgrade_) { request.add(this); }
    foreach(Request &request, install_) { request.add(this); }

    foreach(Criterion &crit, criteria.criteria)
    {
        if (crit.measurement == Criterion::ALIGNED)
        {
            foreach(EntityList &list, entityMap_ | boost::adaptors::map_values)
            {
                foreach(Entity *ent, list)
                {
                    Package *pkg = dynamic_cast<Package*>(ent);
                    if (pkg)
                    {
                        // performance: a pair<uint32_t,bool> as value would be sufficient
                        aligned_[std::make_pair(std::make_pair(crit.attrUid1, crit.attrUid2), pkg->getProp(crit.attrUid1))].insert(pkg->getProp(crit.attrUid2));
                    }
                }
            }
        }
    }

    foreach(EntityList &list, entityMap_ | boost::adaptors::map_values)
    {
        foreach(Entity *ent, list)
        {
            Package *pkg = dynamic_cast<Package*>(ent);
            if(pkg)
            {
                foreach(Criterion &crit, criteria.criteria)
                {
                    if (pkg->satisfies(this, list, crit)) { pkg->add(this); }
                }
            }
        }
    }
}

void Dependency::closure()
{
    rewriteRequests();
    if (addAll_)
    {
        foreach(EntityList &list, entityMap_ | boost::adaptors::map_values)
        {
            foreach(Entity *ent, list) { ent->add(this); }
        }
    }
    else { initClosure(); }
    for(PackageList::size_type i = 0; i < closure_.size(); i++) { closure_[i]->doAdd(this); }
    if (verbose_)
    {
        std::cerr << "sizes: " << std::endl;
        std::cerr << "  features: " << features_.size() << std::endl;
        std::cerr << "  packages: " << packages_.size() << std::endl;
        std::cerr << "  closure:  " << closure_.size() << std::endl;
    }
}

uint32_t Dependency::addClause(PackageList &clause, std::ostream &out)
{
    sort_uniq(clause);
    std::pair<ClauseMap::iterator,bool> res = clauses_.insert(ClauseMap::value_type(clause, 0));
    if (res.second)
    {
        res.first->second = clauses_.size();
        foreach(Package *pkg, clause)
        {
            out << "satisfies(\"" << string(pkg->name) << "\"," << pkg->version << "," << res.first->second << ").\n";
        }
    }
    return res.first->second;
}

bool Dependency::addAll() const
{
    return addAll_;
}

void Dependency::dumpAsFacts(std::ostream &out)
{
    foreach(Entity *ent, closure_) { ent->dumpAsFacts(this, out); }
    // installed(VP)
    foreach(Package &pkg, packages_)
    {
        if (pkg.installed)
        {
            out << "installed(\"" << string(pkg.name) << "\"," << pkg.version << ").\n";
        }
    }
    // requests according to install request
    foreach(Request &request, install_)
    {
        PackageList pkgClause;
        foreach(Entity *ent, request.requests) { ent->addToClause(pkgClause); }
        uint32_t condition = addClause(pkgClause, out);
        out << "request(" << condition << ").\n";
    }
    // requests/conflicts according to upgrade request
    foreach(Request &request, upgrade_)
    {
        PackageList pkgClause;
        foreach(Entity *ent, request.requests) { ent->addToClause(pkgClause); }
        uint32_t condition = addClause(pkgClause, out);
        out << "request(" << condition << ").\n";
        // foreach requested package there has to be at most one version
        foreach(Entity *ent, request.requests)
        {
            PackageList pkgClause;
            foreach(Entity *other, entityMap_[request.name])
            {
                assert(ent->name == other->name);
                // the entity conflicts with all other versions
                if (ent->version != other->version || ent->allVersions())
                {
                    other->addToClause(pkgClause);
                }
            }
            uint32_t condition = addClause(pkgClause, out);
            PackageList pkgReason;
            ent->addToClause(pkgReason);
            sort_uniq_ptr(pkgReason);
            foreach(Package *pkg, pkgReason)
            {
                out << "conflict(\"" << string(pkg->name) << "\"," << pkg->version << "," << condition << ").\n";
            }
        }
    }
    // TODO: fix the blub conditions ...
    foreach(EntityList &list, entityMap_ | boost::adaptors::map_values)
    {
        foreach(Entity *ent, list)
        {
            Package *pkg = dynamic_cast<Package*>(ent);
            if(pkg)
            {
                bool addedUnsatRecom = false;
                foreach (Criterion &crit, criteria.criteria)
                {
                    if (crit.measurement == Criterion::UNSAT_RECOMMENDS)
                    {
                        // recommends(VP,D)
                        bool blub = true;
                        if (!addedUnsatRecom && !pkg->recommends.empty() && (addAll_ || blub))
                        {
                            typedef std::map<uint32_t, uint32_t> OccurMap;
                            OccurMap occur;
                            foreach(EntityList &clause, pkg->recommends)
                            {
                                PackageList pkgClause;
                                foreach(Entity *ent, clause) { ent->addToClause(pkgClause); }
                                uint32_t condition = addClause(pkgClause, out);
                                occur[condition]++;
                            }
                            foreach(OccurMap::value_type val, occur)
                            {
                                out << "recommends(\"" << string(pkg->name) << "\"," << pkg->version << "," << val.first << "," << val.second << ").\n";
                            }
                        }
                    }
                    else if (crit.measurement == Criterion::ALIGNED)
                    {
                        // TODO: add align attributes
                        bool blub = true;
                        // TODO: check to not print out an attribute twice ...
                        if (blub)
                        {
                            pkg->dumpAttr(this, out, crit.attrUid1);
                            pkg->dumpAttr(this, out, crit.attrUid2);
                        }
                    }
                    else if (crit.measurement == Criterion::SUM && pkg->visited)
                    {
                        // attributes(VP,K,V)
                        bool blub = true;
                        // TODO: check to not print out an attribute twice ...
                        if (blub)
                        {
                            pkg->dumpAttr(this, out, crit.attrUid1);
                        }
                    }
                }
            }
        }
    }
    // criteria
    int priotity = 0;
    foreach (Criterion &crit, criteria.criteria)
    {
        out << "criterion(" << (crit.optimize ? "maximize" : "minimize") << ",";
        switch (crit.selector)
        {
            case Criterion::SOLUTION: { out << "solution"; break;  }
            case Criterion::NEW:      { out << "new"; break;  }
            case Criterion::REMOVED:  { out << "removed"; break;  }
            case Criterion::CHANGED:  { out << "changed"; break;  }
            case Criterion::UP:       { out << "up"; break;  }
            case Criterion::DOWN:     { out << "down"; break;  }
        }
        out << ",";
        switch (crit.measurement)
        {
            case Criterion::COUNT:            { out << "count"; break;  }
            case Criterion::SUM:              { out << "sum(" << crit.attr1 << ")"; break;  }
            case Criterion::UNSAT_RECOMMENDS: { out << "unsat_recommends"; break;  }
            case Criterion::NOTUPTODATE:      { out << "notuptodate"; break;  }
            case Criterion::ALIGNED:          { out << "aligned(" << crit.attr1 << "," << crit.attr2 << ")"; break;  }
        }
        out << priotity++ << ").\n";
    }
}
