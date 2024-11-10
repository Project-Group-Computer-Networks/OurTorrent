#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <type_traits>
#include <cstdint>

namespace bencoding
{
    struct string_subs
    {
    public:
        std::string str;
        std::string::const_iterator citer;
        string_subs(std::string input) : str{input}, citer{str.begin()} {}
        void refresh() { citer = str.begin(); }
    };

    class bencode_base
    {
    public:
        static const char delimiter_token = ':';
        static const char end_token = 'e';
        static const char integer_token = 'i';
        static const char list_token = 'l';
        static const char dict_token = 'd';

        bencode_base() = default;
        bencode_base(const bencode_base &) = default;
        bencode_base &operator=(const bencode_base &) = default;
        bencode_base(bencode_base &&other) noexcept = default;
        bencode_base &operator=(bencode_base &&other) noexcept = default;

        virtual ~bencode_base() = default;

        virtual std::string encode() const = 0;
        virtual void encode_n_dump(std::ostream &out) const = 0;
        virtual void decode(const std::string &in, std::string::const_iterator &start) = 0;
        virtual std::string get_as_str() const = 0;
    };

    class bencode_integer : public bencode_base
    {
    public:
        using value_type = int64_t;

        bencode_integer() = default;
        bencode_integer(value_type value) : integer_value_{value} {}

        bencode_integer &operator=(value_type value)
        {
            integer_value_ = value;
            return *this;
        }
        value_type get() const { return integer_value_; }

        std::string encode() const override
        {
            return std::string(1, integer_token) + std::to_string(integer_value_) + std::string(1, end_token);
        }
        void encode_n_dump(std::ostream &out) const override
        {
            out << integer_token << integer_value_ << end_token;
        }
        void decode(const std::string &in, std::string::const_iterator &start) override
        {
            start++; // Assuming *start == 'i'
            value_type value = 0;
            bool neg = false;
            if (*start == '-')
                neg = true, start++;

            while (start != in.end() && *start != end_token)
                value = value * 10 + (*start - '0'), start++;

            integer_value_ = neg ? -value : value;
            if (start != in.end())
                start++; // assuming it ends with 'e'
        }
        std::string get_as_str() const override
        {
            return std::to_string(integer_value_);
        }

    private:
        value_type integer_value_{0};
    };

    class bencode_string : public bencode_base
    {
    public:
        using string_type = std::string;
        using iterator = string_type::iterator;
        using const_iterator = string_type::const_iterator;
        using CharT = char;

        bencode_string() = default;
        bencode_string(string_type str) : str_{str} {}
        bencode_string(const CharT *cstr) : str_{cstr} {}

        iterator begin() { return str_.begin(); }
        const_iterator begin() const { return str_.begin(); }
        const_iterator cbegin() const { return str_.cbegin(); }
        iterator end() { return str_.end(); }
        const_iterator end() const { return str_.end(); }
        const_iterator cend() const { return str_.cend(); }

        bencode_string &operator=(string_type str)
        {
            str_ = str;
            return *this;
        }
        bencode_string &operator=(const CharT *cstr)
        {
            str_ = cstr;
            return *this;
        }
        size_t size() const { return str_.length(); }
        string_type get() const { return str_; }

        std::string encode() const override
        {
            return std::to_string(str_.length()) + std::string(1, delimiter_token) + str_;
        }
        void encode_n_dump(std::ostream &out) const override
        {
            out << str_.length() << delimiter_token << str_;
        }
        void decode(const std::string &in, std::string::const_iterator &start) override
        {
            size_t len = 0; // Assuming it starts with length
            while (start != in.end() && *start != delimiter_token)
                len = len * 10 + (*start - '0'), start++;

            start++;
            str_ = std::string(start, start + len);
            start += len;
        }
        std::string get_as_str() const override
        {
            return this->get();
        }

    private:
        string_type str_;
    };

    std::unique_ptr<bencode_base> make_value(const std::string &in, std::string::const_iterator &start);

    template <typename T>
    concept bencodeDerived = std::is_base_of<bencode_base, T>::value;

    class bencode_list : public bencode_base
    {
    public:
        using value_ptr_type = std::unique_ptr<bencode_base>;
        using container_type = std::vector<value_ptr_type>;
        using iterator = container_type::iterator;
        using const_iterator = container_type::const_iterator;

        bencode_list() = default;
        bencode_list(size_t size_) : bencode_list() { list_.resize(size_); }

        iterator begin() { return list_.begin(); }
        const_iterator begin() const { return list_.begin(); }
        const_iterator cbegin() const { return list_.cbegin(); }
        iterator end() { return list_.end(); }
        const_iterator end() const { return list_.end(); }
        const_iterator cend() const { return list_.cend(); }

