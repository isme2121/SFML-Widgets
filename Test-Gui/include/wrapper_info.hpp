#pragma once
#include "info_base.hpp"
#include <iostream>

#if defined(WIDGETS_HPP) && defined(COMPILE_TESTS)


namespace sfml_widgets
{
///////////////////////////////////////////////////////////////////
/// @brief Wrapper for widget for setters/getters accessability
///
/// @tparam WidgetT must be a derived widget type
///
///////////////////////////////////////////////////////////////////
template<typename WidgetT>
class InfoWidget
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
    explicit InfoWidget(Ts&&... args) : m_widget(new WidgetT(std::forward<Ts>(args)...)) , m_isManaged(false)
    {
        // link all getters/setters for this widget
        BaseAccessorsLink<WidgetT>::link(m_lookup);
        AccessorsLink<WidgetT>::link(m_lookup);
    }

    explicit InfoWidget(WidgetT* w) : m_widget(w), m_isManaged(true)
    {
        // link all getters/setters for this widget
        BaseAccessorsLink<WidgetT>::link(m_lookup);
        AccessorsLink<WidgetT>::link(m_lookup);
    }

    ~InfoWidget()
    {
        if(!m_isManaged)
        {
            delete m_widget;
            m_widget = nullptr;
        }
    }

    InfoWidget(const InfoWidget&) = delete;
    InfoWidget& operator=(const InfoWidget&) = delete;
    InfoWidget(InfoWidget&&) = delete;
    InfoWidget& operator=(InfoWidget&&) = delete;

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
        auto it = m_lookup.find(key);
        if(it == m_lookup.end() || !it->second.getter) return std::nullopt;
        return it->second.getter(*m_widget);
    }

    ///////////////////////////////////////////////////////////////////
    /// @brief Calls the proper setter based on `key` string, `v` must
    /// match the proper type of the setter
    ///
    ///////////////////////////////////////////////////////////////////
    bool set(const std::string& key, Accessed_t v)
    {
        auto it = m_lookup.find(key);
        if(it != m_lookup.end() && it->second.setter) return it->second.setter(*m_widget, v);
        return false;
    }

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns all the current getter results with their pair keys
    ///
    ///////////////////////////////////////////////////////////////////
    std::map<std::string, Accessed_t> getValues() const
    {
        std::map<std::string, Accessed_t> tmp;
        for(const auto& [key, funcs] : m_lookup)
            if(funcs.getter) tmp.emplace(key, funcs.getter(*m_widget));
        return tmp;
    }

    ///////////////////////////////////////////////////////////////////
    /// @brief Fills data vector with row information for each key
    ///
    /// @param data vector reference received from `InfoWrapper` base object
    ///
    ///////////////////////////////////////////////////////////////////
    void fetchData(std::vector<Info>& data) const
    {
        data.clear();
        data.reserve(m_lookup.size());
        for(const auto& [key, funcs] : m_lookup)
        {
            Info info;
            info.info_key = key;
            info.info_has_setter = static_cast<bool>(funcs.setter);
            if(funcs.getter) info.info_val = funcs.getter(*m_widget);
            data.emplace_back(std::move(info));
        }
    }

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates the value attribute of the current data vector
    ///
    /// @param data vector reference received from `InfoWrapper` base object
    ///
    ///////////////////////////////////////////////////////////////////
    void updateValues(std::vector<Info>& data) const
    {
        for(Info& info : data)
        {
            if(!info.info_val.has_value()) continue;

            auto accessor_it = m_lookup.find(info.info_key);
            if(accessor_it != m_lookup.end() && accessor_it->second.getter)
                info.info_val = accessor_it->second.getter(*m_widget);
        }
    }

private:
    WidgetT* m_widget;                  // main widget
    LookupMap_t<WidgetT> m_lookup;      // accessor lookup
    bool m_isManaged{false};            // true = widget memory is managed externally, false = widget memory is managed by class
};

#ifdef WRAPPER_DEFINED

///////////////////////////////////////////////////////////////////
/// @brief `InfoWidget` wrapper for `InfoWrapper` class, provides
/// access to `InfoWidget` methods under a non-templated scope.
///
/// construct with `make_wrapper`
///
///////////////////////////////////////////////////////////////////
class info_ptr
{
    friend class InfoWrapper;
    using GetFn_t = std::optional<Accessed_t> (*)(void*, const std::string&);
    using SetFn_t = bool (*)(void*, const std::string&, Accessed_t);
    using FetchFn_t = void (*)(void*, std::vector<Info>&);
    using UpdateFn_t = void (*)(void*, std::vector<Info>&);
    using GetValsFn_t = std::map<std::string, Accessed_t> (*)(void*);
    using DeleterFn_t = void (*)(void*);

public:
    info_ptr() = default;

    info_ptr(void* w, GetFn_t getter, SetFn_t setter, FetchFn_t fetch_data, UpdateFn_t update_data, GetValsFn_t get_all, DeleterFn_t del) : m_wrappedWidget(w),
    info_Get(getter),
    info_Set(setter),
    info_Fetch(fetch_data),
    info_Update(update_data),
    info_GetAll(get_all),
    info_Delete(del)
    {

    }

