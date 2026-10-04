#include "wrapper_info.hpp"
#include "tinyfiledialogs.h"

#ifdef WRAPPER_DEFINED

namespace
{
    constexpr ImVec4 HEADER_1{0.65f, 0.65f, 0.65f, 1.0f};   // gray
    constexpr ImVec4 HEADER_2{0.35f, 0.85f, 0.45f, 1.0f};   // green
    constexpr ImVec4 HEADER_3{0.55f, 0.75f, 1.0f, 1.0f};    // blue
    constexpr ImVec4 RED{0.80f, 0.25f, 0.25f, 1.0f};
    constexpr ImVec4 DARK_RED{0.70f, 0.20f, 0.20f, 1.0f};
    constexpr ImVec4 DARKER_RED{0.55f, 0.15f, 0.15f, 1.0f};
    constexpr ImVec4 LIME_GREEN{0.25f, 0.55f, 0.30f, 1.0f};    // lime
    constexpr ImVec4 SECTION_LABEL_COLOR{0.55f, 0.75f, 1.0f, 1.0f};     // light green

    constexpr const char* IMG_EXTENSIONS[] = {"*.png", "*.jpg", "*.jpeg"};
    constexpr int N_IMG_EXTENSIONS = 3;
    constexpr const char* SHADER_EXTENSIONS[] = {"*.frag", "*.glsl"};
    constexpr int N_SHADER_EXTENSIONS = 2;

    constexpr unsigned int FPS_TEXT_LENGTH = 12;
    constexpr unsigned int WIDGET_LABEL_LENGTH = 16;
    constexpr unsigned int GROUP_LABEL_LENGTH = 32;
    constexpr unsigned int TT_OBJECT_ID_ENTRY_LENGTH = 12;
    constexpr unsigned int TT_TEXT_OBJECT_LENGTH = 256;

    constexpr const char* FPS_TEXT_FORMAT = "FPS: %.1f";

    constexpr unsigned int FLAGS_N_ROWS = 6;
    constexpr ImU32 SET_CELL_FILL_COLOR = IM_COL32(80, 200, 120, 255);  // green
    constexpr ImU32 RESET_CELL_FILL_COLOR = IM_COL32(70, 70, 70, 255);  // gray


    constexpr float CHAR_INPUT_WIDTH = 40.f;
    constexpr float SMALL_BUTTON_WIDTH = 60.f;
    constexpr float NUMERIC_INPUT_WIDTH = 80.f;
    constexpr float ENUM_INPUT_WIDTH = 100.f;
    constexpr float DOUBLE_INPUT_WIDTH = 120.f;
    constexpr float TEXT_INPUT_WIDTH = 120.f;
    constexpr float PATH_INPUT_WIDTH = 160.f;
    constexpr float COMBO_INPUT_WIDTH = 200.f;
    constexpr float UNKNOWN_INPUT_WIDTH = 200.f;
    constexpr ImVec2 MULTITEXT_INPUT_DIMENSIONS{200.f, 40.f};

    constexpr const char* NULL_TEXT = "NOT FOUND";
    constexpr const char* ROOT_WIN_TEXT = "Main Window";

    // anonymous namespace, alongside the existing UI constants
    constexpr float TOOLTIP_TREE_HEIGHT = 120.f;

    sf::Color toSfColor(const float rgba[4])
    {
        return sf::Color(
            static_cast<std::uint8_t>(rgba[0] * 255.f + 0.5f),
            static_cast<std::uint8_t>(rgba[1] * 255.f + 0.5f),
            static_cast<std::uint8_t>(rgba[2] * 255.f + 0.5f),
            static_cast<std::uint8_t>(rgba[3] * 255.f + 0.5f)
        );
    }

    void toFloatColor(sf::Color c, float out[4])
    {
        out[0] = c.r / 255.f;
        out[1] = c.g / 255.f;
        out[2] = c.b / 255.f;
        out[3] = c.a / 255.f;
    }
}

