// {{{ MIT License

// Copyright 2017 Roland Kaminski

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

// }}}
//////////////////// Preamble ///////////////////////// {{{1

#include <cudf/dependency.hh>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm/unique.hpp>
#include <boost/range/algorithm/find.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <typeinfo>
#include <map>

//////////////////// Helper /////////////////////////// {{{1

namespace {
    template <class T>
    void sort_uniq_ptr(T &t) {
	using U = typename T::value_type;
        t.resize(boost::range::unique(boost::range::sort(t, [](U a, U b){ return *a < *b; }), [](U a, U b){ return *a == *b; }).size());
    }

    template <class T>
    void sort_uniq(T &t) {
        t.resize(boost::range::unique(boost::range::sort(t)).size());
    }

    struct CudfPackageRefFilter {
        CudfPackageRefFilter(const Cudf::PackageRef &ref) : ref(&ref) { }
        bool operator()(const Entity *entity) const {
            switch (ref->op) {
                case Cudf::PackageRef::EQ:
                    return (entity->version == ref->version || entity->allVersions()) && ref->version != 0;
                case Cudf::PackageRef::NEQ:
                    return  entity->version != ref->version || entity->allVersions();
                case Cudf::PackageRef::LE:
                    return (entity->version <= ref->version || entity->allVersions()) && ref->version != 0;
                case Cudf::PackageRef::GE:
                    return  entity->version >= ref->version || entity->allVersions();
            }
            return true;
        }
        const Cudf::PackageRef *ref;
    };

#define refRange(map, ref) ( map[ref.name] | boost::adaptors::filtered(CudfPackageRefFilter(ref)) )

    void unroll(Dependency::EntityMap &map, const Cudf::PkgList &clause, EntityList &list) {
        for (const Cudf::PackageRef &ref : clause) {
            boost::range::push_back(list, refRange(map, ref));
        }
        sort_uniq_ptr(list);
    }

    void unroll(Dependency::EntityMap &map, const Cudf::PkgFormula &formula, EntityFormula &list) {
        for (const Cudf::PkgList &clause : formula) {
            list.push_back(EntityList());
            unroll(map, clause, list.back());
        }
    }

    void unroll(Dependency::EntityMap &map, const Cudf::PkgList &formula, Dependency::RequestList &requests) {
        for (const Cudf::PackageRef &clause : formula) {
            requests.push_back(Request(clause.name));
            boost::range::push_back(requests.back().requests, refRange(map, clause));
            sort_uniq_ptr(requests.back().requests);
        }
    }

#undef refRange
}

//////////////////// Entity /////////////////////////// {{{1

Entity::Entity(uint32_t name, int32_t version, bool installed)
    : name(name)
    , version(version)
    , visited(false)
    , installed(installed)
    , remove_(false) { }

bool Entity::operator<(const Entity &ent) const {
    // NOTE: will sort the maximum version to the end
    //       see Feature::allVersions()
    if (version != ent.version) { return version < ent.version; }
    else if (name != ent.name)  { return name    < ent.name; }
    else                        { return typeid(this).before(typeid(&ent)); }
}

bool Entity::operator==(const Entity &ent) const {
    return
        typeid(this) == typeid(&ent) &&
        name         == ent.name &&
        version      == ent.version;
}

void Entity::remove(Dependency *dep) {
    if (!remove_) {
        remove_ = true;
        doRemove(dep);
    }
}

void Entity::add(Dependency *dep) {
    if (!visited) {
        visited = true;
        dep->add(this);
    }
}

bool Entity::allVersions() const {
    return version == std::numeric_limits<int32_t>::max();
}

Entity::~Entity() { }

//////////////////// Package ////////////////////////// {{{1

Package::Package(const Cudf::Package &pkg)
    : Entity(pkg.name, pkg.version, pkg.installed)
    , keep(pkg.keep)
    , intProps(pkg.intProps)
    , stringProps(pkg.stringProps)
    , optInstalled(false)
    , optGtMaxInstalled(false)
    , optLtMinInstalled(false)
    , optMaxVersion(false)
    , optInInstall(false)
    , optInUpgrade(false)
    , dfsVisited(false) { }

void Package::doRemove(Dependency *) { }

