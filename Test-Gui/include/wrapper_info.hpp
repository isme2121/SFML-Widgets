#pragma once
#include "info_base.hpp"
#include <iostream>

#if defined(WIDGETS_HPP) && defined(BUILD_TESTS)


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
    using ResetFn_t = bool (*)(void*, const std::string&);
    using FetchFn_t = LookupVec_t* (*)(void*);
    using UpdateFn_t = void (*)(void*);
    using GetValsFn_t = std::unordered_map<std::string, Accessed_t> (*)(void*);
    using DeleterFn_t = void (*)(void*);

public:
    info_ptr() = default;

    info_ptr(void* w, GetFn_t getter, SetFn_t setter, ResetFn_t resetter, FetchFn_t fetch_data, UpdateFn_t update_data, GetValsFn_t get_all, DeleterFn_t del) : m_wrappedWidget(w),
    info_Get(getter),
    info_Set(setter),
    info_Reset(resetter),
    info_Fetch(fetch_data),
    info_Update(update_data),
    info_GetAll(get_all),
    info_Delete(del)
    {

    }

    info_ptr(info_ptr&& other) noexcept : m_wrappedWidget(other.m_wrappedWidget),
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

    info_ptr& operator=(info_ptr&& other) noexcept
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
            +[](void* iw, const std::string& key) -> std::optional<Accessed_t>
            {
                return static_cast<InfoWidget<WidgetT>*>(iw)->get(key);
            },
            +[](void* iw, const std::string& key, Accessed_t val) -> bool
            {
                return static_cast<InfoWidget<WidgetT>*>(iw)->set(key, val);
            },
            +[](void* iw, const std::string& key) -> bool
            {
                return static_cast<InfoWidget<WidgetT>*>(iw)->reset(key);
            },
            +[](void* iw) -> LookupVec_t*
            {
                return static_cast<InfoWidget<WidgetT>*>(iw)->fetchData();
            },
            +[](void* iw) -> void
            {
                return static_cast<InfoWidget<WidgetT>*>(iw)->updateValues();
            },
            +[](void* iw) -> std::unordered_map<std::string, Accessed_t>
            {
                return static_cast<InfoWidget<WidgetT>*>(iw)->getValues();
            },
            +[](void* p) -> void
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
        info_Reset = nullptr;
        info_Delete = nullptr;
        info_Fetch = nullptr;
        info_Update = nullptr;
        info_GetAll = nullptr;
        info_Delete = nullptr;
    }

private:
    void* m_wrappedWidget{nullptr};         //!< `InfoWidget<T>` pointer
    GetFn_t info_Get{nullptr};              //!< `get` method lambda wrapper
    SetFn_t info_Set{nullptr};              //!< `set` method lambda wrapper
    ResetFn_t info_Reset{nullptr};          //!< `reset` method lambda wrapper
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
    enum class ShaderType : int;