        size_t size() const { return list_.size(); }
        value_ptr_type &operator[](std::size_t index) { return list_[index]; }

        template <bencodeDerived T, typename... Args>
        void push_back(Args &&...args)
        {
            list_.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
        }
        void pop_back()
        {
            if (!list_.empty())
                list_.pop_back();
        }

        std::string encode() const override
        {
            std::string enc_str(1, list_token);
            for (const value_ptr_type &ptr_ : list_)
                if (ptr_)
                    enc_str += (ptr_->encode());
            enc_str += end_token;
            return enc_str;
        }

        void encode_n_dump(std::ostream &out) const override
        {
            out << list_token;
            for (const value_ptr_type &ptr_ : list_)
                if (ptr_)
                    ptr_->encode_n_dump(out);
            out << end_token;
        }

        void decode(const std::string &in, std::string::const_iterator &start) override
        {
            list_.clear();
            start++; // assuming *start == 'l'
            while (start != in.end() && *start != end_token)
                list_.push_back(make_value(in, start));
            start++; // assuming *start == 'e'
        }

        std::string get_as_str() const override
        {
            return this->encode();
        }

    private:
        container_type list_;
    };

    class bencode_dict : public bencode_base
    {
    private:
        // custom comparator
        template <typename T_>
        struct lexicographical_compare
        {
            bool operator()(const T_ &lhs, const T_ &rhs) const
            {
                return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
            }
        };

    public:
        using key_type = bencode_string;
        using mapped_type = std::unique_ptr<bencode_base>;
        using value_type = std::pair<const key_type, mapped_type>;
        using comparator_type = lexicographical_compare<key_type>;
        using container_type = std::map<key_type, mapped_type, comparator_type>;
        using iterator = container_type::iterator;
        using const_iterator = container_type::const_iterator;

        bencode_dict() = default;

        iterator begin() { return dict_.begin(); }
        const_iterator begin() const { return dict_.begin(); }
        const_iterator cbegin() const { return dict_.cbegin(); }
        iterator end() { return dict_.end(); }
        const_iterator end() const { return dict_.end(); }
        const_iterator cend() const { return dict_.cend(); }

        std::size_t size() const { return dict_.size(); }
        mapped_type &operator[](const key_type &index) { return dict_[index]; }
        std::size_t erase(const key_type &key) { return dict_.erase(key); }
        iterator erase(iterator pos) { return dict_.erase(pos); }
        iterator find(const key_type &key) { return dict_.find(key); }

        std::pair<iterator, bool> insert(const key_type &key, mapped_type value)
        {
            return dict_.emplace(key, std::move(value));
        }
        template <bencodeDerived T, typename... Args>
        std::pair<iterator, bool> insert_or_assign(const key_type &key, Args &&...args)
        {
            mapped_type value_ptr = std::make_unique<T>(std::forward<Args>(args)...);
            return dict_.insert_or_assign(key, std::move(value_ptr));
        }
        // Rejected Idea :
        // overloading [] operator for assigning derived object
        // to later construct unique_ptr. (since too messy and unintuitive)

        std::string encode() const override
        {
            std::string enc_str(1, dict_token);
            for (const auto &[key_, ptr_] : dict_)
            {
                if (ptr_)
                {
                    enc_str += key_.encode();
                    enc_str += ptr_->encode();
                }
            }
            enc_str += end_token;
            return enc_str;
        }

        void encode_n_dump(std::ostream &out) const override
        {
            out << dict_token;
            for (const auto &[key_, ptr_] : dict_)
            {
                if (ptr_)
                {
                    key_.encode_n_dump(out);
                    ptr_->encode_n_dump(out);
                }
            }
            out << end_token;
        }

        void decode(const std::string &in, std::string::const_iterator &start) override
        {
            dict_.clear();
            start++; // assuming *start = 'd'
            while (start != in.end() && *start != end_token)
            {
                key_type key_;
                key_.decode(in, start);
                dict_[key_] = make_value(in, start);
            }
            start++; // assuming *start = 'e'
        }

        std::string get_as_str() const override
        {
            return this->encode();
        }

    private:
        container_type dict_;
    };

    std::unique_ptr<bencode_base> make_value(const std::string &in, std::string::const_iterator &start)
    {
        std::unique_ptr<bencode_base> ptr_;
        if (*start == bencode_base::integer_token)
            ptr_ = std::make_unique<bencode_integer>();
        else if (*start == bencode_base::list_token)
            ptr_ = std::make_unique<bencode_list>();
        else if (*start == bencode_base::dict_token)
            ptr_ = std::make_unique<bencode_dict>();
        else
            ptr_ = std::make_unique<bencode_string>();
        ptr_->decode(in, start);
        return ptr_;
    }
}