bool Package::satisfies(Criterion::Selector sel) {
    switch (sel) {
        case Criterion::SOLUTION:       { return !remove_; }
        case Criterion::CHANGED:        { return !remove_ || installed; }
        case Criterion::NEW:            { return !remove_ && !optInstalled; }
        case Criterion::REMOVED:        { return installed; }
        case Criterion::UP:             { return !remove_ && optGtMaxInstalled; }
        case Criterion::DOWN:           { return !remove_ && optLtMinInstalled; }
        case Criterion::INSTALLREQUEST: { return !remove_ && optInInstall; }
        case Criterion::UPGRADEREQUEST: { return !remove_ && optInUpgrade; }
        case Criterion::REQUEST:        { return !remove_ && (optInInstall || optInUpgrade); }
    }
    assert(false);
    return false;
}

Package::Relevant Package::relevant(bool maximize, Criterion::Selector sel) {
    switch (sel) {
        case Criterion::SOLUTION:       { return maximize                            ? RELEVANT_SELF : RELEVANT_NONE; }
        case Criterion::CHANGED:        { return (installed != maximize) && !remove_ ? RELEVANT_SELF : RELEVANT_NONE; }
        case Criterion::NEW:            { return maximize                            ? RELEVANT_SELF : RELEVANT_NONE; }
        case Criterion::REMOVED:        { return maximize                            ? RELEVANT_NONE : RELEVANT_EQUAL; }
        case Criterion::UP:             { return maximize                            ? RELEVANT_SELF : RELEVANT_NONE; }
        case Criterion::DOWN:           { return maximize                            ? RELEVANT_SELF : RELEVANT_NONE; }
        case Criterion::INSTALLREQUEST: { return maximize                            ? RELEVANT_SELF : RELEVANT_NONE; }
        case Criterion::UPGRADEREQUEST: { return maximize                            ? RELEVANT_SELF : RELEVANT_NONE; }
        case Criterion::REQUEST:        { return maximize                            ? RELEVANT_SELF : RELEVANT_NONE; }
    }
    assert(false);
    return RELEVANT_NONE;
}

unsigned Package::relevant(Criterion &crit) {
    if (!satisfies(crit.selector)) { return RELEVANT_NONE; }
    switch (crit.measurement) {
        case Criterion::COUNT:            { return                                                      relevant(crit.optimize, crit.selector); }
        case Criterion::NOTUPTODATE:      { return !optMaxVersion                                     ? relevant(crit.optimize, crit.selector) : RELEVANT_NONE; }
        case Criterion::ALIGNED:          { return crit.optAligned[getProp(crit.attrUid1)].size() > 1 ? relevant(crit.optimize, crit.selector) : RELEVANT_NONE; }
        case Criterion::UNSAT_RECOMMENDS: {
            unsigned rel =  !recommends.empty() ? relevant(crit.optimize, crit.selector) : RELEVANT_NONE;
            if (!crit.optimize) { rel = rel | RELEVANT_RECOMMENDED; }
            return rel;
        }
        case Criterion::SUM: {
            int attr = intProps[crit.attrUid1];
            return attr != 0 ? relevant((attr > 0) == crit.optimize, crit.selector) : RELEVANT_NONE;
        }
    }
    assert(false);
    return RELEVANT_NONE;
}

void Package::doAdd(Dependency *dep) {
    if (!dep->addAll()) {
        if (!remove_) {
            for (EntityList &clause : depends) {
                for (Entity *ent : clause) {
                    if (!ent->remove_) { ent->add(dep); }
                }
            }
        }
    }
}

uint32_t Package::getProp(uint32_t uid) const {
    IntPropMap::const_iterator it = intProps.find(uid);
    if (it != intProps.end()) { return it->second; }
    else {
        StringPropMap::const_iterator jt = stringProps.find(uid);
        if (jt != stringProps.end()) { return jt->second; }
    }
    return 0;
}