    info_ptr(info_ptr&& other) noexcept : m_wrappedWidget(other.m_wrappedWidget),
    info_Get(other.info_Get),
    info_Set(other.info_Set),
    info_Fetch(other.info_Fetch),
    info_Update(other.info_Update),
    info_GetAll(other.info_GetAll),
    info_Delete(other.info_Delete)
    {
        other.release();
    }

    info_ptr& operator=(info_ptr&& other) noexcept
    {
        if(this != &other)
        {
            reset();
            m_wrappedWidget = other.m_wrappedWidget;
            info_Get = other.info_Get;
            info_Set = other.info_Set;
            info_Fetch= other.info_Fetch;
            info_Update = other.info_Update;
            info_GetAll = other.info_GetAll;
            info_Delete = other.info_Delete;
            other.release();
        }
        return *this;
    }

    info_ptr(const info_ptr&) = delete;
    info_ptr& operator=(const info_ptr&) = delete;

    ~info_ptr()
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
    /// @brief Returns a `info_ptr` object for widget type
    /// 
    /// @tparam WidgetT derived widget type from `Widget_t` 
    /// @param w widget pointer
    /// @return `info_ptr` wrapper for widget
    ///
    ///////////////////////////////////////////////////////////////////
    template<typename WidgetT>
    static info_ptr make_wrapper(WidgetT* w)
    {
        static_assert(is_widget_type_v<WidgetT>, "Template type does not match any supported widget derived type.");

        return info_ptr(
            new InfoWidget<WidgetT>(w),
            +[](void* w, const std::string& key) -> std::optional<Accessed_t> // get
            {
                return static_cast<InfoWidget<WidgetT>*>(w)->get(key);
            },
            +[](void* w, const std::string& key, Accessed_t val) -> bool // set
            {
                return static_cast<InfoWidget<WidgetT>*>(w)->set(key, val);
            },
            +[](void* w, std::vector<Info>& data) -> void // fetch
            {
                return static_cast<InfoWidget<WidgetT>*>(w)->fetchData(data);
            },
            +[](void* w, std::vector<Info>& data) -> void // update
            {
                return static_cast<InfoWidget<WidgetT>*>(w)->updateValues(data);
            },
            +[](void* w) -> std::map<std::string, Accessed_t> // get values
            {
                return static_cast<InfoWidget<WidgetT>*>(w)->getValues();
            },
            +[](void* p) -> void // delete
            {
                delete static_cast<InfoWidget<WidgetT>*>(p);
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
        info_Delete = nullptr;
        info_Fetch = nullptr;
        info_Update= nullptr;
        info_GetAll = nullptr;
        info_Delete = nullptr;
    }

private:
    void* m_wrappedWidget{nullptr};         //!< `InfoWidget<T>` pointer
    GetFn_t info_Get{nullptr};              //!< `get` method lambda wrapper
    SetFn_t info_Set{nullptr};              //!< `set` method lambda wrapper
    FetchFn_t info_Fetch{nullptr};          //!< `fetchData` method lambda wrapper
    UpdateFn_t info_Update{nullptr};        //!< `updateValues` method lambda wrapper
    GetValsFn_t info_GetAll{nullptr};       //!< `getValues` method lambda wrapper
    DeleterFn_t info_Delete{nullptr};       //!< deleter for wrapped widget
};

///////////////////////////////////////////////////////////////////
/// @brief Integration of InfoWidget to dear ImGui's interface
///
///////////////////////////////////////////////////////////////////
class InfoWrapper : public ImGuiWrapper
{
public:
    explicit InfoWrapper(sf::RenderWindow&, const std::string& = "");

    InfoWrapper(const InfoWrapper&)            = delete;
    InfoWrapper& operator=(const InfoWrapper&) = delete;
    InfoWrapper(InfoWrapper&&)                 = delete;
    InfoWrapper& operator=(InfoWrapper&&)      = delete;

    ~InfoWrapper() override = default;

    ///////////////////////////////////////////////////////////////////
    /// @brief Toggles FPS text in GUI
    ///
    ///////////////////////////////////////////////////////////////////
    void enableFps(bool);


    ///////////////////////////////////////////////////////////////////
    /// @brief Constructs a new sfml widget internally
    ///
    /// Created widget is tracked and managed by this instance, fails and
    /// returns false if the maximum count of `s_MAX_WIDGET_COUNT` is already reached.
    ///
    /// @tparam WidgetT derived widget type from `Widget_t`
    /// @tparam Arguments types
    /// @param args widget arguments
    ///
    /// @return true if the widget is created successfully, otherwise false.
    ///
    ///////////////////////////////////////////////////////////////////
    template<typename WidgetT, typename... Ts>
    bool addWidget(Ts&&... args)
    {
        static_assert(is_widget_type_v<WidgetT>, "Template type does not match any supported widget derived type.");

        if(m_widgetsCount >= s_MAX_WIDGET_COUNT)
        {
            std::cerr << "Failed to add widget: Max widget count{" << s_MAX_WIDGET_COUNT << "} reached.\n";
            return false;
        }

        // filling empty slots
        for(std::optional<Widget_t>& widget : m_widgets)
        {
            if(!widget.has_value())
            {
                widget.emplace(std::in_place_type<WidgetT>, std::forward<Ts>(args)...);
                ++m_widgetsCount;

                std::visit([this](auto& w)
                {
                    // this applies to all types
                    using ActualT = std::decay_t<decltype(w)>;
                    Widget* widget_ptr = &w;

                    // selecting the widget event
                    w.bindMouseEvent(Widget::W_MouseEventType::FocusSet, [this, &w](std::optional<sf::Event>)
                    {
                        selectWidget<ActualT>(&w);
                    });

                    // unselecting the widget event
                    w.bindMouseEvent(Widget::W_MouseEventType::FocusLost, [this, widget_ptr](std::optional<sf::Event>)
                    {
                        if(m_selected.widget_ptr == widget_ptr) unselectWidget();
                    });
                }, *widget);

                break;
            }
        }

        return true;
    }

    ///////////////////////////////////////////////////////////////////
    /// @brief Selects a managed widget
    ///
    ///////////////////////////////////////////////////////////////////
    template<typename WidgetT>
    void selectWidget(WidgetT* w)
    {
        static_assert(is_widget_type_v<WidgetT>, "Template type does not match any supported widget derived type.");

        if(w == m_selected.widget_ptr) return;
        
        m_selected.widget_info_ptr = info_ptr::make_wrapper<WidgetT>(w);
        m_selected.widget_ptr = w;
        m_selected.enable_if_frame_ptr = nullptr;
        if constexpr(std::is_same_v<WidgetT, Frame>)
            m_selected.enable_if_frame_ptr = w;

        ++m_selectToken;
        refreshUI();
    }

    ///////////////////////////////////////////////////////////////////
    /// @brief Unselects currently selected widget if any
    ///
    ///////////////////////////////////////////////////////////////////
    void unselectWidget()
    {
        m_selected.widget_info_ptr.reset();
        m_selected.widget_ptr = nullptr;
        m_selected.enable_if_frame_ptr = nullptr;
        m_rows.clear();
        ++m_selectToken;
    }

private:
    ///////////////////////////////////////////////////////////////////
    /// @brief Clears all widgets from the window
    ///
    ///////////////////////////////////////////////////////////////////
    void resetUI();

    ///////////////////////////////////////////////////////////////////
    /// @brief Removes the currently selected widget
    ///
    ///////////////////////////////////////////////////////////////////
    void deleteSelectedWidget();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates accessor values for selected widget
    ///
    ///////////////////////////////////////////////////////////////////
    void refreshValues();

    ///////////////////////////////////////////////////////////////////
    /// @brief Updates ImGui interface, triggers when a new widget is selected
    ///
    ///////////////////////////////////////////////////////////////////
    void refreshUI(); // rebuildRows

    ///////////////////////////////////////////////////////////////////
    /// @brief Displays info data as widgets in ImGui interface
    ///
    ///////////////////////////////////////////////////////////////////
    void renderRow(Info&);

    ///////////////////////////////////////////////////////////////////
    /// @brief Attempt to set a value after an update
    ///
    ///////////////////////////////////////////////////////////////////
    void trySetFromText(const std::string& key, const std::string& text);

    template<typename EnumT, std::size_t N>
    static bool renderIndexedEnum(EnumT& value, const std::array<const char*, N>& names)
    {
        int idx = std::clamp(static_cast<int>(value), 0, static_cast<int>(N) - 1);

        ImGui::SetNextItemWidth(100.f);
        bool changed = ImGui::InputInt("##key", &idx);
        if (changed)
        {
            idx = std::clamp(idx, 0, static_cast<int>(N) - 1);
            value = static_cast<EnumT>(idx);
        }
        ImGui::SameLine();
        ImGui::Text("%s selected", names[static_cast<std::size_t>(idx)]);
        return changed;
    }

    ///////////////////////////////////////////////////////////////////
    /// @brief Processing external events
    ///
    ///////////////////////////////////////////////////////////////////
    bool processEvents(const sf::Event&) override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Draw ImGui interface + sfml widgets
    ///
    ///////////////////////////////////////////////////////////////////
    void renderWidgets() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Selected widget information
    ///
    ///////////////////////////////////////////////////////////////////
    struct SelectedWidget
    {
        Widget* widget_ptr{nullptr};            //!< selected widget
        Frame* enable_if_frame_ptr{nullptr};    //!< also selected widget, enabled if it's a `Frame` type
        info_ptr widget_info_ptr;               //!< info wrapper for widget
    };
    
    static constexpr std::size_t s_MAX_WIDGET_COUNT{10};

    std::vector<Info> m_rows;   //!< selected widget accessor data
    std::array<std::optional<Widget_t>, s_MAX_WIDGET_COUNT> m_widgets;  //!< managed widgets
    unsigned int m_widgetsCount{0}; //!< active widgets count
    unsigned int m_selectToken{0};  //!< context switch between selections to prevent value inheritence
    SelectedWidget m_selected;      //!< selected widget
    bool m_revealFps{false};        //!< fps toggle
};
}

#endif
#endif