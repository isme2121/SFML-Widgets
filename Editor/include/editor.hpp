#pragma once
#include "imgui_panel.hpp"
#if defined(WIDGETS_HPP) && defined(BUILD_EDITOR_GUI)
#include "editor_cache.hpp"
#include "wrapper_ptr.hpp"
#include <iostream>

namespace sfml_widgets
{
///////////////////////////////////////////////////////////////////
/// @brief Integration of WidgetWrapper to dear ImGui's interface
///
///////////////////////////////////////////////////////////////////
class WidgetEditor : public ImGuiFrame
{
    enum class ShaderType : int;

public:
    explicit WidgetEditor(sf::RenderWindow&, const std::string& = "");

    WidgetEditor(const WidgetEditor&) = delete;
    WidgetEditor& operator=(const WidgetEditor&) = delete;
    WidgetEditor(WidgetEditor&&) = delete;
    WidgetEditor& operator=(WidgetEditor&&) = delete;

    ~WidgetEditor() override;

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
        
        m_selected.widget_info_ptr = wrapper_ptr::make_wrapper<WidgetT>(w);
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

    void _renderCanvasLineSection(GridCanvas* GridCanvas, CanvasCache& cache);
    void _renderCanvasCellSection(GridCanvas* GridCanvas, CanvasCache& cache);
    void _renderCanvasQuickDrawSection(GridCanvas* GridCanvas, CanvasCache& cache);

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
        wrapper_ptr widget_info_ptr;               //!< info wrapper for widget
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