void Package::dumpAttrs(Dependency *dep, std::ostream &out) {
    // installed(VP)
    if (installed) {
        out << "installed(\"" << dep->string(name) << "\"," << version << ").\n";
    }
    // maxversion(VP)
    if (optMaxVersion) {
        out << "maxversion(\"" << dep->string(name) << "\"," << version << ").\n";
    }
    // additional attributes
    bool recom = false;
    std::set<uint32_t> attr;
    for (Criterion &crit : dep->criteria.criteria) {
        switch (crit.measurement) {
            case Criterion::UNSAT_RECOMMENDS: {
                if (dep->addAll() || satisfies(crit.selector)) { recom = true; }
                break;
            }
            case Criterion::ALIGNED: {
                if (dep->addAll() || satisfies(crit.selector)) {
                    attr.insert(crit.attrUid1);
                    attr.insert(crit.attrUid2);
                }
                break;
            }
            case Criterion::SUM: {
                if (dep->addAll() || satisfies(crit.selector)) {
                    attr.insert(crit.attrUid1);
                }
                break;
            }
            default: { break; }
        }
    }
    // recommends(VP,D)
    if (recom) {
        typedef std::map<uint32_t, uint32_t> OccurMap;
        OccurMap occur;
        for (EntityList &clause : recommends) {
            PackageList pkgClause;
            for (Entity *ent : clause) { ent->addToClause(pkgClause); }
            uint32_t condition = dep->addClause(pkgClause, out);
            occur[condition]++;
        }
        for (OccurMap::value_type val : occur) {
            out << "recommends(\"" << dep->string(name) << "\"," << version << "," << val.first << "," << val.second << ").\n";
        }
    }
    // attributes(VP,K,V)
    for (uint32_t uid : attr) {
        out << "attribute(\"" << dep->string(name) << "\"," << version << ",\"" << dep->string(uid) << "\",";
        IntPropMap::const_iterator it = intProps.find(uid);
        if (it != intProps.end()) { out << it->second; }
        else {
            StringPropMap::const_iterator jt = stringProps.find(uid);
            // Note: we do not care for the value at all
            if (jt != stringProps.end()) { out << jt->second; }
        }
        out << ").\n";
    }
}

void Package::dumpAsFacts(Dependency *dep, std::ostream &out) {
    // unit(VP)
    out << "unit(\"" << dep->string(name) << "\"," << version << "," << (remove_ ? "out" : "in") << ").\n";
    if (!remove_) {
        // satisfies(VP,D)
        // depends(VP,D)
        for (EntityList &clause : depends) {
            PackageList pkgClause;
            for (Entity *ent : clause) { ent->addToClause(pkgClause); }
            uint32_t condition = dep->addClause(pkgClause, out);
            out << "depends(\"" << dep->string(name) << "\"," << version << "," << condition << ").\n";
        }
        // conflicts(VP, D)
        if (!conflicts.empty()) {
            PackageList pkgClause;
            for (Entity *ent : conflicts) { ent->addToClause(pkgClause, this); }
            uint32_t condition = dep->addClause(pkgClause, out);
            out << "conflict(\"" << dep->string(name) << "\"," << version << "," << condition << ").\n";
        }
    }
}

void Package::addToClause(PackageList &clause, Package *self) {
    if (!remove_ && this != self) { clause.push_back(this); }
}

void Package::addConflictEdges(ConflictGraph &g) {
    if (!remove_) {
        PackageList clause;
        for (Entity *ent : conflicts) {
            ent->addToClause(clause, this);
        }
        g.addEdges(this, clause);
    }
}

//////////////////// Feature ////////////////////////// {{{1

Feature::Feature(const Cudf::PackageRef &ftr)
    : Entity(ftr.name, ftr.version == 0 ? std::numeric_limits<int32_t>::max() : ftr.version, false) { }

void Feature::doRemove(Dependency *dep) {
    for (Package *pkg : providedBy) { pkg->remove(dep); }
}

void Feature::doAdd(Dependency *dep) {
    for (Package *pkg : providedBy) { pkg->add(dep); }
}

void Feature::dumpAsFacts(Dependency *, std::ostream &) { }

size_t hash_value(const Feature &ftr) {
    size_t seed = 0;
    boost::hash_combine(seed, ftr.name);
    boost::hash_combine(seed, ftr.version);
    return seed;
}

void Feature::addToClause(PackageList &clause, Package *self) {
    if (!remove_) {
        for (Package *pkg : providedBy) {
            pkg->addToClause(clause, self);
        }
    }
}

void Feature::addConflictEdges(ConflictGraph &) {
    // nothing to do
}

//////////////////// Request ////////////////////

Request::Request(uint32_t name)
    : name(name) { }

void Request::add(Dependency *dep) {
    for (Entity *ent : requests) {
        if (!ent->remove_) { ent->add(dep); }
    }
}

//////////////////// Criteria ///////////////////////// {{{1

