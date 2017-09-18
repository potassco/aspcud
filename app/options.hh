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
//////////////////// Preamble /////////////////////////////////// {{{1

#pragma once

#include <map>
#include <unordered_map>
#include <memory>
#include <string>
#include <cstring>
#include <cassert>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iterator>

//////////////////// Options /////////////////////////////////// {{{1
class OptionsException : public std::runtime_error {
public:
    explicit OptionsException(char const *msg) : std::runtime_error(msg) { }
};

class AbstractOption {
public:
    AbstractOption(int limit) : limit_{limit}, assigned_{0} { }
    virtual bool has_argument() const = 0;
    virtual void parse(char const *value) = 0;
    virtual ~AbstractOption() { }
    bool check_limit() {
        ++assigned_;
        return limit_ <= 0 || assigned_ <= limit_;
    }
    int assigned() const {
        return assigned_;
    }
private:
    int limit_;
    int assigned_;
};

class OptionFlag : public AbstractOption {
public:
    OptionFlag(bool &target, int limit) : AbstractOption{limit}, target_{target} { }
    bool has_argument() const override {
        return false;
    }
    void parse(char const *) override {
        target_ = true;
    }
private:
    bool &target_;
};

template <class T>
std::string option_to_string(T const &target, unsigned) {
    std::ostringstream oss;
    oss << target;
    return oss.str();
}

template <class T, class A>
std::string option_to_string(std::vector<T, A> const &, int) {
    return "";
}

template <class T>
void option_from_string(char const *value, T &target, unsigned) {
    std::istringstream iss{value};
    iss >> target;
}

template <class T, class A>
void option_from_string(char const *value, std::vector<T,A> &target, int) {
    target.emplace_back();
    std::istringstream iss{value};
    iss >> target.back();
}

template <class T>
void option_from_string(char const *value, T &target, typename std::enable_if<std::is_signed<T>::value && std::is_integral<T>::value, int>::type) {
    try {
        size_t end;
        auto ret = std::stoll(value, &end, 10);
        target = ret;
        if (end == strlen(value) && ret == target) { return; }
    }
    catch (...) { }
    throw std::runtime_error("invalid number");
}

template <class T>
void option_from_string(char const *value, T &target, typename std::enable_if<std::is_unsigned<T>::value && std::is_integral<T>::value, int>::type) {
    try {
        size_t end;
        auto ret = std::stoull(value, &end, 10);
        target = ret;
        if (end == strlen(value) && ret == target) { return; }
    }
    catch (...) { }
    throw std::runtime_error("invalid number");
}

template <class T>
class OptionParse : public AbstractOption {
public:
    OptionParse(T &target, int limit) : AbstractOption{limit}, target_{target} { }
    bool has_argument() const override {
        return true;
    }
    void parse(char const *value) override {
        option_from_string(value, target_, 0);
    }
private:
    T &target_;
};

class Options {
public:
    Options(char const *positional = "", size_t align_column = 30, size_t max_column = 79, bool force_positional_last = false)
    : positional_{positional}
    , align_column_{align_column}
    , max_column_{max_column}
    , force_positional_last_{force_positional_last} { }
    void group(char const *caption, char const *suffix = ":", int align_column = -1) {
        description_ += "\n";
        description_ += caption;
        description_ += suffix;
        description_ += "\n";
        if (align_column >= 0) {
            align_column_ = align_column;
        }
    }
    void add(bool &target, char const *name, char const *desc, int limit = 1, bool hidden = false) {
        add_(std::make_unique<OptionFlag>(target, limit), name, desc, nullptr, nullptr, 0, hidden);
    }
    template <class T>
    void add(T &target, char const *name, char const *desc, char const *arg = "arg", int limit = 1, int minimum = 0, bool hidden = false) {
        add(target, name, desc, option_to_string(target, 0).c_str(), arg, limit, minimum, hidden);
    }
    template <class T>
    void add(T &target, char const *name, char const *desc, char const *def, char const *arg = "arg", int limit = 1, int minimum = 0, bool hidden = false) {
        add_(std::make_unique<OptionParse<T>>(target, limit), name, desc, arg, def, minimum, hidden);
    }
    char const *description() const {
        return description_.c_str();
    }
    void parse(int argc, char **argv) {
        for (auto it = argv + 1, ie = argv + argc; it != ie; ++it) {
            auto arg = *it;
            auto len = std::strlen(arg);
            if (std::strcmp("--", arg) == 0) {
                throw std::runtime_error("unsupported argument --");
            }
            else if (std::strncmp("--", arg, 2) == 0) {
                arg += 2;
                len -= 2;
                auto pos = std::find(arg, arg + len, '=');
                std::string name{arg, pos};
                auto je = long_options_.end(), kt = je;
                for (auto jt = long_options_.lower_bound(name); jt != je; ++jt) {
                    if (std::strncmp(jt->first.c_str(), name.c_str(), len) == 0) {
                        if (kt != je) {
                            fail_("ambigious option --", name, ": could be --", kt->first, " or --", jt->first);
                        }
                        else { kt = jt; }
                    }
                    else { break; }
                }
                if (kt == long_options_.end()) {
                    fail_("unknown option --", name);
                }
                it = parse_(*kt->second, name.c_str(), pos, arg + len, it, ie, 1, "option", "--");
            }
            else if (std::strncmp("-", arg, 1) == 0 && arg[1] != '\0') {
                arg += 1;
                len -= 1;
                do {
                    std::string name{*arg};
                    auto jt = short_options_.find(name);
                    if (jt == short_options_.end()) {
                        fail_("unknown option -", name);
                    }
                    auto value = arg + 1;
                    size_t vlen;
                    if (jt->second->has_argument()) {
                        vlen = len - 1;
                        len = 0;
                    }
                    else {
                        vlen = 0;
                        ++arg;
                        --len;
                    }
                    it = parse_(*jt->second, name.c_str(), value, value + vlen, it, ie, 0, "option", "-");

                }
                while (len > 0);
            }
            else if (!positional_.empty()) {
                do {
                    auto jt = long_options_.find(positional_);
                    if (jt == long_options_.end()) {
                        fail_("unknown option --", positional_);
                    }
                    it = parse_(*jt->second, positional_.c_str(), arg, arg, it - 1, ie, 0, "argument", "");
                }
                while (force_positional_last_ && (++it, it != ie));
                if (force_positional_last_) { break; }
            }
            else {
                fail_("unexpected positional argument ", arg);
            }
        }
        for (auto &option : required_options_) {
            if (std::get<2>(option)->assigned() < std::get<1>(option)) {
                if (std::get<0>(option) == positional_) {
                    fail_("required argument ", std::get<0>(option), " is missing");
                }
                else {
                    fail_("required option --", std::get<0>(option), " is missing");
                }
            }
        }
    }