public:
    explicit InfoWrapper(sf::RenderWindow&, const std::string& = "");

    InfoWrapper(const InfoWrapper&) = delete;
    InfoWrapper& operator=(const InfoWrapper&) = delete;
    InfoWrapper(InfoWrapper&&) = delete;
    InfoWrapper& operator=(InfoWrapper&&) = delete;

    ~InfoWrapper() override;

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
                    using Derived_t = std::decay_t<decltype(w)>;
                    Widget* widget_ptr = &w;
                    CachedAttributes& cache = m_cache[widget_ptr];
                    cache.label = "Widget " + std::to_string(m_labelCounter++); 

                    /* Reserved events */
                    w.bindMouseEvent(Widget::W_MouseEventType::FocusSet, [this, &w](std::optional<sf::Event>)
                    {
                        selectWidget<Derived_t>(&w);
                    });
                    w.bindMouseEvent(Widget::W_MouseEventType::FocusLost, [this, widget_ptr](std::optional<sf::Event>)
                    {
                        if(m_selected.widget_ptr == widget_ptr) unselectWidget();
                    });


                    /* Bind events */
                    w.bindMouseEvent(Widget::W_MouseEventType::Press, [this, &w, &cache](std::optional<sf::Event> e)
                    {
                        if(isBuiltinFlagSet(cache.flags, BuiltinBindFlags::Drag))
                        {
                            if(const sf::Event::MouseButtonPressed* pressed = e->getIf<sf::Event::MouseButtonPressed>())
                            {
                                if(const Frame* frame = w.getRootFrame())
                                    cache.grab_offset = frame->mapPixelToCoords(pressed->position) - w.getBounds().position;
                                else
                                    cache.grab_offset = sf::Vector2f(pressed->position) - w.getBounds().position;
                            }
                        }
                    });

                    w.bindMouseEvent(Widget::W_MouseEventType::Press, [this, &w, &cache](std::optional<sf::Event> e)
                    {
                        if(isBuiltinFlagSet(cache.flags, BuiltinBindFlags::RightClickDelete))
                        {
                            if(&w == m_selected.widget_ptr) m_selected.to_be_deleted = true;
                        }
                    }, sf::Mouse::Button::Right);

                    w.bindMouseEvent(Widget::W_MouseEventType::Hold, [this, &w, &cache](std::optional<sf::Event>)
                    {
                        if(isBuiltinFlagSet(cache.flags, BuiltinBindFlags::Drag))
                        {
                            sf::Vector2f current;
                            const sf::Vector2i window_pixel = sf::Mouse::getPosition(m_window);
                            if(const Frame* frame = w.getRootFrame())
                                current = frame->mapPixelToCoords(window_pixel);
                            else
                                current = sf::Vector2f(window_pixel);

                            w.setWidgetPosition(current - cache.grab_offset);
                        }
                    });

                    w.bindMouseEvent(Widget::W_MouseEventType::Scroll, [this, &w, &cache](std::optional<sf::Event> e)
                    {
                        if(isBuiltinFlagSet(cache.flags, BuiltinBindFlags::ScrollResize))
                        {
                            if(e)
                            {
                                if(const auto* scrolled = e->getIf<sf::Event::MouseWheelScrolled>())
                                {
                                    sf::Vector2f current_size = w.getWidgetSize();
                                    w.setWidgetSize({current_size.x + 5.f * scrolled->delta, current_size.y + 5.f * scrolled->delta});
                                }
                            }
                        }
                    });

                    w.bindMouseEvent(Widget::W_MouseEventType::HoverSet, [this, &cache](std::optional<sf::Event>)
                    {
                        if(isBuiltinFlagSet(cache.flags, BuiltinBindFlags::HighlightOnHover))
                        {
                            std::cout << "Hovered (highlight event)\n";
                        }
                    });

                    w.bindMouseEvent(Widget::W_MouseEventType::HoverLost, [this, &cache](std::optional<sf::Event>)
                    {
                        if(isBuiltinFlagSet(cache.flags, BuiltinBindFlags::HighlightOnHover))
                        {
                            std::cout << "Not Hovered (highlight event)\n";
                        }
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
        if(m_selected.widget_ptr && m_selected.widget_ptr->isFlag(FLAGS::HIDDEN | FLAGS::BLOCKED))
        {
            // prevent soft-locking a widget
            FLAGS reset_before_its_too_late;
            if(m_selected.widget_ptr->isFlag(FLAGS::HIDDEN)) reset_before_its_too_late = reset_before_its_too_late | FLAGS::HIDDEN;
            if(m_selected.widget_ptr->isFlag(FLAGS::BLOCKED)) reset_before_its_too_late = reset_before_its_too_late | FLAGS::BLOCKED;
            m_selected.widget_ptr->configure(reset_before_its_too_late);
        }
        m_selected.widget_info_ptr.reset();
        m_selected.widget_ptr = nullptr;
        m_selected.enable_if_frame_ptr = nullptr;
        m_selected.to_be_deleted = false;
        m_rows = nullptr;
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
    void refreshUI();

    ///////////////////////////////////////////////////////////////////
    /// @brief Render a header in ImGui interface
    ///
    ///////////////////////////////////////////////////////////////////
    void textHeader(const char*, bool = true, bool = true, ImVec4 = ImVec4(1.f, 1.f, 1.f, 1.f)) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Attempt to set a value after an update
    ///
    ///////////////////////////////////////////////////////////////////
    void trySetFromText(const std::string& key, const std::string& text);

    ///////////////////////////////////////////////////////////////////
    /// @brief Opens a file dialog on a separate thread
    ///
    /// Used for file loader keys
    ///
    /// @param key accessor key
    /// @param title dialog title
    /// @param filters allowed file extensions
    /// @param filter_count number of extensions listed on filters
    /// @param filter_desc description in dialog
    ///
    ///////////////////////////////////////////////////////////////////
    void openFileDialog(const std::string& key, const char* title, const char* const* filters, int filter_count, const char* filter_desc);

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
    /// @brief Renders sfml widgets
    ///
    ///////////////////////////////////////////////////////////////////
    void renderWinElements() override;

    ///////////////////////////////////////////////////////////////////
    /// @brief Renders "Edit" section in ImGui interface
    ///
    ///////////////////////////////////////////////////////////////////
    void renderEditSection();

    ///////////////////////////////////////////////////////////////////
    /// @brief Renders "Selected" section in ImGui interface
    ///
    ///////////////////////////////////////////////////////////////////
    void renderSelectedSection();
    
    ///////////////////////////////////////////////////////////////////
    /// @brief Renders "Flags" section in ImGui interface
    ///
    ///////////////////////////////////////////////////////////////////
    void renderFlagsSection();

    ///////////////////////////////////////////////////////////////////
    /// @brief Renders widget attributes in ImGui interface
    ///
    ///////////////////////////////////////////////////////////////////
    void renderAccessorSections();

    ///////////////////////////////////////////////////////////////////
    /// @brief Renders "Shaders" section in ImGui interface
    ///
    ///////////////////////////////////////////////////////////////////
    void renderShadersSection();

    ///////////////////////////////////////////////////////////////////
    /// @brief Renders "ToolTip" section in ImGui interface
    ///
    ///////////////////////////////////////////////////////////////////
    void renderToolTipSection();

    ///////////////////////////////////////////////////////////////////
    /// @brief Managed by `renderSelectedSection`
    ///
    ///////////////////////////////////////////////////////////////////
    void _renderProperties(CacheMap_t::iterator& cached_data);

    ///////////////////////////////////////////////////////////////////
    /// @brief Managed by `renderSelectedSection`
    ///
    ///////////////////////////////////////////////////////////////////
    void _renderBindings(CacheMap_t::iterator& cached_data);

    ///////////////////////////////////////////////////////////////////
    /// @brief Managed by `renderSelectedSection`
    ///
    ///////////////////////////////////////////////////////////////////
    void _renderTheme(CacheMap_t::iterator& cached_data);

    ///////////////////////////////////////////////////////////////////
    /// @brief Managed by `renderSelectedSection`
    ///
    ///////////////////////////////////////////////////////////////////
    void _renderWidgetGroups();

    ///////////////////////////////////////////////////////////////////
    /// @brief Renders a single shader type in the UI
    ///
    /// @param label shader label
    /// @param is_suffix Widget ID
    /// @param state current selected path
    /// @param loadFn `Widget` class function for loading the shader
    /// @param resetFn `Widget` class function for resetting the shader
    ///
    ///////////////////////////////////////////////////////////////////
    void _addShaderUI(const char* label, ShaderType shader_type, ShaderCache& state);

    ///////////////////////////////////////////////////////////////////
    /// @brief Renders the selected `ToolTip` object's accessor variables
    ///
    ///////////////////////////////////////////////////////////////////
    void _renderToolTipEditor(ToolTipCache&, ToolTipObject&);

    ///////////////////////////////////////////////////////////////////
    /// @brief Renders a single key attribute in accessor section
    ///
    ///////////////////////////////////////////////////////////////////
    void renderRow(Accessor_ptr&);

    ///////////////////////////////////////////////////////////////////
    /// @brief Renders FPS in "Edit" section
    ///
    ///////////////////////////////////////////////////////////////////
    void renderFps() const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Syncs file dialog returned path with internal path
    ///
    ///////////////////////////////////////////////////////////////////
    void pullDialogPath();

    ///////////////////////////////////////////////////////////////////
    /// @brief Returns the label of the root object for parameter widget
    ///
    ///////////////////////////////////////////////////////////////////
    const char* getRootLabel(Widget* w) const;

    ///////////////////////////////////////////////////////////////////
    /// @brief Selected widget information
    ///
    ///////////////////////////////////////////////////////////////////
    struct SelectedWidget
    {
        Widget* widget_ptr{nullptr};            //!< selected widget
        Frame* enable_if_frame_ptr{nullptr};    //!< also selected widget, enabled if it's a `Frame` type
        info_ptr widget_info_ptr;               //!< info wrapper for widget
        bool to_be_deleted{false};              //!< selected widget is marked for deletion
    };

    enum class ShaderType
    {
        Clip,
        Color
    };
    
    static constexpr unsigned int s_MAX_WIDGET_COUNT{10};
    static constexpr unsigned int s_GROUPS_COUNT{4};

    std::array<std::optional<Widget_t>, s_MAX_WIDGET_COUNT> m_widgets;  //!< widgets storage
    std::array<LabeledGroup, s_GROUPS_COUNT> m_widgetGroups;

    SelectedWidget m_selected;
    LookupVec_t* m_rows{nullptr};                                       //!< selected widget accessor data
    CacheMap_t m_cache;                                                 //!< memory for attributes which can't be obtained from the widget
    DialogThread m_dialog;                                              //!< path dialog

    unsigned int m_widgetsCount{0};                                     //!< active widgets count
    unsigned int m_labelCounter{0};
    unsigned int m_selectToken{0};                                      //!< context switch between selections to prevent value inheritence
    bool m_revealFps{false};
};
}

#endif
#endif