Criteria::Criteria() { }

void Criteria::init(Dependency *dep, CritVec &vec) {
    std::swap(criteria, vec);
    for (Criterion &crit : criteria) {
        if (!crit.attr1.empty()) {
            crit.attrUid1 = dep->index(crit.attr1);
            optProps.push_back(crit.attrUid1);
        }
        if (!crit.attr2.empty()) {
            crit.attrUid2 = dep->index(crit.attr2);
            optProps.push_back(crit.attrUid2);
        }
    }
    sort_uniq(optProps);
}

//////////////////// ConflictGraph //////////////////// {{{1

bool ConflictGraph::edgeSort(Package *a, Package *b) {
    // Note: prefer self-conflicts
    if (a->name != b->name) { return a->name < b->name; }
    return edges_[a].size() > edges_[b].size();
}

bool ConflictGraph::PkgCmp::operator()(Package *a, Package *b) const {
    if (a->name != b->name) { return a->name < b->name; }
    return a->version < b->version;
}

size_t ConflictGraph::PkgHash::operator()(Package *pkg) const {
    size_t seed = pkg->name;
    boost::hash_combine(seed, pkg->version);
    return seed;
}

void ConflictGraph::addEdges(Package *a, PackageList const &neighbors) {
    if (!neighbors.empty()) {
        Edges::mapped_type &out = edges_[a];
        out.insert(out.end(), neighbors.begin(), neighbors.end());
    }
}

void ConflictGraph::init(bool verbose) {
    Edges copy = edges_;
    for (Edges::value_type &out : copy) {
        for (Package *pkg : out.second) {
            edges_[pkg].push_back(out.first);
            edgeSet_.insert(std::make_pair(std::min(pkg, out.first),std::max(pkg, out.first)));
        }
    }
    for (Edges::value_type &out : edges_) {
        out.second.resize(boost::range::unique(boost::range::sort(out.second, PkgCmp())).size());
    }
    components_(verbose);
    cliques_(verbose);
}

void ConflictGraph::components_(bool verbose) {
    uint32_t min = 0, max = 0, sum = 0;
    for (Edges::value_type &out : edges_) {
        if (!out.first->dfsVisited) {
            components.push_back(PackageList());
            PackageList &component = components.back();
            out.first->dfsVisited = true;
            component.push_back(out.first);
            for (PackageList::size_type i = 0; i < component.size(); ++i) {
                for (Package *pkg : edges_[component[i]]) {
                    if (!pkg->dfsVisited) {
                        pkg->dfsVisited = true;
                        component.push_back(pkg);
                    }
                }
            }
            assert(component.size() > 1);
            sum+= component.size();
            if (min == 0 || component.size() < min) { min = component.size(); }
            if (component.size() > max) { max = component.size(); }
        }
    }
    if (verbose) {
        std::cerr <<     "components: " << components.size() << std::endl;
        if (!components.empty()) {
            std::cerr << "  average : " << double(sum) / components.size() << std::endl;
            std::cerr << "  min size: " << min << std::endl;
            std::cerr << "  max size: " << max << std::endl;
        }
    }
}

void ConflictGraph::cliques_(bool verbose) {
    uint32_t min = 0, max = 0, sum = 0;
    for (PackageList &component : components) {
        if (component.size() == 2) {
            cliques.push_back(component);
            sum += 2;
            min  = 2;
            if (2 > max) { max = 2; }
        }
        else {
            PackageList candidates = component, next;
            boost::sort(candidates, boost::bind(&ConflictGraph::edgeSort, this, _1, _2));
            // TODO: sort by out-going edges
            do {
                cliques.push_back(PackageList());
                PackageList &clique = cliques.back();
                for (Package *pkg : candidates) {
                    bool extendsClique = true;
                    for (Package *member : clique) {
                        extendsClique = edgeSet_.find(std::make_pair(std::min(pkg, member),std::max(pkg, member))) != edgeSet_.end();
                        if (!extendsClique) { break; }
                    }
                    if (extendsClique) { clique.push_back(pkg); }
                    else { next.push_back(pkg); }
                }
                if (clique.size() < 2) { cliques.pop_back(); }
                else {
                    sum+= clique.size();
                    if (min == 0 || clique.size() < min) { min = clique.size(); }
                    if (clique.size() > max) { max = clique.size(); }
                }
                candidates.clear();
                std::swap(candidates, next);
            }
            while (candidates.size() > 1);
        }
    }
    if (verbose) {
        std::cerr <<     "cliques   : " << cliques.size() << std::endl;
        if (!cliques.empty()) {
            std::cerr << "  average : " << double(sum) / cliques.size() << std::endl;
            std::cerr << "  min size: " << min << std::endl;
            std::cerr << "  max size: " << max << std::endl;
        }
    }
}