namespace sfml_widgets
{
InfoWrapper::InfoWrapper(sf::RenderWindow& window, const std::string& title) : ImGuiWrapper(window, title)
{
    for(int idx{0}; idx < s_GROUPS_COUNT; idx++)
    {
        m_widgetGroups[idx].group_name = "Group " + std::to_string(idx);
        m_widgetGroups[idx].group_ptr = WidgetGroup::make_group();
    }
}

InfoWrapper::~InfoWrapper()
{
    if(m_dialog.thread.joinable()) m_dialog.thread.join();
}

void InfoWrapper::enableFps(bool show_fps)
{
    m_revealFps = show_fps;
}

void InfoWrapper::resetUI()
{
    unselectWidget();
    for(std::optional<Widget_t>& widget : m_widgets)
    {
        if(widget.has_value())
        {
            m_cache.erase(get_base_widget(*widget));
            widget.reset();
        }
    }
    m_widgetsCount = 0;
}

void InfoWrapper::deleteSelectedWidget()
{
    if(!m_selected.widget_ptr) return;

    for(std::optional<Widget_t>& widget : m_widgets)
    {
        if(!widget.has_value()) continue;

        // address compare
        if(get_base_widget(*widget) == m_selected.widget_ptr)
        {
            m_cache.erase(m_selected.widget_ptr);
            unselectWidget();
            widget.reset();
            --m_widgetsCount;
            return;
        }
    }
}

void InfoWrapper::refreshValues()
{
    if(!m_selected.widget_info_ptr) return;
    m_selected.widget_info_ptr.info_Update(m_selected.widget_info_ptr.m_wrappedWidget);
}

void InfoWrapper::refreshUI()
{
    if(!m_selected.widget_info_ptr) return;
    m_rows = m_selected.widget_info_ptr.info_Fetch(m_selected.widget_info_ptr.m_wrappedWidget);
}

void InfoWrapper::textHeader(const char* title, bool add_space, bool add_sep, ImVec4 color) const
{
    if(add_space) ImGui::Spacing();
    if(add_sep) ImGui::Separator();
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::TextUnformatted(title);
    ImGui::PopStyleColor();
    if(add_sep) ImGui::Separator();
    if(add_space) ImGui::Spacing();
}

void InfoWrapper::trySetFromText(const std::string& key, const std::string& text)
{
    // attempt to call setter with unknown accessor type
    auto try_set_from_val = [&](Accessed_t v) -> bool
    {
        if(!m_selected.widget_info_ptr.info_Set(m_selected.widget_info_ptr.m_wrappedWidget, key, std::move(v))) return false;
        refreshValues();
        return true;
    };

    /* Attempt to match type section */

    // bool type
    if(text == "true" || text == "false")
        if(try_set_from_val(Accessed_t(text == "true"))) return;

    // sf::Vector type
    {
        // hard-coded formats
        unsigned int ux, uy;
        if(std::sscanf(text.c_str(), "%u , %u", &ux, &uy) == 2 || std::sscanf(text.c_str(), "%u %u", &ux, &uy) == 2)
            if(try_set_from_val(Accessed_t(sf::Vector2u(ux, uy)))) return;
        int ix, iy;
        if(std::sscanf(text.c_str(), "%d , %d", &ix, &iy) == 2 || std::sscanf(text.c_str(), "%d %d", &ix, &iy) == 2)
            if(try_set_from_val(Accessed_t(sf::Vector2i(ix, iy)))) return;
        float fx, fy;
        if(std::sscanf(text.c_str(), "%f , %f", &fx, &fy) == 2 || std::sscanf(text.c_str(), "%f %f", &fx, &fy) == 2)
            if(try_set_from_val(Accessed_t(sf::Vector2f(fx, fy)))) return;
    }

    // unsigned int types
    if(!text.empty())
    {
        char* is_valid = nullptr;
        long val = std::strtol(text.c_str(), &is_valid, 10);
        if(is_valid && *is_valid == '\0')
        {
            if(try_set_from_val(Accessed_t(static_cast<unsigned int>(val)))) return;
            if(try_set_from_val(Accessed_t(static_cast<std::size_t>(val)))) return;
        }
    }

    // float type
    if(!text.empty())
    {
        char* is_valid = nullptr;
        float val = std::strtof(text.c_str(), &is_valid);
        if(is_valid && *is_valid == '\0')
            if(try_set_from_val(Accessed_t(val))) return;
    }

    // char type
    if(text.size() == 1)
        if(try_set_from_val(Accessed_t(text[0]))) return;

    // string types
    if(try_set_from_val(Accessed_t(std::string(text)))) return;
    try_set_from_val(Accessed_t(sf::String(text)));

    return;
}

void InfoWrapper::openFileDialog(const std::string& key, const char* title, const char* const* filters, int filter_count, const char* filter_desc)
{
    if(m_dialog.is_open.load()) return;
    if(m_dialog.thread.joinable()) m_dialog.thread.join();

    m_dialog.req_key = key;
    m_dialog.req_token = m_selectToken;
    m_dialog.is_open = true;

    m_dialog.thread = std::thread([this, title, filters, filter_count, filter_desc]()
    {
        const char* result = tinyfd_openFileDialog(title, "", filter_count, filters, filter_desc, 0);
        {
            std::lock_guard<std::mutex> lock(m_dialog.guard);
            m_dialog.protected_path = result ? result : "";
        }
        m_dialog.path_found.store(result != nullptr);
        m_dialog.is_open.store(false);
    });
}

bool InfoWrapper::processEvents(const sf::Event& event)
{
    bool event_used{false};

    if(const auto* key_press = event.getIf<sf::Event::KeyPressed>())
    {
        if(key_press->code == sf::Keyboard::Key::F1)
        {
            toggle();
            event_used = true;
        }
    }

    auto it_rbegin = m_widgets.rbegin();
    auto it_rend = m_widgets.rend();
    while(it_rbegin != it_rend)
    {
        if(it_rbegin->has_value())
        {
            std::visit([&](auto& w)
            {
                if(w.getRootWindow())
                {

                    if(w.captureMouseEvents(event) || w.captureKeyEvents(event))
                    {
                        event_used = true;
                        if(&w == m_selected.widget_ptr) refreshValues();
                    }
                }
            },
            **it_rbegin);
        }
        ++it_rbegin;
    }
    return event_used;
}

void InfoWrapper::renderWidgets()
{
    pullDialogPath();

    if(m_selected.to_be_deleted) deleteSelectedWidget();

    /* Edit section */
    renderEditSection();

    /* Selected widget section */
    if(!m_selected.widget_ptr) return;

    ImGui::PushID(m_selectToken);   //!< avoids value inheritence between previous and new selected widgets
    
    // rendered in order
    renderSelectedSection();
    renderFlagsSection();
    renderAccessorSections();
    renderShadersSection();
    renderToolTipSection();
    ImGui::PopID();
}

void InfoWrapper::renderWinElements()
{
    for(std::optional<Widget_t>& widget : m_widgets)
    {
        if(widget.has_value())
        {
            std::visit([&](auto& w)
            {
                if(w.getRootWindow()) w.draw();
            },
            *widget);
        }
    }
}

void InfoWrapper::renderEditSection()
{
    ImGui::Separator();
    ImGui::PushStyleColor(ImGuiCol_Text, HEADER_1);
    ImGui::TextUnformatted("Edit Section");
    ImGui::PopStyleColor();
    renderFps();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextDisabled("Add Widget");
    if(ImGui::BeginTable("addwidget##grid", 3, ImGuiTableFlags_SizingStretchSame))
    {
        ImGui::TableNextColumn();
        if(ImGui::Button("Button", ImVec2(-FLT_MIN, 0))) //!< -FLT_MIN cuz automatically rounds correctly
        {
            if(m_selected.enable_if_frame_ptr)
            {
                addWidget<Button>(*m_selected.enable_if_frame_ptr, sf::Vector2f{10.f, 10.f}, sf::Vector2u{180, 30}, "Button");
                refreshValues();
            }
            else addWidget<Button>(m_window, sf::Vector2f{10.f, 10.f}, sf::Vector2u{200, 50}, "Button");
        }

        ImGui::TableNextColumn();
        if(ImGui::Button("Frame", ImVec2(-FLT_MIN, 0)))
        {
            if(m_selected.enable_if_frame_ptr)
            {
                addWidget<Frame>(*m_selected.enable_if_frame_ptr, sf::Vector2f{10.f, 10.f}, sf::Vector2u{320, 320});
                refreshValues();
            }
            else addWidget<Frame>(m_window, sf::Vector2f{10.f, 10.f}, sf::Vector2u{320, 320});
        }

        ImGui::TableNextColumn();
        if(ImGui::Button("Label", ImVec2(-FLT_MIN, 0)))
        {
            if(m_selected.enable_if_frame_ptr)
            {
                addWidget<Label>(*m_selected.enable_if_frame_ptr, sf::Vector2f{10.f, 10.f}, sf::Vector2u{160, 50}, "Label");
                refreshValues();
            }
            else addWidget<Label>(m_window, sf::Vector2f{10.f, 10.f}, sf::Vector2u{150, 60}, "Label");
        }

        ImGui::TableNextColumn();
        if(ImGui::Button("Entry", ImVec2(-FLT_MIN, 0)))
        {
            if(m_selected.enable_if_frame_ptr)
            {
                addWidget<Entry>(*m_selected.enable_if_frame_ptr, sf::Vector2f{10.f, 10.f}, sf::Vector2u{180, 30});
                refreshValues();
            }
            else addWidget<Entry>(m_window, sf::Vector2f{10.f, 10.f}, sf::Vector2u{200, 50});
        }

        ImGui::TableNextColumn();
        if(ImGui::Button("CheckBox", ImVec2(-FLT_MIN, 0)))
        {
            if(m_selected.enable_if_frame_ptr)
            {
                addWidget<CheckBox>(*m_selected.enable_if_frame_ptr, sf::Vector2f{10.f, 10.f}, sf::Vector2u{50, 50});
                refreshValues();
            }
            else addWidget<CheckBox>(m_window, sf::Vector2f{10.f, 10.f}, sf::Vector2u{50, 50});
        }

        ImGui::TableNextColumn();
        if(ImGui::Button("ImageFrame", ImVec2(-FLT_MIN, 0)))
        {
            if(m_selected.enable_if_frame_ptr)
            {
                addWidget<ImageFrame>(*m_selected.enable_if_frame_ptr, sf::Vector2f{10.f, 10.f}, sf::Vector2u{200, 200});
                refreshValues();
            }
            else addWidget<ImageFrame>(m_window, sf::Vector2f{10.f, 10.f}, sf::Vector2u{200, 200});
        }

        ImGui::TableNextColumn();
        if(ImGui::Button("GridCanvas", ImVec2(-FLT_MIN, 0)))
        {
            if(m_selected.enable_if_frame_ptr)
            {
                addWidget<GridCanvas>(*m_selected.enable_if_frame_ptr, sf::Vector2f{10.f, 10.f}, sf::Vector2u{320, 320}, std::size_t{100}, 0.f, true);
                refreshValues();
            }
            else addWidget<GridCanvas>(m_window, sf::Vector2f{10.f, 10.f}, sf::Vector2u{320, 320}, std::size_t{100}, 0.f, true);
        }
        ImGui::TableNextColumn();
        if(ImGui::Button("Canvas", ImVec2(-FLT_MIN, 0)))
        {
            if(m_selected.enable_if_frame_ptr)
            {
                addWidget<Canvas>(*m_selected.enable_if_frame_ptr, sf::Vector2f{10.f, 10.f}, sf::Vector2u{320, 320}, sf::Vector2u{10, 10});
                refreshValues();
            }
            else addWidget<Canvas>(m_window, sf::Vector2f{10.f, 10.f}, sf::Vector2u{320, 320}, sf::Vector2u{10, 10});
        }
        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::TextDisabled("Actions");
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Button, RED);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, DARK_RED);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, DARKER_RED);

    ImGui::BeginDisabled(m_selected.widget_ptr == nullptr);
    if(ImGui::Button("Delete Selected")) deleteSelectedWidget();
    ImGui::EndDisabled();

    ImGui::SameLine();
    if(ImGui::Button("Reset Board")) resetUI();

    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    if(ImGui::Button("Refresh")) refreshUI();
}

void InfoWrapper::renderSelectedSection()
{
    auto it = m_cache.find(m_selected.widget_ptr);
    if(it == m_cache.end())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, RED);
        ImGui::TextUnformatted("CACHE ERROR");
        ImGui::Spacing();
        return;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::PushStyleColor(ImGuiCol_Text, HEADER_1);
    ImGui::TextUnformatted("Selected Widget: ");
    ImGui::SameLine(0.f, 12.f);
    ImGui::TextUnformatted(it->second.label.c_str());
    ImGui::PopStyleColor();

    ImGui::Separator();
    ImGui::Spacing();

    ImGui::BeginGroup();
    _renderProperties(it);
    _renderBindings(it);
    _renderTheme(it);
    ImGui::EndGroup();

    ImGui::SameLine();
    ImGui::Dummy(ImVec2(24.f, 0.f));
    ImGui::SameLine();

    ImGui::BeginGroup();
    _renderWidgetGroups();
    ImGui::EndGroup();
}

