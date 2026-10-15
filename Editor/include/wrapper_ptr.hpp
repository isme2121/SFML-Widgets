#pragma once
#include "editor_accessors.hpp"
#if defined(WIDGETS_HPP) && defined(BUILD_EDITOR_GUI)

/*
* Contains: types used in WidgetEditor class
*
* WidgetWrapper ==> wrapper for derived widget instance, contains all accessor data
*
* wrapper_ptr ==> wrapper for WidgetWrapper wrapper so that WidgetEditor avoids templating
*/
namespace sfml_widgets
{
///////////////////////////////////////////////////////////////////
/// @brief Wrapper for widget for setters/getters accessability
///
/// @tparam WidgetT must be a derived widget type
///
///////////////////////////////////////////////////////////////////
template<typename WidgetT>
class WidgetWrapper
{
    static_assert(std::is_base_of_v<Widget, WidgetT>, "Type is not a widget.");
    static_assert(!std::is_abstract_v<WidgetT>, "Type is not an abstract.");
    static_assert(is_widget_type_v<WidgetT>, "Template type does not match any supported widget derived type.");

public:
    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs a sfml widget with a wrapper
    ///
    ///////////////////////////////////////////////////////////////////
    template<typename... Ts, typename = std::enable_if_t<std::is_constructible_v<WidgetT, Ts...>>>
    explicit WidgetWrapper(Ts&&... args) : m_widget(new WidgetT(std::forward<Ts>(args)...)) , m_isManaged(false)
    {
        // link all getters/setters for this widget
        BaseAccessorsLink<WidgetT>::link(m_lookup);
        AccessorsLink<WidgetT>::link(m_lookup);
        updateValues();
    }

    explicit WidgetWrapper(WidgetT* w) : m_widget(w), m_isManaged(true)
    {
        // link all getters/setters for this widget
        BaseAccessorsLink<WidgetT>::link(m_lookup);
        AccessorsLink<WidgetT>::link(m_lookup);
        updateValues();
    }

    ~WidgetWrapper()
    {
        if(!m_isManaged)
        {
            delete m_widget;
            m_widget = nullptr;
        }
    }

    WidgetWrapper(const WidgetWrapper&) = delete;
    WidgetWrapper& operator=(const WidgetWrapper&) = delete;
    WidgetWrapper(WidgetWrapper&&) = delete;
    WidgetWrapper& operator=(WidgetWrapper&&) = delete;

    WidgetT& widget()
    {
        return *m_widget;
    }

    const WidgetT& widget() const
    {
        return *m_widget;
    }

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns value relative to key, otherwise `nullopt` if
    /// key is not found
    ///
    ///////////////////////////////////////////////////////////////////
    std::optional<Accessed_t> get(const std::string& key) const
    {
        auto it = std::find(m_lookup.begin(), m_lookup.end(), key);
        if(it == m_lookup.end() || !it->getter) return std::nullopt;
        return it->getter(static_cast<void*>(m_widget));
    }

    ///////////////////////////////////////////////////////////////////
    /// @brief Calls the proper setter based on `key` string, `v` must
    /// match the proper type of the setter
    ///
    ///////////////////////////////////////////////////////////////////
    bool set(const std::string& key, Accessed_t v)
    {
        auto it = std::find(m_lookup.begin(), m_lookup.end(), key);
        if(it != m_lookup.end() && it->setter) return it->setter(static_cast<void*>(m_widget), v);
        return false;
    }

    ///////////////////////////////////////////////////////////////////
    /// @brief Calls the proper setter based on `key` string, `v` must
    /// match the proper type of the setter
    ///
    ///////////////////////////////////////////////////////////////////
    bool reset(const std::string& key)
    {
        auto it = std::find(m_lookup.begin(), m_lookup.end(), key);
        if(it != m_lookup.end() && it->resetter) return it->resetter(static_cast<void*>(m_widget));
        return false;
    }

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns all the current getter results with their pair keys
    ///
    ///////////////////////////////////////////////////////////////////
    std::unordered_map<std::string, Accessed_t> getValues() const
    {
        std::unordered_map<std::string, Accessed_t> tmp;
        for(const auto& accessor_val : m_lookup)
            if(accessor_val.getter) tmp.emplace(accessor_val.key, accessor_val.getter(static_cast<void*>(m_widget)));
        return tmp;
    }

    ///////////////////////////////////////////////////////////////////
    /// @brief Fills data vector with row information for each key
    ///
    ///////////////////////////////////////////////////////////////////
    LookupVec_t* fetchData()
    {
        return &m_lookup;
    }

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the value attribute of the current data vector
    ///
    ///////////////////////////////////////////////////////////////////
    void updateValues()
    {
        for(Accessor_ptr& element : m_lookup)
        {
            if(!element.getter) continue;
            element.attrib.val = element.getter(static_cast<void*>(m_widget));
        }
    }

private:
    WidgetT* m_widget;                  // main widget
    LookupVec_t m_lookup;               // accessor lookup
    bool m_isManaged{false};            // true = widget memory is managed externally, false = widget memory is managed by class
};

#ifdef WRAPPER_DEFINED

///////////////////////////////////////////////////////////////////
/// @brief `WidgetWrapper` wrapper for `WidgetEditor` class, provides
/// access to `WidgetWrapper` methods under a non-templated scope.
///
/// construct with `make_wrapper`
///
///////////////////////////////////////////////////////////////////
class wrapper_ptr
{
    friend class WidgetEditor;
    using GetFn_t = std::optional<Accessed_t> (*)(void*, const std::string&);
    using SetFn_t = bool (*)(void*, const std::string&, Accessed_t);
    using ResetFn_t = bool (*)(void*, const std::string&);
    using FetchFn_t = LookupVec_t* (*)(void*);
    using UpdateFn_t = void (*)(void*);
    using GetValsFn_t = std::unordered_map<std::string, Accessed_t> (*)(void*);
    using DeleterFn_t = void (*)(void*);

public:
    wrapper_ptr() = default;