void ConflictGraph::dump(Dependency *dep, std::ostream &out) {
    uint32_t index = 0;
    for (PackageList &clique : cliques) {
        for (Package *pkg : clique) {
            out << "clique(" << index << ",\"" << dep->string(pkg->name) << "\"," << pkg->version << ").\n";
        }
        ++index;
    }
}

//////////////////// Dependency /////////////////////// {{{1

Dependency::Dependency(Criteria::CritVec &crits, bool addAll, bool verbose)
    : verbose_(verbose)
    , addAll_(addAll) {
    criteria.init(this, crits);
}

uint32_t Dependency::index(const std::string &s) {
    StringSet::iterator it = strings_.push_back(s).first;
    return it - strings_.begin();
}

uint32_t Dependency::index(const char *s) {
    return index(std::string(s));
}

const std::string &Dependency::string(uint32_t index) {
    return strings_.at(index);
}

void Dependency::init(const Cudf::Document &doc) {
    // first pass: add packages and features
    for  (const Cudf::Package &cudfPkg : doc.packages) {
        packages_.emplace_back(std::make_unique<Package>(cudfPkg));
        auto &pkg = packages_.back();
        entityMap_[pkg->name].push_back(pkg.get());
        for  (const Cudf::PackageRef &provided : cudfPkg.provides) {
            // NOTE: version might be zero here, which is than mapped to the maximum integer value
            std::pair<FeatureSet::iterator, bool> res = features_.insert(Feature(provided));
            Feature *ftr = const_cast<Feature*>(&*res.first);
            if (pkg->installed) { ftr->installed = true; }
            pkg->provides.push_back(ftr);
            ftr->providedBy.push_back(pkg.get());
            if (res.second) { entityMap_[ftr->name].push_back(ftr); }
        }
        sort_uniq_ptr(pkg->provides);
    }
    // second pass: roll out dependencies
    PackageSet::iterator current = packages_.begin();
    for  (const Cudf::Package &cudfPkg : doc.packages) {
        auto &pkg = *current++;
        unroll(entityMap_, cudfPkg.conflicts, pkg->conflicts);
        unroll(entityMap_, cudfPkg.depends, pkg->depends);
        unroll(entityMap_, cudfPkg.recommends, pkg->recommends);
    }
    unroll(entityMap_, doc.request.remove,  remove_);
    unroll(entityMap_, doc.request.install, install_);
    unroll(entityMap_, doc.request.upgrade, upgrade_);
}

void Dependency::add(Entity *ent) {
    closure_.push_back(ent);
}

bool Dependency::test_contains(std::string const &name, int32_t version) {
    for  (Entity *ent : closure_) {
        Package *pkg = dynamic_cast<Package*>(ent);
        if (pkg && string(pkg->name) == name && pkg->version == version) { return true; }
    }
    return false;
}