    int assigned(char const *option) {
        auto it = long_options_.find(option);
        if (it != long_options_.end()) {
            return it->second->assigned();
        }
        auto jt = short_options_.find(option);
        if (jt != short_options_.end()) {
            return jt->second->assigned();
        }
        return 0;
    }

private:

    template <class... T>
    void fail_(T&&... args) {
        std::ostringstream oss;
        // C++17 has fold expressions for this...
        using expand_variadic_pack = int[];
        (void)expand_variadic_pack{0, ((oss << std::forward<T>(args)), void(), 0)... };
        throw std::runtime_error(oss.str());
    }

    char **parse_(AbstractOption &option, char const *name, char const *jt, char const *je, char **it, char **ie, int skip, char const *what, char const *pre) {
        if (!option.check_limit()) {
            fail_("too many occurrences of ", what, " ", pre, name);
        }
        std::string value;
        if (jt != je) {
            if (!option.has_argument()) {
                fail_("unexpeced argument for option ", pre, name);
            }
            value.assign(jt + skip, je);
        }
        else if (option.has_argument()) {
            ++it;
            if (it == ie) {
                fail_("missing argument for option ", pre, name);
            }
            value = *it;
        }
        try {
            option.parse(value.c_str());
        }
        catch (std::exception const &e) {
            fail_("error parsing ", what, " ", pre, name, ": ", e.what());
        }
        return it;
    }
    void add_(std::unique_ptr<AbstractOption> &&option, char const *name, char const *desc, char const *arg, char const *def, int minimum, bool hidden) {
        auto len = std::strlen(name);
        assert(len > 0);
        std::string sname, lname;
        if (len == 1) {
            sname = name;
        }
        else {
            if (name[1] == ',') {
                sname.push_back(name[0]);
                name+= 2;
            }
            lname = name;
        }
        if (!lname.empty()) {
            long_options_.emplace(lname, option.get());
            if (minimum > 0) {
                required_options_.emplace_back(lname, minimum, option.get());
                minimum = 0;
            }
        }
        if (!sname.empty()) {
            short_options_.emplace(sname, option.get());
            if (minimum > 0) {
                required_options_.emplace_back(sname, minimum, option.get());
                minimum = 0;
            }
        }
        options_.emplace_back(std::move(option));
        if (!hidden) {
            auto len = description_.size();
            if (!sname.empty()) {
                description_ += "  -";
                description_ += sname;
                if (!lname.empty()) {
                    description_ += " [ --";
                    description_ += lname;
                    description_ += " ]";
                }
            }
            else {
                description_ += "  --";
                description_ += lname;
            }
            if (arg) {
                description_ += " ";
                description_ += arg;
                description_ += " (=";
                description_ += def;
                description_ += ")";
            }
            len = description_.size() - len + 3;
            if (std::strchr(desc, '\n') == nullptr && len <= align_column_ && len + strlen(desc) < max_column_) {
                for (; len < align_column_; ++len) {
                    description_ += ' ';
                }
                description_ += " : ";
                description_ += desc;
                description_ += "\n";
            }
            else {
                description_ += '\n';
                auto it = desc, ie = it + std::strlen(desc);
                while (it != ie) {
                    auto jt = std::find(it, ie, '\n');
                    description_ +=  "    ";
                    std::copy(it, jt, std::back_inserter(description_));
                    description_ += "\n";
                    it = jt;
                    if (it != ie) { ++it; }
                }
            }
        }
    }
private:
    std::map<std::string, AbstractOption*> long_options_;
    std::unordered_map<std::string, AbstractOption*> short_options_;
    std::vector<std::tuple<std::string, int, AbstractOption*>> required_options_;
    std::vector<std::unique_ptr<AbstractOption>> options_;
    std::string description_;
    std::string positional_;
    size_t align_column_;
    size_t max_column_;
    bool force_positional_last_;
};
