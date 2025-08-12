#pragma once

#include <type_traits>
#include <utility>
#include <string>
#include <string_view>
#include <cctype>
#include <fmt/format.h>
#include <magic_enum/magic_enum.hpp>


namespace refl
{

template <class Tp_>
struct ReflectTrait_
{
    static constexpr bool is_reflected_ = false;
    static constexpr const char class_name_[] = "";

    template <class Ty_, class Fn_>
    static constexpr void foreach_member_(Ty_ &&object_, Fn_ &&body_) {
        static_assert(((void)object_, false), "this type has not been registered for reflect");
    }
};

#define REFLECT_BEGIN(type_) \
template <> \
struct refl::ReflectTrait_<type_> \
{ \
    REFLECT_BEGIN_(type_)

#define REFLECT_BEGIN_(type_) \
    static constexpr bool is_reflected_ = true; \
    static constexpr const char class_name_[] = #type_; \
    \
    template <class Ty_, class Fn_> \
    static constexpr void foreach_member_(Ty_ &&object_, Fn_ &&fn_) {

#define REFLECT_MEMBER(member_) \
        fn_(object_.member_, #member_);

#define REFLECT_END_() \
    }

#define REFLECT_END() \
    REFLECT_END_() \
};


template <class Ty_>
constexpr const char *class_name()
{
    static_assert(ReflectTrait_<std::remove_cvref_t<Ty_>>::is_reflected_, "this type has not been registered for reflect");
    return ReflectTrait_<std::remove_cvref_t<Ty_>>::class_name_;
}

template <class Ty_, class Fn_>
constexpr void foreach(Ty_ &&object_, Fn_ &&fn_)
{
    static_assert(ReflectTrait_<std::remove_cvref_t<Ty_>>::is_reflected_, "this type has not been registered for reflect");
    ReflectTrait_<std::remove_cvref_t<Ty_>>::foreach_member_(std::forward<Ty_>(object_), std::forward<Fn_>(fn_));
}

template <class Ty_>
inline std::string to_string(Ty_ &&object_);

template <class Ty_>
inline std::string to_typed_string(Ty_ &&object_);

struct ToStringFn_
{
    std::string result_;

    template <class Mty_>
    std::enable_if_t<std::is_same_v<std::remove_cvref_t<Mty_>, unsigned char> || std::is_same_v<std::remove_cvref_t<Mty_>, char> || std::is_same_v<std::remove_cvref_t<Mty_>, signed char>> invoke_(Mty_ &member_, const char *name_, std::false_type, int) {
        result_ += fmt::format("{}=", name_);
        uint8_t c = static_cast<uint8_t>(member_);
        if (c <= 0x7F && std::isprint(c)) {
            result_ += fmt::format("{}(`{}`)", static_cast<uint32_t>(c), static_cast<char>(c));
        } else {
            result_ += fmt::format("{}", static_cast<uint32_t>(c));
        }
    }

    template <class Mty_>
    std::enable_if_t<std::is_same_v<std::remove_cvref_t<Mty_>, std::string> || std::is_same_v<std::remove_cvref_t<Mty_>, std::string_view> || std::is_same_v<std::decay_t<Mty_>, char *> || std::is_same_v<std::decay_t<Mty_>, const char *>> invoke_(Mty_ &member_, const char *name_, std::false_type, int) {
        result_ += fmt::format("{}=`{}`", name_, member_);
    }

    template <class Mty_>
    std::enable_if_t<std::is_enum_v<std::remove_cvref_t<Mty_>>> invoke_(Mty_ &member_, const char *name_, std::false_type, int) {
        result_ += fmt::format("{}=", name_);
        uint8_t c = static_cast<uint8_t>(member_);
        if (magic_enum::enum_contains(member_)) {
            result_ += fmt::format("{}({})", member_, magic_enum::enum_name(member_));
        } else {
            result_ += fmt::format("{}", member_);
        }
    }

    template <class Mty_>
    void invoke_(Mty_ &member_, const char *name_, std::false_type, ...) {
        result_ += fmt::format("{}={}", name_, member_);
    }

    template <class Mty_>
    void invoke_(Mty_ &member_, const char *name_, std::true_type, ...) {
        result_ += fmt::format("{}={}", name_, ::refl::to_typed_string(member_));
    }

    template <class Mty_>
    void operator()(Mty_ &member_, const char *name_) {
        if (!result_.empty()) {
            result_ += ' ';
        }
        return invoke_(member_, name_, std::bool_constant<ReflectTrait_<std::remove_cvref_t<Mty_>>::is_reflected_>{}, 0);
    }
};

template <class Ty_>
std::string to_string(Ty_ &&object_)
{
    static_assert(ReflectTrait_<std::remove_cvref_t<Ty_>>::is_reflected_, "this type has not been registered for reflect");
    ToStringFn_ fn_;
    ReflectTrait_<std::remove_cvref_t<Ty_>>::foreach_member_(object_, fn_);
    return std::move(fn_.result_);
}

template <class Ty_>
std::string to_typed_string(Ty_ &&object_)
{
    static_assert(ReflectTrait_<std::remove_cvref_t<Ty_>>::is_reflected_, "this type has not been registered for reflect");
    ToStringFn_ fn_;
    fn_.result_ = fmt::format("[{}", ReflectTrait_<std::remove_cvref_t<Ty_>>::class_name_);
    ReflectTrait_<std::remove_cvref_t<Ty_>>::foreach_member_(object_, fn_);
    fn_.result_ += ']';
    return std::move(fn_.result_);
}

}
