#pragma once

#include <type_traits>
#include <utility>
#include <string>
#include <fmt/format.h>


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
    std::enable_if_t<!ReflectTrait_<Mty_>::is_reflected_> operator()(Mty_ &member_, const char *name_) {
        if (!result_.empty()) {
            result_ += ' ';
        }
        result_ += fmt::format("{}={}", name_, member_);
    }

    template <class Mty_>
    std::enable_if_t<ReflectTrait_<Mty_>::is_reflected_> operator()(Mty_ &member_, const char *name_) {
        if (!result_.empty()) {
            result_ += ' ';
        }
        result_ += fmt::format("{}={}", name_, ::refl::to_typed_string(member_));
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