void InfoWrapper::renderFlagsSection()
{
    textHeader("Flags", true, false, HEADER_2);

    /* Flags Grid */
    ImGui::BeginGroup();
    textHeader("Active Flags:", false, false, HEADER_3);

    constexpr std::size_t N_FLAGS = enum_literals::FLAG_BITS_LITERAL.size();
    constexpr unsigned int N_ROWS = FLAGS_N_ROWS;
    constexpr unsigned int N_COLS = (static_cast<unsigned int>(N_FLAGS) + N_ROWS - 1) / N_ROWS;
    constexpr float CELL_BASE = 18.f;
    constexpr float CELL_PADDING = 2.f;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 initial_pos = ImGui::GetCursorScreenPos();


    for(unsigned int i{0}; i < N_FLAGS; ++i)
    {
        // position
        unsigned int col = i / N_ROWS;
        unsigned int row = i % N_ROWS;

        ImVec2 p0(initial_pos.x + col * (CELL_BASE + CELL_PADDING), initial_pos.y + row * (CELL_BASE + CELL_PADDING));
        ImVec2 p1(p0.x + CELL_BASE, p0.y + CELL_BASE);

        // color
        bool is_set = m_selected.widget_ptr->isFlag(enum_literals::FLAG_BITS_LITERAL[i].get<1>());
        ImU32 fill_color =  is_set ? SET_CELL_FILL_COLOR : RESET_CELL_FILL_COLOR;

        draw_list->AddRectFilled(p0, p1, fill_color, 3.f);
        draw_list->AddRect(p0, p1, IM_COL32(20, 20, 20, 255), 3.f);

        // tooltip
        ImGui::SetCursorScreenPos(p0);
        ImGui::InvisibleButton(enum_literals::FLAG_BITS_LITERAL[i].get<0>(), ImVec2(CELL_BASE, CELL_BASE));
        if(ImGui::IsItemHovered()) ImGui::SetTooltip("bit %u: %s = %s", i, enum_literals::FLAG_BITS_LITERAL[i].get<0>(), is_set ? "true" : "false");
    }

    // end
    ImGui::SetCursorScreenPos(ImVec2(initial_pos.x, initial_pos.y + N_ROWS * (CELL_BASE + CELL_PADDING)));
    ImGui::Dummy(ImVec2(N_COLS * (CELL_BASE + CELL_PADDING), 0));
    ImGui::EndGroup();

    // space
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(12.f, 0.f));
    ImGui::SameLine();

    /* Configurable flags */

    ImGui::BeginGroup();
    textHeader("Configurable Flags:", false, false, HEADER_3);

    if(ImGui::BeginTable("config_flags", 3, ImGuiTableFlags_SizingStretchSame))
    {
        for(const auto& flag_literal : enum_literals::FLAG_BITS_LITERAL)
        {
            if(!flag_literal.get<2>()) continue;    //!< not modifiable

            ImGui::TableNextColumn();
            bool state = m_selected.widget_ptr->isFlag(flag_literal.get<1>());
            if(ImGui::Checkbox(flag_literal.get<0>(), &state))
            {
                m_selected.widget_ptr->configure(flag_literal.get<1>());
            }
        }

        refreshValues();
        ImGui::EndTable();
    }
    ImGui::EndGroup();
}

void InfoWrapper::renderAccessorSections()
{
    textHeader("Attributes", true, false, HEADER_2);

    const float TAKEN_WIDTH = ImGui::GetContentRegionAvail().x;
    const float RIGHT_COL_START_X = ImGui::GetCursorPosX() + TAKEN_WIDTH * (1 - 0.48f);
    
    float curr_y = ImGui::GetCursorPosY();
    float max_bottom_y = curr_y;
    unsigned int section_count = 0;

    auto append_section = [&](const char* section_label, auto&& render_section)
    {
        if(section_count % 2 == 1)
        {
            ImGui::SetCursorPos(ImVec2(RIGHT_COL_START_X, curr_y));
        }
        else
        {
            ImGui::SetCursorPosY(max_bottom_y);
            curr_y = ImGui::GetCursorPosY();
        }

        ImGui::BeginGroup();
        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Text, SECTION_LABEL_COLOR);
        ImGui::TextUnformatted(section_label);
        ImGui::PopStyleColor();
        render_section();
        ImGui::EndGroup();

        float section_final_y = ImGui::GetCursorPosY() + 6.f;
        if(section_count % 2 == 0) max_bottom_y = section_final_y;
        else max_bottom_y = std::max(max_bottom_y, section_final_y);

        ++section_count;
    };

    for(const auto& section_literal : enum_literals::ENUM_SECTION_LITERAL)
    {
        bool has_keys = std::any_of(m_rows->begin(), m_rows->end(),
            [&](const Accessor_ptr& row)
            {
                return row.attrib.section == section_literal.get<1>();
            });

        if(!has_keys) continue;

        append_section(section_literal.get<0>(), [&]()
        {
            for(Accessor_ptr& row : *m_rows)
            {
                if(row.attrib.section != section_literal.get<1>()) continue;
                ImGui::PushID(row.key.c_str());
                ImGui::TextUnformatted(row.key.c_str());
                renderRow(row);
                ImGui::PopID();
            }
        });
    }

    if(Canvas* canvas = dynamic_cast<Canvas*>(m_selected.widget_ptr))
    {
        auto it = m_cache.find(m_selected.widget_ptr);
        if(it != m_cache.end())
        {
            CanvasCache& cache = it->second.canvas;
            append_section("Canvas (Lines)", [&](){_renderCanvasLineSection(canvas, cache);});
            append_section("Canvas (Cell)",[&](){_renderCanvasCellSection(canvas, cache);});
        }
    }

    ImGui::SetCursorPosY(max_bottom_y);
}

void InfoWrapper::renderShadersSection()
{
    textHeader("Shaders", true, true, HEADER_2);
    auto it = m_cache.find(m_selected.widget_ptr);
    if(it == m_cache.end()) return;

    _addShaderUI("Clip Shader:", ShaderType::Clip, it->second.clip_shader_cache);
    ImGui::Spacing();
    _addShaderUI("Color Shader:", ShaderType::Color, it->second.color_shader_cache);
}