void Dependency::rewriteRequests() {
    for  (Entity *ent : remove_) { ent->remove(this); }
    for  (Request &request : upgrade_) {
        for  (Entity *ent : request.requests) { ent->visited = true; }
        int32_t removeVersion = 0;
        for  (Entity *ent : entityMap_[request.name]) {
            // if some version is installed then a >= version must be installed
            if (ent->installed && removeVersion < ent->version) { removeVersion = ent->version; }
        }
        for  (Entity *ent : entityMap_[request.name]) {
            // no unrequested or smaller version may be installed
            if (!ent->visited || (removeVersion != 0 && (ent->version < removeVersion || ent->allVersions()))) {
                ent->remove(this);
            }
        }
        for  (Entity *ent : request.requests) { ent->visited = false; }
    }
    // rewrite keep flags into installs
    for  (auto &pkg : packages_) {
        if (pkg->installed) {
            switch(pkg->keep) {
                case Cudf::Package::FEATURE: {
                    for  (Feature *ftr : pkg->provides) {
                        install_.push_back(Request(ftr->name));
                        install_.back().requests.push_back(ftr);
                    }
                    break;
                }
                case Cudf::Package::VERSION: {
                    install_.push_back(Request(pkg->name));
                    install_.back().requests.push_back(pkg.get());
                    break;
                }
                case Cudf::Package::PACKAGE: {
                    install_.push_back(Request(pkg->name));
                    for  (Entity *ent : entityMap_[pkg->name]) {
                        if (dynamic_cast<Package*>(ent)) {
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

void Dependency::initClosure() {
    for  (Request &request : upgrade_) {
        request.add(this);
        for  (Entity *ent : request.requests) {
            Package *pkg = dynamic_cast<Package*>(ent);
            if (pkg) { pkg->optInUpgrade = true; }
        }
    }
    for  (Request &request : install_) {
        request.add(this);
        for  (Entity *ent : request.requests) {
            Package *pkg = dynamic_cast<Package*>(ent);
            if (pkg) { pkg->optInInstall = true; }
        }
    }
    // intialize opt... members to make satisfies calls constant time
    for  (EntityList &list : entityMap_ | boost::adaptors::map_values) {
        bool installed        = false;
        Package *max          = 0;
        Package *minInstalled = 0;
        Package *maxInstalled = 0;
        for  (Entity *ent : list) {
            Package *pkg = dynamic_cast<Package*>(ent);
            if (pkg) {
                if (!max || max->version < pkg->version) { max = pkg; }
                if (pkg->installed) { installed = true; }
                if (pkg->installed) {
                    if (!minInstalled || minInstalled->version > pkg->version) { minInstalled = pkg; }
                    if (!maxInstalled || maxInstalled->version < pkg->version) { maxInstalled = pkg; }
                }
            }
        }
        for  (Entity *ent : list) {
            Package *pkg = dynamic_cast<Package*>(ent);
            if (pkg) {
                pkg->optMaxVersion     = pkg == max;
                pkg->optInstalled      = installed;
                pkg->optLtMinInstalled = minInstalled && pkg->version < minInstalled->version;
                pkg->optGtMaxInstalled = maxInstalled && pkg->version > maxInstalled->version;
                for  (Criterion &crit : criteria.criteria) {
                    if (crit.measurement == Criterion::ALIGNED) {
                        // NOTE: a pair<uint32_t,bool> as value would be sufficient
                        crit.optAligned[pkg->getProp(crit.attrUid1)].insert(pkg->getProp(crit.attrUid2));
                    }
                }
            }
        }
    }
    for  (EntityList &list : entityMap_ | boost::adaptors::map_values) {
        for  (Entity *ent : list) {
            for  (Criterion &crit : criteria.criteria) {
                Package *pkg = dynamic_cast<Package*>(ent);
                if (pkg) {
                    unsigned rel = pkg->relevant(crit);
                    if (rel & Package::RELEVANT_SELF) {
                        pkg->add(this);
                    }
                    if (rel & Package::RELEVANT_RECOMMENDED) {
                        for  (EntityList &clause : pkg->recommends) {
                            for  (Entity *ent : clause) {
                                if (!ent->remove_) { ent->add(this); }
                            }
                        }
                    }
                    if (rel & Package::RELEVANT_EQUAL) {
                        for  (Entity *other : entityMap_[pkg->name]) {
                            if (!other->remove_) { other->add(this); }
                        }
                    }
                }
            }
        }
    }
}

void Dependency::closure() {
    rewriteRequests();
    if (addAll_) {
        for  (EntityList &list : entityMap_ | boost::adaptors::map_values) {
            Package *max = 0;
            for  (Entity *ent : list) {
                ent->add(this);
                Package *pkg = dynamic_cast<Package*>(ent);
                if (pkg) {
                    if (!max || max->version < pkg->version) { max = pkg; }
                }
            }
            for  (Entity *ent : list) {
                Package *pkg = dynamic_cast<Package*>(ent);
                if (pkg) {
                    pkg->optMaxVersion = pkg == max;
                }
            }
        }
    }
    else { initClosure(); }
    for(PackageList::size_type i = 0; i < closure_.size(); i++) { closure_[i]->doAdd(this); }
    if (verbose_) {
        std::cerr << "sizes: " << std::endl;
        std::cerr << "  features: " << features_.size() << std::endl;
        std::cerr << "  packages: " << packages_.size() << std::endl;
        std::cerr << "  closure:  " << closure_.size() << std::endl;
    }
}

uint32_t Dependency::addClause(PackageList &clause, std::ostream &out) {
    sort_uniq(clause);
    std::pair<ClauseMap::iterator,bool> res = clauses_.insert(ClauseMap::value_type(clause, 0));
    if (res.second) {
        res.first->second = clauses_.size();
        for  (Package *pkg : clause) {
            out << "satisfies(\"" << string(pkg->name) << "\"," << pkg->version << "," << res.first->second << ").\n";
        }
    }
    return res.first->second;
}

bool Dependency::addAll() const {
    return addAll_;
}

void Dependency::conflicts() {
    for  (Entity *ent : closure_) {
        ent->addConflictEdges(conflictGraph_);
    }
    conflictGraph_.init(verbose_);
}

void Dependency::dumpAsFacts(std::ostream &out) {
    bool installrequest = false;
    bool upgraderequest = false;
    for  (Criterion &crit : criteria.criteria) {
        if (crit.selector == Criterion::INSTALLREQUEST || crit.selector == Criterion::REQUEST) { installrequest = true; }
        if (crit.selector == Criterion::UPGRADEREQUEST || crit.selector == Criterion::REQUEST) { upgraderequest = true; }
    }
    for  (auto &pkg : packages_) { pkg->dumpAttrs(this, out); }
    for  (Entity *ent : closure_) { ent->dumpAsFacts(this, out); }
    // requests according to install request
    for  (Request &request : install_) {
        PackageList pkgClause;
        for  (Entity *ent : request.requests) {
            ent->addToClause(pkgClause);
            Package *pkg = dynamic_cast<Package*>(ent);
            if (installrequest && pkg) {
                out << "installrequest(\"" << string(pkg->name) << "\"," << pkg->version << ").\n";
            }
        }
        uint32_t condition = addClause(pkgClause, out);
        out << "request(" << condition << ").\n";
    }
    // requests/conflicts according to upgrade request
    for  (Request &request : upgrade_) {
        PackageList pkgClause;
        for  (Entity *ent : request.requests) {
            ent->addToClause(pkgClause);
            Package *pkg = dynamic_cast<Package*>(ent);
            if (upgraderequest && pkg) {
                out << "upgraderequest(\"" << string(pkg->name) << "\"," << pkg->version << ").\n";
            }
        }
        uint32_t condition = addClause(pkgClause, out);
        out << "request(" << condition << ").\n";
        // foreach requested package there has to be at most one version
        for  (Entity *ent : request.requests) {
            PackageList pkgClause;
            for  (Entity *other : entityMap_[request.name]) {
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
            for  (Package *pkg : pkgReason) {
                out << "conflict(\"" << string(pkg->name) << "\"," << pkg->version << "," << condition << ").\n";
            }
        }
    }
    conflictGraph_.dump(this, out);
    // criteria
    int priotity = criteria.criteria.size();
    for  (Criterion &crit : criteria.criteria) {
        out << "criterion(" << (crit.optimize ? "maximize" : "minimize") << ",";
        switch (crit.selector) {
            case Criterion::SOLUTION:       { out << "solution"; break;  }
            case Criterion::NEW:            { out << "new"; break;  }
            case Criterion::REMOVED:        { out << "removed"; break;  }
            case Criterion::CHANGED:        { out << "changed"; break;  }
            case Criterion::UP:             { out << "up"; break;  }
            case Criterion::DOWN:           { out << "down"; break;  }
            case Criterion::INSTALLREQUEST: { out << "installrequest"; break;  }
            case Criterion::UPGRADEREQUEST: { out << "upgraderequest"; break;  }
            case Criterion::REQUEST:        { out << "request"; break;  }
        }
        out << ",";
        switch (crit.measurement) {
            case Criterion::COUNT:            { out << "count"; break;  }
            case Criterion::SUM:              { out << "sum(\"" << crit.attr1 << "\")"; break;  }
            case Criterion::UNSAT_RECOMMENDS: { out << "unsat_recommends"; break;  }
            case Criterion::NOTUPTODATE:      { out << "notuptodate"; break;  }
            case Criterion::ALIGNED:          { out << "aligned(\"" << crit.attr1 << "\",\"" << crit.attr2 << "\")"; break;  }
        }
        out << "," << priotity-- << ").\n";
    }
}