    wrapper_ptr(void* w, GetFn_t getter, SetFn_t setter, ResetFn_t resetter, FetchFn_t fetch_data, UpdateFn_t update_data, GetValsFn_t get_all, DeleterFn_t del) : m_wrappedWidget(w),
    info_Get(getter),
    info_Set(setter),
    info_Reset(resetter),
    info_Fetch(fetch_data),
    info_Update(update_data),
    info_GetAll(get_all),
    info_Delete(del)
    {

    }

    wrapper_ptr(wrapper_ptr&& other) noexcept : m_wrappedWidget(other.m_wrappedWidget),
    info_Get(other.info_Get),
    info_Set(other.info_Set),
    info_Reset(other.info_Reset),
    info_Fetch(other.info_Fetch),
    info_Update(other.info_Update),
    info_GetAll(other.info_GetAll),
    info_Delete(other.info_Delete)
    {
        other.release();
    }

    wrapper_ptr& operator=(wrapper_ptr&& other) noexcept
    {
        if(this != &other)
        {
            reset();
            m_wrappedWidget = other.m_wrappedWidget;
            info_Get = other.info_Get;
            info_Set = other.info_Set;
            info_Reset = other.info_Reset;
            info_Fetch= other.info_Fetch;
            info_Update = other.info_Update;
            info_GetAll = other.info_GetAll;
            info_Delete = other.info_Delete;
            other.release();
        }
        return *this;
    }

    wrapper_ptr(const wrapper_ptr&) = delete;
    wrapper_ptr& operator=(const wrapper_ptr&) = delete;

    ~wrapper_ptr()
    {
        reset();
    }

    void reset()
    {
        if(m_wrappedWidget)
        {
            info_Delete(m_wrappedWidget);
            release();
        }
    }

    operator bool() const
    {
        return m_wrappedWidget != nullptr;
    }


    ///////////////////////////////////////////////////////////////////
    /// @brief Returns a `wrapper_ptr` object for widget type
    /// 
    /// @tparam WidgetT derived widget type from `Widget_t` 
    /// @param w widget pointer
    /// @return `wrapper_ptr` wrapper for widget
    ///
    ///////////////////////////////////////////////////////////////////
    template<typename WidgetT>
    static wrapper_ptr make_wrapper(WidgetT* w)
    {
        return wrapper_ptr(
            new WidgetWrapper<WidgetT>(w),
            +[](void* iw, const std::string& key) -> std::optional<Accessed_t>
            {
                return static_cast<WidgetWrapper<WidgetT>*>(iw)->get(key);
            },
            +[](void* iw, const std::string& key, Accessed_t val) -> bool
            {
                return static_cast<WidgetWrapper<WidgetT>*>(iw)->set(key, val);
            },
            +[](void* iw, const std::string& key) -> bool
            {
                return static_cast<WidgetWrapper<WidgetT>*>(iw)->reset(key);
            },
            +[](void* iw) -> LookupVec_t*
            {
                return static_cast<WidgetWrapper<WidgetT>*>(iw)->fetchData();
            },
            +[](void* iw) -> void
            {
                return static_cast<WidgetWrapper<WidgetT>*>(iw)->updateValues();
            },
            +[](void* iw) -> std::unordered_map<std::string, Accessed_t>
            {
                return static_cast<WidgetWrapper<WidgetT>*>(iw)->getValues();
            },
            +[](void* p) -> void
            {
                delete static_cast<WidgetWrapper<WidgetT>*>(p);
            }
        );
    }
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Removes ownership of widget
    ///
    ///////////////////////////////////////////////////////////////////
    void release()
    {
        m_wrappedWidget = nullptr;
        info_Get = nullptr;
        info_Set = nullptr;
        info_Reset = nullptr;
        info_Delete = nullptr;
        info_Fetch = nullptr;
        info_Update = nullptr;
        info_GetAll = nullptr;
        info_Delete = nullptr;
    }

private:
    void* m_wrappedWidget{nullptr};         //!< `WidgetWrapper<T>` pointer
    GetFn_t info_Get{nullptr};              //!< `get` method lambda wrapper
    SetFn_t info_Set{nullptr};              //!< `set` method lambda wrapper
    ResetFn_t info_Reset{nullptr};          //!< `reset` method lambda wrapper
    FetchFn_t info_Fetch{nullptr};          //!< `fetchData` method lambda wrapper
    UpdateFn_t info_Update{nullptr};        //!< `updateValues` method lambda wrapper
    GetValsFn_t info_GetAll{nullptr};       //!< `getValues` method lambda wrapper
    DeleterFn_t info_Delete{nullptr};       //!< deleter for wrapped widget
};
}
#endif
#endif