void InfoWrapper::renderToolTipSection()
{
    auto it_cache = m_cache.find(m_selected.widget_ptr);
    if(it_cache == m_cache.end()) return;
    
    textHeader("ToolTip", true, true, HEADER_2);
    
    constexpr std::size_t N_STYLES = enum_literals::TT_STYLE_LITERAL.size();
    const float SECTION_WIDTH = ImGui::GetContentRegionAvail().x * 0.5f;
    ToolTipCache& tooltip_cache = it_cache->second.tooltip;
    bool is_enabled = static_cast<bool>(tooltip_cache.ptr);

    /* THE CHECKBOX */
    if(ImGui::Checkbox("Enable ToolTip", &is_enabled))
    {
        if(is_enabled)
        {
            tooltip_cache.ptr = m_selected.widget_ptr->EnableToolTip();
        }
        else
        {
            m_selected.widget_ptr->DisableToolTip();
            tooltip_cache.ptr = nullptr;
            tooltip_cache.content.clear();
            tooltip_cache.selected_id.reset();
        }
    }

    /* left column */
    ImGui::BeginGroup();
    ImGui::BeginDisabled(!is_enabled);
    textHeader("Parameters", false, false, HEADER_2);

    { // Hover Delay
        textHeader("Delay (ms):", false, false, HEADER_3);
        std::int32_t delay = is_enabled ? tooltip_cache.ptr->getDelay() : 0;
        ImGui::SetNextItemWidth(NUMERIC_INPUT_WIDTH);
        if(ImGui::InputInt("##tt_delayms", &delay) && is_enabled)
        {
            delay = std::max(0, delay);
            tooltip_cache.ptr->setDelay(delay);
        }
    }
    { // Phase Duration
        textHeader("Phase Duration (ms):", false, false, HEADER_3);
        float phase = is_enabled ? tooltip_cache.ptr->getPhaseDuration() : 0.f;
        ImGui::SetNextItemWidth(NUMERIC_INPUT_WIDTH);
        if(ImGui::InputFloat("##tt_phasems", &phase) && is_enabled)
        {
            phase = std::max(0.f, phase);
            tooltip_cache.ptr->setPhaseDuration(phase);
        }
    }
    { // Offset
        textHeader("Offset:", false, false, HEADER_3);
        sf::Vector2f offset = is_enabled ? tooltip_cache.ptr->getOffset() : sf::Vector2f{0.f, 0.f};
        float offset_xy[2]{offset.x, offset.y};
        ImGui::SetNextItemWidth(DOUBLE_INPUT_WIDTH);
        if(ImGui::InputFloat2("##tt_Offset", offset_xy) && is_enabled)
            tooltip_cache.ptr->setOffset(sf::Vector2f(offset_xy[0], offset_xy[1]));
    }
    { // Local Offset
        textHeader("Local Offset:", false, false, HEADER_3);
        sf::Vector2f local_offset = is_enabled ? tooltip_cache.ptr->getLocalOffset() : sf::Vector2f{0.f, 0.f};
        float local_offset_xy[2]{local_offset.x, local_offset.y};
        ImGui::SetNextItemWidth(DOUBLE_INPUT_WIDTH);
        if(ImGui::InputFloat2("##tt_LocalOffset", local_offset_xy) && is_enabled)
            tooltip_cache.ptr->setLocalOffset(sf::Vector2f(local_offset_xy[0], local_offset_xy[1]));
    }
    {
        textHeader("Corner Radius:", false, false, HEADER_3);
        float radius = is_enabled ? tooltip_cache.ptr->getCornerRadius() : 0.f;
        ImGui::SetNextItemWidth(NUMERIC_INPUT_WIDTH);
        if(ImGui::InputFloat("##tt_CornerRad", &radius) && is_enabled)
            tooltip_cache.ptr->setCornerRadius(radius);
    }
    { // Style
        textHeader("ToolTip Style:", false, false, HEADER_3);
        std::size_t& curr_style_idx = tooltip_cache.style_idx;
        ImGui::SetNextItemWidth(COMBO_INPUT_WIDTH);
        if(ImGui::BeginCombo("##tt_StyleSelect", enum_literals::TT_STYLE_LITERAL[curr_style_idx].get<0>()))
        {
            for(std::size_t i{0}; i < N_STYLES; ++i)
            {
                bool is_selected = (i == curr_style_idx);
                if(ImGui::Selectable(enum_literals::TT_STYLE_LITERAL[i].get<0>(), is_selected))
                {
                    if(i != curr_style_idx)
                    {
                        tooltip_cache.ptr->setTheme(enum_literals::TT_STYLE_LITERAL[i].get<1>());
                        curr_style_idx = i;
                    }
                }
                if(is_selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    /* OBJECTS TABLE */
    ImGui::Spacing();
    if(ImGui::Button("Add Text##tt"))
    {
        std::size_t id = tooltip_cache.ptr->createContent(TT_RenderedType::Text, sf::Vector2f{0.f, 0.f});
        tooltip_cache.content.push_back(ToolTipObject{id, TT_RenderedType::Text, {}});
        tooltip_cache.selected_id = id;
    }
    ImGui::SameLine();
    if(ImGui::Button("Add Image##tt"))
    {
        std::size_t id = tooltip_cache.ptr->createContent(TT_RenderedType::Image, sf::Vector2f{0.f, 0.f});
        tooltip_cache.content.push_back(ToolTipObject{id, TT_RenderedType::Image, {}});
        tooltip_cache.selected_id = id;
    }

    ImGui::Spacing();
    ImGui::BeginChild("tt_tree", ImVec2(SECTION_WIDTH, TOOLTIP_TREE_HEIGHT), true);
    if(tooltip_cache.content.empty())
    {
        ImGui::TextDisabled("ID (Type)");
    }
    else
    {
        for(ToolTipObject& rendered_obj : tooltip_cache.content)
        {
            char obj_label[TT_OBJECT_ID_ENTRY_LENGTH];
            std::snprintf(obj_label, sizeof(obj_label), "#%zu (%s)", rendered_obj.id,
                          rendered_obj.type == TT_RenderedType::Text ? "Text" : "Image");
    
            bool is_selected = tooltip_cache.selected_id && *tooltip_cache.selected_id == rendered_obj.id;
            if(ImGui::Selectable(obj_label, is_selected)) tooltip_cache.selected_id = rendered_obj.id;
        }
    }
    ImGui::EndChild();
    ImGui::EndDisabled();
    ImGui::EndGroup();

    /* right column */

    ImGui::SameLine();
    ImGui::Dummy(ImVec2(12.f, 0.f));
    ImGui::SameLine();

    ImGui::BeginGroup();
    ImGui::BeginDisabled(!is_enabled);

    /* Selecting an object */
    ToolTipObject* selected_entry{nullptr};

    if(is_enabled && tooltip_cache.selected_id)
    {
        auto it_selected = std::find_if(tooltip_cache.content.begin(), tooltip_cache.content.end(),
                [&](const ToolTipObject& e)
                {
                    return e.id == *tooltip_cache.selected_id;
                });

        if(it_selected != tooltip_cache.content.end()) selected_entry = &(*it_selected);
    }

    if(selected_entry)
    {
        /* right column start here actually */
        ImGui::TextDisabled("Selected ID: ");
        ImGui::SameLine();
        _renderToolTipEditor(tooltip_cache, *selected_entry);
    }
    ImGui::EndDisabled();
    ImGui::EndGroup();
}

void InfoWrapper::_renderProperties(CacheMap_t::iterator& cached_data)
{
    textHeader("Properties", true, false, HEADER_2);

    ImGui::TextUnformatted("Label");
    ImGui::SameLine();
    char widget_label[WIDGET_LABEL_LENGTH];
    std::snprintf(widget_label, sizeof(widget_label), "%s", cached_data->second.label.c_str());
    ImGui::SetNextItemWidth(TEXT_INPUT_WIDTH);
    if(ImGui::InputText("##widgetlabel", widget_label, sizeof(widget_label)))
    {
        cached_data->second.label = widget_label;
    }

    ImGui::TextUnformatted("Root Label");
    ImGui::SameLine();
    ImGui::TextDisabled("%s", getRootLabel(m_selected.widget_ptr));
}

void InfoWrapper::_renderBindings(CacheMap_t::iterator& cached_data)
{
    textHeader("Bindings", true, false, HEADER_2);

    BuiltinBindFlags& flags_ref = cached_data->second.flags;
    for(const auto& bind_literal : enum_literals::QUICK_BINDINGS_LITERAL)
    {
        bool state = isBuiltinFlagSet(flags_ref, bind_literal.get<1>());
        if(ImGui::Checkbox(bind_literal.get<0>(), &state))
        {
            flags_ref = state ? (flags_ref | bind_literal.get<1>()) : (flags_ref & ~bind_literal.get<1>());
        }
    }
}

void InfoWrapper::_renderTheme(CacheMap_t::iterator& cached_data)
{
    textHeader("Theme", true, false, HEADER_2);

    constexpr std::size_t N_THEMES = enum_literals::THEME_LITERAL.size();
    int& curr_theme_idx = cached_data->second.theme_idx;

    const char* theme_label = (curr_theme_idx >= 0) ?
                enum_literals::THEME_LITERAL[curr_theme_idx].get<0>() :
                "None";

    ImGui::SetNextItemWidth(COMBO_INPUT_WIDTH);
    if(ImGui::BeginCombo("##theme_select", theme_label))
    {
        if(ImGui::Selectable("None", curr_theme_idx == -1))
        {
            m_selected.widget_ptr->unlinkTheme();
        }

        for(int i{0}; i < N_THEMES; ++i)
        {
            bool is_selected = (i == curr_theme_idx);
            if(ImGui::Selectable(enum_literals::THEME_LITERAL[i].get<0>(), is_selected))
            {
                if(i != curr_theme_idx)
                {
                    m_selected.widget_ptr->linkTheme(enum_literals::THEME_LITERAL[i].get<1>());
                    curr_theme_idx = i;
                }
            }
            if(is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::TextDisabled(curr_theme_idx != -1 ? "Theme Linked" : "No theme is currently linked");
}

void InfoWrapper::_renderWidgetGroups()
{

    textHeader("Groups", true, false, HEADER_2);

    for(std::size_t i = 0; i < m_widgetGroups.size(); ++i)
    {
        ImGui::PushID(static_cast<int>(i));

        std::shared_ptr<WidgetGroup>& group = m_widgetGroups[i].group_ptr;
        ImGui::BeginDisabled(!group);
        
        bool is_member = group && group->contains(m_selected.widget_ptr);
        if(is_member) ImGui::PushStyleColor(ImGuiCol_Button, LIME_GREEN);

        if(ImGui::Button(is_member ? "Leave" : "Join", ImVec2(SMALL_BUTTON_WIDTH, 0.f)))
        {
            m_selected.widget_ptr->linkWidget(group);
        }
        if(is_member) ImGui::PopStyleColor();

        if(ImGui::IsItemHovered()) ImGui::SetTooltip(is_member ? "click to leave" : "click to join");

        ImGui::SameLine();
        char group_label[GROUP_LABEL_LENGTH];
        std::snprintf(group_label, sizeof(group_label), "%s", m_widgetGroups[i].group_name.c_str());

        ImGui::SetNextItemWidth(TEXT_INPUT_WIDTH);
        if(ImGui::InputText("##groupname", group_label, sizeof(group_label)))
            m_widgetGroups[i].group_name = group_label;

        if(group)
        {
            ImGui::Checkbox("Focus Gain##FG", &group->groupFocusCallback);
            if(ImGui::IsItemHovered()) ImGui::SetTooltip("Triggers FocusGain event for all group widgets at once if checked");
            ImGui::SameLine();
            ImGui::Checkbox("Focus Loss##FL", &group->groupFocuslossCallback);
            if(ImGui::IsItemHovered()) ImGui::SetTooltip("Triggers FocusLoss event for all group widgets at once if checked");
        }

        ImGui::EndDisabled();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::PopID();
    }
}

void InfoWrapper::_addShaderUI(const char* label_id, ShaderType shader_type, ShaderCache& state)
{
    bool memory_selected{state.mode == Widget::W_LoadType::Memory};

    ImGui::PushID(label_id);
    ImGui::TextUnformatted(label_id);

    ImGui::BeginDisabled(memory_selected);
    if(ImGui::Button("Memory")) state.mode = Widget::W_LoadType::Memory;
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(!memory_selected);
    if(ImGui::Button("File")) state.mode = Widget::W_LoadType::File;
    ImGui::EndDisabled();

    if(state.mode == Widget::W_LoadType::Memory)
    {
        constexpr std::size_t N = enum_literals::BUILTIN_SHADERS_LITERAL.size();
        bool no_shader = (state.combo_idx < 0);
        const char* preview = no_shader ? "None" : enum_literals::BUILTIN_SHADERS_LITERAL[state.combo_idx].get<0>();

        ImGui::SetNextItemWidth(COMBO_INPUT_WIDTH);

        if(ImGui::BeginCombo("##comboshader", preview))
        {
            if(ImGui::Selectable("None", no_shader)) state.combo_idx = -1;
            if(no_shader) ImGui::SetItemDefaultFocus();

            for(int i{0}; i < N; i++)
            {
                bool selected_option = (state.combo_idx == i);
                if(ImGui::Selectable(enum_literals::BUILTIN_SHADERS_LITERAL[i].get<0>(), selected_option))
                {
                    state.combo_idx = i;
                }
                if(selected_option) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }
    else
    {
        ImGui::SetNextItemWidth(PATH_INPUT_WIDTH);
        ImGui::InputTextWithHint("##pathshader", "Path to Shader", state.file_path.data(), state.file_path.size());

        ImGui::SameLine();
        ImGui::BeginDisabled(m_dialog.is_open.load());
        if(ImGui::Button("...##browse")) openFileDialog(label_id, "Open Shader", SHADER_EXTENSIONS, N_SHADER_EXTENSIONS, "Shader files");
        ImGui::EndDisabled();

        if(m_dialog.safe_path && m_dialog.req_key == label_id && m_selectToken == m_dialog.req_token)
        {
            std::snprintf(state.file_path.data(), state.file_path.size(), "%s", m_dialog.safe_path->c_str());
            m_dialog.safe_path.reset();
        }
    }

    ImGui::SameLine();
    ImGui::SetCursorPosX(COMBO_INPUT_WIDTH + 12.f);
    if(ImGui::Button("Load##loadshader"))
    {
        if(shader_type == ShaderType::Clip)
        {
            if(state.mode == Widget::W_LoadType::Memory)
            {
                if(state.combo_idx < 0) m_selected.widget_ptr->resetClipShader();
                else m_selected.widget_ptr->loadClipShader(enum_literals::BUILTIN_SHADERS_LITERAL[state.combo_idx].get<1>(), state.mode);
            }
            else
            {
                if(state.file_path[0] == '\0') m_selected.widget_ptr->resetClipShader();
                else m_selected.widget_ptr->loadClipShader(state.file_path.data(), Widget::W_LoadType::File);
            }
        }
        else
        {
            if(state.mode == Widget::W_LoadType::Memory)
            {
                if(state.combo_idx < 0) m_selected.widget_ptr->resetColorShader();
                else m_selected.widget_ptr->loadColorShader(enum_literals::BUILTIN_SHADERS_LITERAL[state.combo_idx].get<1>(), state.mode);
            }
            else
            {
                if(state.file_path[0] == '\0') m_selected.widget_ptr->resetColorShader();
                else m_selected.widget_ptr->loadColorShader(state.file_path.data(), Widget::W_LoadType::File);
            }
        }
    }
}

void InfoWrapper::_renderToolTipEditor(ToolTipCache& tooltip_cache, ToolTipObject& selected_obj)
{
    ImGui::PushID(static_cast<int>(selected_obj.id));
    
    /* TITLE */
    ImGui::TextDisabled("%zu", selected_obj.id);
    ImGui::SameLine(0.f, 12.f);
    ImGui::PushStyleColor(ImGuiCol_Button, RED);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, DARK_RED);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, DARKER_RED);
    if(ImGui::SmallButton("Delete Object##tt"))
    {
        std::size_t deleted_id = selected_obj.id;
        (void)tooltip_cache.ptr->deleteContent(deleted_id);
        tooltip_cache.content.erase(std::remove_if(tooltip_cache.content.begin(), tooltip_cache.content.end(),
                [&](const ToolTipObject& e)
                {
                    return e.id == deleted_id;
                }), tooltip_cache.content.end());

        tooltip_cache.selected_id.reset();
    }
    ImGui::PopStyleColor(3);

    /* Accessors */

    if(selected_obj.type == TT_RenderedType::Text)
    {
        ToolTip::RenderedText* text_ptr = tooltip_cache.ptr->getText(selected_obj.id);
        if(!text_ptr)
        {
            ImGui::PopID();
            return;
        }

        { // Text
            textHeader("Text:", false, false, HEADER_3);
            std::string text = text_ptr->getText().toAnsiString();
            char text_buf[TT_TEXT_OBJECT_LENGTH];
            std::snprintf(text_buf, sizeof(text_buf), "%s", text.c_str());
            if(ImGui::InputTextMultiline("##tt_TextGet", text_buf, sizeof(text_buf), MULTITEXT_INPUT_DIMENSIONS))
                text_ptr->setText(text_buf);
        }

        { // Text Position
            textHeader("Text Position:", false, false, HEADER_3);
            sf::Vector2f pos = text_ptr->getPosition();
            float pos_xy[2]{pos.x, pos.y};
            ImGui::SetNextItemWidth(DOUBLE_INPUT_WIDTH);
            if(ImGui::InputFloat2("##tt_TextPos", pos_xy))
                text_ptr->setPosition(sf::Vector2f(pos_xy[0], pos_xy[1]));
        }

        { // Char size
            textHeader("Char Size:", false, false, HEADER_3);
            unsigned int char_size = text_ptr->getCharacterSize();
            ImGui::SetNextItemWidth(NUMERIC_INPUT_WIDTH);
            if(ImGui::InputScalar("##tt_TextSize", ImGuiDataType_U32, &char_size))
                text_ptr->setCharacterSize(char_size);
        }

        { // Text Line Spacing
            textHeader("Line Spacing:", false, false, HEADER_3);
            float line_spacing = text_ptr->getLineSpacing();
            ImGui::SetNextItemWidth(NUMERIC_INPUT_WIDTH);
            if(ImGui::InputFloat("##tt_TextLineSpacing", &line_spacing))
                text_ptr->setLineSpacing(line_spacing);
        }

        { // Text Letter Spacing
            textHeader("Letter Spacing:", false, false, HEADER_3);
            float letter_spacing = text_ptr->getLetterSpacing();
            ImGui::SetNextItemWidth(NUMERIC_INPUT_WIDTH);
            if(ImGui::InputFloat("##tt_TextLetterSpacing", &letter_spacing))
                text_ptr->setLetterSpacing(letter_spacing);
        }

        { // Text Fill Color
            textHeader("Fill Color:", false, false, HEADER_3);
            float fill[4];
            toFloatColor(text_ptr->getFillColor(), fill);
            if(ImGui::ColorEdit4("##tt_TextColor", fill, ImGuiColorEditFlags_NoInputs))
                text_ptr->setFillColor(toSfColor(fill));
        }

        { // Text Outline Color
            textHeader("Outline Color:", false, false, HEADER_3);
            float outline[4];
            toFloatColor(text_ptr->getOutlineColor(), outline);
            if(ImGui::ColorEdit4("##tt_TextOutlineColor", outline, ImGuiColorEditFlags_NoInputs))
                text_ptr->setOutlineColor(toSfColor(outline));
        }

        { // Text Outline Thickness
            textHeader("Outline Thickness:", false, false, HEADER_3);
            unsigned int outline_thickness = text_ptr->getOutlineThickness();
            ImGui::SetNextItemWidth(NUMERIC_INPUT_WIDTH);
            if(ImGui::InputScalar("##tt_TextOutlineThickness", ImGuiDataType_U32, &outline_thickness))
                text_ptr->setOutlineThickness(outline_thickness);
        }

        { // Text Style
            textHeader("Text Style:", false, false, HEADER_3);
            int style_flags = static_cast<int>(text_ptr->getStyle());
            bool style_changed = false;
            constexpr std::size_t N = enum_literals::TEXT_STYLE_LITERAL.size();
            for(std::size_t i = 0; i < N; ++i)
            {
                if(i % 2 == 1) ImGui::SameLine(ENUM_INPUT_WIDTH);
                style_changed |= ImGui::CheckboxFlags(enum_literals::TEXT_STYLE_LITERAL[i].get<0>(), &style_flags,
                                 static_cast<int>(enum_literals::TEXT_STYLE_LITERAL[i].get<1>()));

            }
            if(style_changed) text_ptr->setStyle(static_cast<sf::Text::Style>(style_flags));
        }
    }
    else
    {
        ToolTip::RenderedImage* image_ptr = tooltip_cache.ptr->getImage(selected_obj.id);
        if(!image_ptr)
        {
            ImGui::PopID();
            return;
        }

        { // Image Position
            textHeader("Image Position:", false, false, HEADER_3);
            sf::Vector2f pos = image_ptr->getPosition();
            float pos_xy[2]{pos.x, pos.y};
            ImGui::SetNextItemWidth(DOUBLE_INPUT_WIDTH);
            if(ImGui::InputFloat2("##tt_ImagePos", pos_xy))
                image_ptr->setPosition(sf::Vector2f(pos_xy[0], pos_xy[1]));
        }

        { // Image Size
            textHeader("Resize Image:", false, false, HEADER_3);
            sf::Vector2u size = image_ptr->getSize();
            int size_xy[2]{static_cast<int>(size.x), static_cast<int>(size.y)};
            ImGui::SetNextItemWidth(DOUBLE_INPUT_WIDTH);
            if(ImGui::InputInt2("##tt_ImageResize", size_xy, ImGuiInputTextFlags_EnterReturnsTrue))
                image_ptr->setSize(sf::Vector2u(size_xy[0], size_xy[1]));
        }

        { // setImage
            textHeader("Load Image:", false, false, HEADER_3);
            std::string dialog_key = "tt_" + std::to_string(selected_obj.id);

            ImGui::BeginDisabled(m_dialog.is_open.load());
            if(ImGui::Button("Browse##tt"))
                openFileDialog(dialog_key, "Open Image", IMG_EXTENSIONS, N_IMG_EXTENSIONS, "Image files");
            ImGui::EndDisabled();

            if(m_dialog.safe_path && m_dialog.req_key == dialog_key && m_selectToken == m_dialog.req_token)
            {
                selected_obj.pending_image_path = *m_dialog.safe_path;
                m_dialog.safe_path.reset();
            }

            ImGui::SameLine();
            int size_xy[2]{static_cast<int>(selected_obj.image_size_param.x), static_cast<int>(selected_obj.image_size_param.y)};
            ImGui::SetNextItemWidth(DOUBLE_INPUT_WIDTH);
            if(ImGui::InputInt2("##tt_ImageSize", size_xy))
            {
                selected_obj.image_size_param.x = std::max(0, size_xy[0]);
                selected_obj.image_size_param.y = std::max(0, size_xy[1]);
            }
        
            ImGui::SameLine();
            
            ImGui::BeginDisabled(selected_obj.pending_image_path.empty());
            if(ImGui::Button("Load##tt"))
            {
                image_ptr->setImage(sf::String(selected_obj.pending_image_path), selected_obj.image_size_param);
                selected_obj.pending_image_path.clear();
                selected_obj.image_size_param = {0, 0};
            }
            ImGui::EndDisabled();
        }

        // Clear Image
        if(ImGui::Button("Clear Image##tt"))
        {
            image_ptr->clearImage();
        }
    }
    ImGui::PopID();
}

void InfoWrapper::renderRow(Accessor_ptr& row)
{
    const bool CAN_BE_MODIFIED = static_cast<bool>(row.setter);

    // for rows with a setter only
    if(!row.attrib.val.has_value() && CAN_BE_MODIFIED)
    {
        ImGui::SetNextItemWidth(UNKNOWN_INPUT_WIDTH);
        if(ImGui::InputTextWithHint("##key", row.attrib.hint.has_value() ? (*row.attrib.hint).c_str() : "Any Value",
                row.attrib.text_content.data(), row.attrib.text_content.size(), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            trySetFromText(row.key, row.attrib.text_content.data());
            if(!row.attrib.is_path) row.attrib.text_content.fill('\0');
        }
        if(row.attrib.is_path)
        {
            ImGui::SameLine();
            ImGui::BeginDisabled(m_dialog.is_open.load());
            if(ImGui::Button("...##browse")) openFileDialog(row.key, "Open Image", IMG_EXTENSIONS, N_IMG_EXTENSIONS, "Image files");
            ImGui::EndDisabled();

            if(m_dialog.safe_path && row.key == m_dialog.req_key && m_selectToken == m_dialog.req_token)
            {
                std::snprintf(row.attrib.text_content.data(), row.attrib.text_content.size(), "%s", m_dialog.safe_path->c_str());
                trySetFromText(row.key, row.attrib.text_content.data());
                m_dialog.safe_path.reset();
            }
        }
        if(row.resetter)
        {
            ImGui::SameLine(0.f, 12.f);
            if(ImGui::SmallButton("Reset##key"))
            {
                m_selected.widget_info_ptr.info_Reset(m_selected.widget_info_ptr.m_wrappedWidget, row.key);
                row.attrib.text_content.fill('\0');
                refreshValues();
            }
        }
        return;
    }

    if(!row.attrib.val.has_value())
    {
        ImGui::TextDisabled("(pending)");
        return;
    }

    bool was_modified = false;

    if(!CAN_BE_MODIFIED) ImGui::BeginDisabled();

    std::visit(visitor{
        [&](bool& v)
        {
            was_modified = ImGui::Checkbox("##key", &v);
        },
        [&](char& v)
        {
            row.attrib.text_content[0] = v;
            row.attrib.text_content[1] = '\0';
            ImGui::SetNextItemWidth(CHAR_INPUT_WIDTH);
            if(ImGui::InputText("##key", row.attrib.text_content.data(), 2))
            {
                v = row.attrib.text_content[0];
                was_modified = true;
            }
        },
        [&](float& v)
        {
            ImGui::SetNextItemWidth(NUMERIC_INPUT_WIDTH);
            if(row.attrib.frange.has_value())
                was_modified = ImGui::SliderFloat("##key", &v, row.attrib.frange->x, row.attrib.frange->y, "%.3f");
            else
                was_modified = ImGui::InputFloat("##key", &v);
        },
        [&](unsigned int& v)
        {
            ImGui::SetNextItemWidth(NUMERIC_INPUT_WIDTH);
            if(row.attrib.uirange.has_value())
            {
                was_modified = ImGui::SliderScalar("##key", ImGuiDataType_U32, &v,
                    &row.attrib.uirange->x, &row.attrib.uirange->y);
            }
            else
            {
                was_modified = ImGui::InputScalar("##key", ImGuiDataType_U32, &v);
            }
        },
        [&](std::size_t& v)
        {
            ImGui::SetNextItemWidth(NUMERIC_INPUT_WIDTH);
            was_modified = ImGui::InputScalar("##key", ImGuiDataType_U64, &v);
        },
        [&](std::string& v)
        {
            std::snprintf(row.attrib.text_content.data(), row.attrib.text_content.size(), "%s", v.c_str());
            ImGui::SetNextItemWidth(TEXT_INPUT_WIDTH);
            if(ImGui::InputText("##key", row.attrib.text_content.data(), row.attrib.text_content.size()))
            {
                v = row.attrib.text_content.data();
                row.attrib.text_content.fill('\0');
                was_modified = true;
            }
        },
        [&](Button::SIDE& v)
        {
            constexpr std::size_t N = enum_literals::BUTTON_SIDE_LITERAL.size();
            int idx = std::clamp(static_cast<int>(v), 0, static_cast<int>(N) - 1);
            ImGui::SetNextItemWidth(ENUM_INPUT_WIDTH);
            if(ImGui::InputInt("##key", &idx))
            {
                idx = std::clamp(idx, 0, static_cast<int>(N) - 1);
                v = static_cast<Button::SIDE>(idx);
                was_modified = true;
            }
            ImGui::SameLine();
            ImGui::Text("%s selected", enum_literals::BUTTON_SIDE_LITERAL[idx].get<0>());
        },
        [&](Frame::F_SeparatorType& v)
        {
            constexpr std::size_t N = enum_literals::SEPARATOR_TYPE_LITERAL.size();
            int idx = std::clamp(static_cast<int>(v), 0, static_cast<int>(N) - 1);
            ImGui::SetNextItemWidth(ENUM_INPUT_WIDTH);
            if(ImGui::InputInt("##key", &idx))
            {
                idx = std::clamp(idx, 0, static_cast<int>(N) - 1);
                v = static_cast<Frame::F_SeparatorType>(idx);
                was_modified = true;
            }
            ImGui::SameLine();
            ImGui::Text("%s selected", enum_literals::SEPARATOR_TYPE_LITERAL[idx].get<0>());
        },
        [&](Frame::F_AfterPreProcess& v)
        {
            constexpr std::size_t N = enum_literals::AFTER_PREPROCESS_LITERAL.size();
            int idx = std::clamp(static_cast<int>(v), 0, static_cast<int>(N) - 1);
            ImGui::SetNextItemWidth(ENUM_INPUT_WIDTH);
            if(ImGui::InputInt("##key", &idx))
            {
                idx = std::clamp(idx, 0, static_cast<int>(N) - 1);
                v = static_cast<Frame::F_AfterPreProcess>(idx);
                was_modified = true;
            }
            ImGui::SameLine();
            ImGui::Text("%s selected", enum_literals::AFTER_PREPROCESS_LITERAL[idx].get<0>());
        },
        [&](DIRECTION& v)
        {
            constexpr std::size_t N = enum_literals::DIRECTION_LITERAL.size();
            int idx = std::clamp(static_cast<int>(v), 0, static_cast<int>(N) - 1);
            ImGui::SetNextItemWidth(ENUM_INPUT_WIDTH);
            if(ImGui::InputInt("##key", &idx))
            {
                idx = std::clamp(idx, 0, static_cast<int>(N) - 1);
                v = static_cast<DIRECTION>(idx);
                was_modified = true;
            }
            ImGui::SameLine();
            ImGui::Text("%s selected", enum_literals::DIRECTION_LITERAL[idx].get<0>());
        },
        [&](Label::HAlign& v)
        {
            constexpr std::size_t N = enum_literals::HALIGN_LITERAL.size();
            int idx = std::clamp(static_cast<int>(v), 0, static_cast<int>(N) - 1);
            ImGui::SetNextItemWidth(ENUM_INPUT_WIDTH);
            if(ImGui::InputInt("##key", &idx))
            {
                idx = std::clamp(idx, 0, static_cast<int>(N) - 1);
                v = static_cast<Label::HAlign>(idx);
                was_modified = true;
            }
            ImGui::SameLine();
            ImGui::Text("%s selected", enum_literals::HALIGN_LITERAL[idx].get<0>());
        },
        [&](Label::VAlign& v)
        {
            constexpr std::size_t N = enum_literals::VALIGN_LITERAL.size();
            int idx = std::clamp(static_cast<int>(v), 0, static_cast<int>(N) - 1);
            ImGui::SetNextItemWidth(ENUM_INPUT_WIDTH);
            if(ImGui::InputInt("##key", &idx))
            {
                idx = std::clamp(idx, 0, static_cast<int>(N) - 1);
                v = static_cast<Label::VAlign>(idx);
                was_modified = true;
            }
            ImGui::SameLine();
            ImGui::Text("%s selected", enum_literals::VALIGN_LITERAL[idx].get<0>());
        },
        [&](sf::Text::Style& v)
        {
            int flags = static_cast<int>(v);
            std::size_t N = enum_literals::TEXT_STYLE_LITERAL.size();
            for(std::size_t i = 0; i < N; ++i)
            {
                was_modified |= ImGui::CheckboxFlags(enum_literals::TEXT_STYLE_LITERAL[i].get<0>(), &flags, static_cast<int>(enum_literals::TEXT_STYLE_LITERAL[i].get<1>()));
                if(i + 1 < N) ImGui::SameLine();
            }
            if(was_modified) v = static_cast<sf::Text::Style>(flags);
        },
        [&](Frame::F_SeparatorInfo& v)
        {
            constexpr std::size_t N = enum_literals::SEPARATOR_TYPE_LITERAL.size();
            int type_idx = std::clamp(static_cast<int>(v.type), 0, static_cast<int>(N) - 1);

            ImGui::SetNextItemWidth(NUMERIC_INPUT_WIDTH);
            if(ImGui::InputInt("##sep_type", &type_idx))
            {
                type_idx = std::clamp(type_idx, 0, static_cast<int>(N) - 1);
                v.type = static_cast<Frame::F_SeparatorType>(type_idx);
                was_modified = true;
            }
            ImGui::SameLine();
            ImGui::Text("%s selected", enum_literals::SEPARATOR_TYPE_LITERAL[type_idx].get<0>());

            ImGui::SetNextItemWidth(NUMERIC_INPUT_WIDTH);
            if(ImGui::InputFloat("##sep_ratio", &v.ratio)) was_modified = true;
            ImGui::SameLine();
            ImGui::TextUnformatted("ratio");

            ImGui::SetNextItemWidth(NUMERIC_INPUT_WIDTH);
            if(ImGui::InputFloat("##sep_dist", &v.dist)) was_modified = true;
            ImGui::SameLine();
            ImGui::TextUnformatted("dist");
        },
        [&](Cell* v)
        {
            if(v) ImGui::Text("Selected Cell Coords:(%u, %u)", v->getCellCoordinates().x, v->getCellCoordinates().y);
            else ImGui::TextUnformatted("No Cell Selected.");
        },
        [&](sf::String& v)
        {
            std::string str = v.toAnsiString();
            std::snprintf(row.attrib.text_content.data(), row.attrib.text_content.size(), "%s", str.c_str());
            ImGui::SetNextItemWidth(row.attrib.is_path ? PATH_INPUT_WIDTH : TEXT_INPUT_WIDTH);
            if(ImGui::InputText("##key", row.attrib.text_content.data(), row.attrib.text_content.size(), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                v = sf::String(row.attrib.text_content.data());
                if(!row.attrib.is_path) row.attrib.text_content.fill('\0');
                was_modified = true;
            }
            if(row.attrib.is_path)
            {
                ImGui::SameLine();
                ImGui::BeginDisabled(m_dialog.is_open.load());
                if(ImGui::Button("...##browse")) openFileDialog(row.key, "Open Image", IMG_EXTENSIONS, N_IMG_EXTENSIONS, "Image files");
                ImGui::EndDisabled();

                if(m_dialog.safe_path && row.key == m_dialog.req_key && m_selectToken == m_dialog.req_token)
                {
                    v = sf::String(*m_dialog.safe_path);
                    was_modified = true;
                    m_dialog.safe_path.reset();
                }
            }
        },
        [&](sf::FloatRect& v)
        {
            ImGui::Text("pos(%.1f, %.1f) size(%.1f, %.1f)", v.position.x, v.position.y, v.size.x, v.size.y);
        },
        [&](sf::Angle& v)
        {
            float d = v.asDegrees();
            ImGui::SetNextItemWidth(NUMERIC_INPUT_WIDTH);
            if(ImGui::SliderFloat("##key (deg)", &d, -180.f, 180.f, "%.1f"))
            {
                v = sf::degrees(d);
                was_modified = true;
            }
        },
        [&](sf::Vector2f& v)
        {
            float xy[2]{v.x, v.y};
            ImGui::SetNextItemWidth(DOUBLE_INPUT_WIDTH);
            if(ImGui::InputFloat2("##key", xy, "%.1f"))
            {
                v = sf::Vector2f(xy[0], xy[1]);
                was_modified = true;
            }
        },
        [&](sf::Vector2u& v)
        {
            int xy[2]{static_cast<int>(v.x), static_cast<int>(v.y)};
            ImGui::SetNextItemWidth(DOUBLE_INPUT_WIDTH);
            if(ImGui::InputInt2("##key", xy))
            {
                v = sf::Vector2u(static_cast<unsigned int>(std::max(0, xy[0])),
                                 static_cast<unsigned int>(std::max(0, xy[1])));
                was_modified = true;
            }
        },
        [&](sf::Vector2i& v)
        {
            int xy[2]{v.x, v.y};
            ImGui::SetNextItemWidth(DOUBLE_INPUT_WIDTH);
            if(ImGui::InputInt2("##key", xy))
            {
                v = sf::Vector2i(xy[0], xy[1]);
                was_modified = true;
            }
        },
        [&](std::optional<sf::Vector2u>& v)
        {
            if(v.has_value()) ImGui::Text("(%u, %u)", v->x, v->y);
            else ImGui::TextUnformatted("(Nan, Nan)");
        },
        [&](Canvas::cell_ptr p)
        {
            if(p) ImGui::Text("Cell ID: %zu", p->getID());
            else ImGui::TextUnformatted("None");
        },
        [&](const sf::RenderWindow* p)
        {
            if(p) ImGui::Text("Window Address: %p", p);
            else ImGui::TextUnformatted("NULL.");
        },
        [&](const Frame* p)
        {
            auto it = m_cache.find(static_cast<const Widget*>(p));
            if(it != m_cache.end()) ImGui::Text("Root Frame: %s", it->second.label.c_str());
            else ImGui::TextUnformatted("No Frame Root Available.");
        }
        }, *row.attrib.val);

    if(!CAN_BE_MODIFIED)
    {
        ImGui::EndDisabled();
    }
    else if(row.resetter)
    {
        ImGui::SameLine(0.f, 12.f);
        if(ImGui::SmallButton("Reset##key"))
        {
            m_selected.widget_info_ptr.info_Reset(m_selected.widget_info_ptr.m_wrappedWidget, row.key);
            refreshValues();
        }
    }

    if(was_modified && CAN_BE_MODIFIED)
    {
        m_selected.widget_info_ptr.info_Set(m_selected.widget_info_ptr.m_wrappedWidget, row.key, *row.attrib.val);
        refreshValues();
    }
}

void InfoWrapper::renderFps() const
{
    if(m_revealFps)
    {
        ImGui::SameLine();
        char fps_text[FPS_TEXT_LENGTH];
        std::snprintf(fps_text, sizeof(fps_text), FPS_TEXT_FORMAT, ImGui::GetIO().Framerate);
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(fps_text).x);
        ImGui::TextUnformatted(fps_text);
    }
}

void InfoWrapper::pullDialogPath()
{
    if(m_dialog.path_found.exchange(false))
    {
        std::lock_guard<std::mutex> lock(m_dialog.guard);
        m_dialog.safe_path = m_dialog.protected_path;
        m_dialog.protected_path.clear();
    }
}

const char* InfoWrapper::getRootLabel(Widget* w) const
{
    if(!w) return NULL_TEXT;
    if(w->getRootWindow()) return ROOT_WIN_TEXT;

    const Frame* root = w->getRootFrame();
    if(!root) return NULL_TEXT;

    auto it = m_cache.find(static_cast<const Widget*>(root));
    if(it == m_cache.end()) return NULL_TEXT;
    return it->second.label.c_str();
}

void InfoWrapper::_renderCanvasLineSection(Canvas* canvas, CanvasCache& cache)
{
    bool draw_button_clicked{false}, remove_button_clicked{false}, thickness_button_clicked{false},
         color_button_clicked{false}, type_button_clicked{false};

    ImGui::TextUnformatted("Unit Range {x0, y0, x1, y1}");
    int range[4]{static_cast<int>(cache.line_x0), static_cast<int>(cache.line_y0),
                 static_cast<int>(cache.line_x1), static_cast<int>(cache.line_y1)};
    ImGui::SetNextItemWidth(UNKNOWN_INPUT_WIDTH);
    if(ImGui::InputInt4("##canvas_line_range", range))
    {
        cache.line_x0 = static_cast<unsigned int>(std::max(0, range[0]));
        cache.line_y0 = static_cast<unsigned int>(std::max(0, range[1]));
        cache.line_x1 = static_cast<unsigned int>(std::max(0, range[2]));
        cache.line_y1 = static_cast<unsigned int>(std::max(0, range[3]));
    }
    if(ImGui::Button("Draw Line")) draw_button_clicked = true;
    ImGui::SameLine();
    if(ImGui::Button("Remove Line")) remove_button_clicked = true;
    ImGui::Spacing();

    ImGui::TextUnformatted("Line Thickness");
    int thickness = static_cast<int>(cache.line_thickness);
    ImGui::SetNextItemWidth(NUMERIC_INPUT_WIDTH);
    if(ImGui::SliderInt("##canvas_line_thickness", &thickness, 0, 7))
    {
        cache.line_thickness = static_cast<unsigned int>(thickness);
    }
    ImGui::SameLine();
    if(ImGui::Button("Set Thickness")) thickness_button_clicked = true;
    ImGui::Spacing();

    ImGui::Checkbox("Use Canvas Outline Color##canvas_line_defcolor", &cache.line_use_default_color);
    ImGui::BeginDisabled(cache.line_use_default_color);
    ImGui::ColorEdit4("Color##canvas_line_color", cache.line_color, ImGuiColorEditFlags_NoInputs);
    ImGui::SameLine();
    if(ImGui::Button("Set Color")) color_button_clicked = true;
    ImGui::EndDisabled();
    ImGui::Spacing();

    ImGui::TextUnformatted("Line Type");
    ImGui::SetNextItemWidth(ENUM_INPUT_WIDTH);
    if(ImGui::BeginCombo("##canvas_line_type", enum_literals::LINE_TYPE_LITERAL[static_cast<std::size_t>(cache.line_type_idx)].get<0>()))
    {
        for(std::size_t i = 0; i < enum_literals::LINE_TYPE_LITERAL.size(); ++i)
        {
            bool is_selected = (cache.line_type_idx == static_cast<int>(i));
            if(ImGui::Selectable(enum_literals::LINE_TYPE_LITERAL[i].get<0>(), is_selected)) cache.line_type_idx = static_cast<int>(i);
            if(is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    if(ImGui::Button("Set Type")) type_button_clicked = true;
    ImGui::Spacing();

    RectRange<unsigned int> range_val{cache.line_x0, cache.line_y0, cache.line_x1, cache.line_y1};
    sf::Color color = cache.line_use_default_color ? DEFAULT_COLOR : toSfColor(cache.line_color);
    LineType type = static_cast<LineType>(cache.line_type_idx);
    if(draw_button_clicked) canvas->drawLine(range_val, static_cast<LineState>(cache.line_thickness), color, type);
    if(remove_button_clicked) canvas->removeLine(range_val);
    if(thickness_button_clicked) canvas->setLineThickness(range_val, static_cast<LineState>(cache.line_thickness));
    if(color_button_clicked) canvas->setLineColor(range_val, color);
    if(type_button_clicked) canvas->setLineType(range_val, type);

    ImGui::PushStyleColor(ImGuiCol_Button, RED);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, DARK_RED);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, DARKER_RED);
    if(ImGui::Button("Clear Canvas")) canvas->clearCanvas();
    ImGui::PopStyleColor(3);
}

void InfoWrapper::_renderCanvasCellSection(Canvas* canvas, CanvasCache& cache)
{
    Canvas::cell_ptr selected = canvas->getSelectedCell();
    if(!selected)
    {
        ImGui::TextDisabled("No cell currently selected.");
        return;
    }

    constexpr std::size_t DIRECTIONS_COUNT = enum_literals::DIRECTION_LITERAL.size();

    ImGui::Text("Cell ID: %zu", selected->getID());

    ImGui::Spacing();

    ImGui::TextUnformatted("Selected Line Side");
    ImGui::SetNextItemWidth(ENUM_INPUT_WIDTH);
    if(ImGui::BeginCombo("##canvas_cell_side", enum_literals::DIRECTION_LITERAL[static_cast<std::size_t>(cache.cell_side_idx)].get<0>()))
    {
        for(std::size_t i = 0; i < DIRECTIONS_COUNT; ++i)
        {
            bool is_selected = (cache.cell_side_idx == static_cast<int>(i));
            if(ImGui::Selectable(enum_literals::DIRECTION_LITERAL[i].get<0>(), is_selected)) cache.cell_side_idx = static_cast<int>(i);
            if(is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    DIRECTION side = static_cast<DIRECTION>(cache.cell_side_idx);

    ImGui::Spacing();

    int outline_thickness = static_cast<int>(cache.cell_outline_thickness);
    ImGui::TextUnformatted("Outline Thickness");
    ImGui::SetNextItemWidth(NUMERIC_INPUT_WIDTH);
    if(ImGui::SliderInt("##canvas_cell_outlinethick", &outline_thickness, 0, 7))
    {
        cache.cell_outline_thickness = static_cast<unsigned int>(outline_thickness);
    }
    ImGui::SameLine();
    if(ImGui::Button("Set Line Thickness##canvas_cell_setthickness"))
    {
        selected->setOutlineThickness(side, static_cast<LineState>(cache.cell_outline_thickness));
    }

    ImGui::Spacing();

    ImGui::Checkbox("Use Canvas Outline Color##canvas_cell_defoutline", &cache.cell_use_default_outline_color);
    ImGui::BeginDisabled(cache.cell_use_default_outline_color);
    ImGui::ColorEdit4("Outline Color##canvas_cell_outlinecolor", cache.cell_outline_color, ImGuiColorEditFlags_NoInputs);
    ImGui::SameLine();
    if(ImGui::Button("Set Outline COlor##canvas_cell_setoutlinecolor"))
    {
        sf::Color c = cache.cell_use_default_outline_color ? DEFAULT_COLOR : toSfColor(cache.cell_outline_color);
        selected->setOutlineColor(side, c);
    }
    ImGui::EndDisabled();

    ImGui::Spacing();

    ImGui::TextUnformatted("Outline Type");
    ImGui::SetNextItemWidth(ENUM_INPUT_WIDTH);
    if(ImGui::BeginCombo("##canvas_cell_outlinetype", enum_literals::LINE_TYPE_LITERAL[static_cast<std::size_t>(cache.cell_outline_type_idx)].get<0>()))
    {
        for(std::size_t i = 0; i < enum_literals::LINE_TYPE_LITERAL.size(); ++i)
        {
            bool is_selected = (cache.cell_outline_type_idx == static_cast<int>(i));
            if(ImGui::Selectable(enum_literals::LINE_TYPE_LITERAL[i].get<0>(), is_selected)) cache.cell_outline_type_idx = static_cast<int>(i);
            if(is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    if(ImGui::Button("Set Outline Type##canvas_cell_setoutlinetype"))
    {
        selected->setOutlineType(side, static_cast<LineType>(cache.cell_outline_type_idx));
    }

    ImGui::Spacing();

    ImGui::TextUnformatted("Cell Fill Color");
    ImGui::ColorEdit4("Color##canvas_cell_fill", cache.cell_fill_color, ImGuiColorEditFlags_NoInputs);
    ImGui::SameLine();
    if(ImGui::Button("Set Color##canvas_cell_setfill"))
    {
        selected->setCellColor(toSfColor(cache.cell_fill_color));
    }

    ImGui::Spacing();

    ImGui::TextUnformatted("Cell Texture");
    ImGui::SetNextItemWidth(PATH_INPUT_WIDTH);
    ImGui::InputTextWithHint("##canvas_cell_texpath", "image path", cache.cell_texture_path.data(), cache.cell_texture_path.size());
    ImGui::SameLine();
    ImGui::BeginDisabled(m_dialog.is_open.load());
    if(ImGui::Button("Browse##canvas_cell_texbrowse"))
    {
        openFileDialog("canvas_cell_texture", "Open Image", IMG_EXTENSIONS, N_IMG_EXTENSIONS, "Image files");
    }
    ImGui::EndDisabled();

    if(m_dialog.safe_path && m_dialog.req_key == "canvas_cell_texture" && m_selectToken == m_dialog.req_token)
    {
        std::snprintf(cache.cell_texture_path.data(), cache.cell_texture_path.size(), "%s", m_dialog.safe_path->c_str());
        m_dialog.safe_path.reset();
    }

    if(ImGui::Button("Load##canvas_cell_texload"))
    {
        selected->setCellTexture(cache.cell_texture_path.data());
    }
    ImGui::SameLine();
    if(ImGui::Button("Clear##canvas_cell_texclear"))
    {
        selected->clearCellTexture();
    }
}
}

#endif