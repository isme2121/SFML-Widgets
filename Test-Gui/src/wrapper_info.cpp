#include "wrapper_info.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>

#ifdef WRAPPER_DEFINED

namespace
{
    constexpr ImVec4 HEADER_1{0.65f, 0.65f, 0.65f, 1.0f};   // gray
    constexpr ImVec4 HEADER_2{0.35f, 0.85f, 0.45f, 1.0f};   // green
    constexpr ImVec4 HEADER_3{0.55f, 0.75f, 1.0f, 1.0f};    // blue
    constexpr const char* IMG_EXTENSIONS[] = {"*.png", "*.jpg", "*.jpeg"};
    constexpr const char* SHADER_EXTENSIONS[] = {"*.frag", "*.glsl"};
}

namespace sfml_widgets
{
InfoWrapper::InfoWrapper(sf::RenderWindow& window, const std::string& title)
    : ImGuiWrapper(window, title)
{
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
            m_widgetMem.erase(get_base_widget(*widget));
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
            m_widgetMem.erase(m_selected.widget_ptr);
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
    m_selected.widget_info_ptr.info_Update(m_selected.widget_info_ptr.m_wrappedWidget, m_rows); //!< only value refreshed for each key with getter
}

void InfoWrapper::refreshUI()
{
    if(!m_selected.widget_info_ptr) return;
    m_selected.widget_info_ptr.info_Fetch(m_selected.widget_info_ptr.m_wrappedWidget, m_rows);  //!< m_rows fully refreshed
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
        // try match with hard-coded formats
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

    /* Edit section */
    renderEditSection();

    /* Selected widget section */
    if(!m_selected.widget_ptr) return;

    textHeader("Selected Widget Section", true, true, HEADER_1);
    ImGui::PushID(m_selectToken);   //!< avoids value inheritence between previous and new selected widgets
    
    // rendered in order
    renderFlagsSection();
    renderAccessorSections();
    renderShadersSection();
    renderBindsSection();
    renderThemeSection();
    ImGui::PopID();
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

    if(ImGui::Button("Add Button"))
    {
        if(m_selected.enable_if_frame_ptr)
        {
            addWidget<Button>(*m_selected.enable_if_frame_ptr, sf::Vector2f{10.f, 10.f}, sf::Vector2u{100, 20}, "Button");
            refreshValues();
        }
        else addWidget<Button>(m_window, sf::Vector2f{10.f, 10.f}, sf::Vector2u{100, 20}, "Button");
    }
    ImGui::SameLine();
    if(ImGui::Button("Add Frame"))
    {
        if(m_selected.enable_if_frame_ptr)
        {
            addWidget<Frame>(*m_selected.enable_if_frame_ptr, sf::Vector2f{10.f, 10.f}, sf::Vector2u{100, 20});
            refreshValues();
        }
        else addWidget<Frame>(m_window, sf::Vector2f{10.f, 10.f}, sf::Vector2u{100, 20});
    }
    ImGui::SameLine();
    if(ImGui::Button("Add Label"))
    {
        if(m_selected.enable_if_frame_ptr)
        {
            addWidget<Label>(*m_selected.enable_if_frame_ptr, sf::Vector2f{10.f, 10.f}, sf::Vector2u{100, 50}, "Label");
            refreshValues();
        }
        else addWidget<Label>(m_window, sf::Vector2f{10.f, 10.f}, sf::Vector2u{100, 50}, "Label");
    }

    if(ImGui::Button("Add Entry"))
    {
        if(m_selected.enable_if_frame_ptr)
        {
            addWidget<Entry>(*m_selected.enable_if_frame_ptr, sf::Vector2f{10.f, 10.f}, sf::Vector2u{100, 20});
            refreshValues();
        }
        else addWidget<Entry>(m_window, sf::Vector2f{10.f, 10.f}, sf::Vector2u{100, 20});
    }
    ImGui::SameLine();
    if(ImGui::Button("Add CheckBox"))
    {
        if(m_selected.enable_if_frame_ptr)
        {
            addWidget<CheckBox>(*m_selected.enable_if_frame_ptr, sf::Vector2f{10.f, 10.f}, sf::Vector2u{30, 30});
            refreshValues();
        }
        else addWidget<CheckBox>(m_window, sf::Vector2f{10.f, 10.f}, sf::Vector2u{30, 30});
    }

    if(ImGui::Button("Add ImageFrame"))
    {
        if(m_selected.enable_if_frame_ptr)
        {
            addWidget<ImageFrame>(*m_selected.enable_if_frame_ptr, sf::Vector2f{10.f, 10.f}, sf::Vector2u{100, 100});
            refreshValues();
        }
        else addWidget<ImageFrame>(m_window, sf::Vector2f{10.f, 10.f}, sf::Vector2u{100, 100});
    }
    ImGui::SameLine();
    if(ImGui::Button("Add GridCanvas"))
    {
        if(m_selected.enable_if_frame_ptr)
        {
            addWidget<GridCanvas>(*m_selected.enable_if_frame_ptr, sf::Vector2f{10.f, 10.f}, sf::Vector2u{160, 160}, std::size_t{16}, 0.f, true);
            refreshValues();
        }
        else addWidget<GridCanvas>(m_window, sf::Vector2f{10.f, 10.f}, sf::Vector2u{160, 160}, std::size_t{16}, 0.f, true);
    }

    ImGui::Spacing();

    ImGui::BeginDisabled(m_selected.widget_ptr == nullptr);
    if(ImGui::Button("Delete Selected")) deleteSelectedWidget();
    ImGui::EndDisabled();

    ImGui::SameLine();
    if(ImGui::Button("Reset Board")) resetUI();

    ImGui::SameLine();
    if(ImGui::Button("Refresh")) refreshUI();
}

void InfoWrapper::renderFps() const
{
    if(m_revealFps)
    {
        ImGui::SameLine();
        char fps_text[12];
        std::snprintf(fps_text, sizeof(fps_text), "FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(fps_text).x);
        ImGui::TextUnformatted(fps_text);
    }
}

void InfoWrapper::renderFlagsSection()
{
    textHeader("Flags", true, false, HEADER_2);

    /* Flags Grid */
    ImGui::BeginGroup();
    textHeader("Active Flags:", false, false, HEADER_3);

    constexpr std::size_t N_FLAGS = enum_literals::FLAG_BITS_LITERAL.size();
    constexpr unsigned int N_ROWS = 6;
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
        ImU32 fill_color =  is_set ? 
                     IM_COL32(80, 200, 120, 255) : 
                     IM_COL32(70, 70, 70, 255);

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

    for(const auto& section_literal : enum_literals::ENUM_SECTION_LITERAL)
    {
        // check if any keys are initialized with current section
        // avoids leaving a dangling section
        bool has_keys = std::any_of(m_rows.begin(), m_rows.end(),
            [&](const Info& row)
            {
                return row.info_section == section_literal.get<1>();
            });

        if(!has_keys) continue;

        if(section_count % 2 == 1)
        {
            ImGui::SetCursorPos(ImVec2(RIGHT_COL_START_X, curr_y));
        }
        else
        {
            // new y
            ImGui::SetCursorPosY(max_bottom_y);
            curr_y = ImGui::GetCursorPosY();
        }

        float initial_y = ImGui::GetCursorPosY();

        ImGui::BeginGroup();
        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.75f, 1.0f, 1.0f));
        ImGui::TextUnformatted(section_literal.get<0>());
        ImGui::PopStyleColor();

        for(Info& row : m_rows)
        {
            if(row.info_section != section_literal.get<1>()) continue;
            ImGui::PushID(row.info_key.c_str());
            ImGui::TextUnformatted(row.info_key.c_str());
            renderRow(row);
            ImGui::PopID();
        }

        ImGui::EndGroup();

        float section_final_y = ImGui::GetCursorPosY() + 6.f; // 6 for spacing

        // start new row from largest y
        if(section_count % 2 == 0) max_bottom_y = section_final_y;
        else max_bottom_y = std::max(max_bottom_y, section_final_y);

        ++section_count;
    }

    ImGui::SetCursorPosY(max_bottom_y);
}

void InfoWrapper::renderRow(Info& row)
{
    constexpr float INPUT_DEFAULT_WIDTH = 80.f;
    constexpr float INPUT_VECTOR_WIDTH = 120.f;
    constexpr float INPUT_TEXT_WIDTH   = 160.f;
    const bool CAN_BE_MODIFIED = row.info_has_setter;

    // for rows with a setter only
    if(!row.info_val.has_value() && CAN_BE_MODIFIED)
    {
        ImGui::SetNextItemWidth(row.info_is_path ? INPUT_TEXT_WIDTH + 90.f : INPUT_TEXT_WIDTH);
        if(ImGui::InputTextWithHint("##key", row.info_hint.has_value() ? (*row.info_hint).c_str() : "Any Value",
                row.info_text.data(), row.info_text.size(),
                            row.info_is_path ? ImGuiInputTextFlags_EnterReturnsTrue : ImGuiInputTextFlags_None));
        {
            trySetFromText(row.info_key, row.info_text.data());
        }
        if(row.info_is_path)
        {
            ImGui::SameLine();
            ImGui::BeginDisabled(m_dialog.is_open.load());
            if(ImGui::Button("...##browse")) openFileDialog(row.info_key, "Open Image", IMG_EXTENSIONS, 3, "Image files");
            ImGui::EndDisabled();

            if(m_dialogResultPath && row.info_key == m_dialog.req_key && m_selectToken == m_dialog.req_token)
            {
                std::snprintf(row.info_text.data(), row.info_text.size(), "%s", m_dialogResultPath->c_str());
                trySetFromText(row.info_key, row.info_text.data());
                m_dialogResultPath.reset();
            }
        }
        return;
    }

    bool was_modified = false;

    if(!CAN_BE_MODIFIED) ImGui::BeginDisabled();

    // display/modify values
    std::visit(visitor{
        [&](bool& v)
        {
            was_modified = ImGui::Checkbox("##key", &v);
        },
        [&](char& v)
        {
            row.info_text[0] = v;
            row.info_text[1] = '\0';
            ImGui::SetNextItemWidth(40.f);
            if(ImGui::InputText("##key", row.info_text.data(), 2))
            {
                v = row.info_text[0];
                was_modified = true;
            }
        },
        [&](float& v)
        {
            ImGui::SetNextItemWidth(INPUT_DEFAULT_WIDTH);
            was_modified = ImGui::InputFloat("##key", &v);
        },
        [&](unsigned int& v)
        {
            ImGui::SetNextItemWidth(INPUT_DEFAULT_WIDTH);
            was_modified = ImGui::InputScalar("##key", ImGuiDataType_U32, &v);
        },
        [&](std::size_t& v)
        {
            ImGui::SetNextItemWidth(INPUT_DEFAULT_WIDTH);
            was_modified = ImGui::InputScalar("##key", ImGuiDataType_U64, &v);
        },
        [&](std::string& v)
        {
            std::snprintf(row.info_text.data(), row.info_text.size(), "%s", v.c_str());
            ImGui::SetNextItemWidth(INPUT_TEXT_WIDTH);
            if(ImGui::InputText("##key", row.info_text.data(), row.info_text.size()))
            {
                v = row.info_text.data();
                was_modified = true;
            }
        },
        [&](Button::SIDE& v)
        {
            constexpr std::size_t N = enum_literals::BUTTON_SIDE_LITERAL.size();
            int idx = std::clamp(static_cast<int>(v), 0, static_cast<int>(N) - 1);

            ImGui::SetNextItemWidth(100.f);
            if (ImGui::InputInt("##key", &idx))
            {
                idx = std::clamp(idx, 0, static_cast<int>(N) - 1);
                v = static_cast<Button::SIDE>(idx);
                was_modified = true;
            }
            ImGui::SameLine();
            ImGui::Text("%s selected", enum_literals::BUTTON_SIDE_LITERAL[static_cast<std::size_t>(idx)].get<0>());

        },
        [&](Frame::F_SeparatorType& v)
        {
            constexpr std::size_t N = enum_literals::SEPARATOR_TYPE_LITERAL.size();
            int idx = std::clamp(static_cast<int>(v), 0, static_cast<int>(N) - 1);

            ImGui::SetNextItemWidth(100.f);
            if (ImGui::InputInt("##key", &idx))
            {
                idx = std::clamp(idx, 0, static_cast<int>(N) - 1);
                v = static_cast<Frame::F_SeparatorType>(idx);
                was_modified = true;
            }
            ImGui::SameLine();
            ImGui::Text("%s selected", enum_literals::SEPARATOR_TYPE_LITERAL[static_cast<std::size_t>(idx)].get<0>());

        },
        [&](Frame::F_AfterPreProcess& v)
        {
            constexpr std::size_t N = enum_literals::AFTER_PREPROCESS_LITERAL.size();
            int idx = std::clamp(static_cast<int>(v), 0, static_cast<int>(N) - 1);

            ImGui::SetNextItemWidth(100.f);
            if (ImGui::InputInt("##key", &idx))
            {
                idx = std::clamp(idx, 0, static_cast<int>(N) - 1);
                v = static_cast<Frame::F_AfterPreProcess>(idx);
                was_modified = true;
            }
            ImGui::SameLine();
            ImGui::Text("%s selected", enum_literals::AFTER_PREPROCESS_LITERAL[static_cast<std::size_t>(idx)].get<0>());

        },
        [&](DIRECTION& v)
        {
            constexpr std::size_t N = enum_literals::DIRECTION_LITERAL.size();
            int idx = std::clamp(static_cast<int>(v), 0, static_cast<int>(N) - 1);

            ImGui::SetNextItemWidth(100.f);
            if (ImGui::InputInt("##key", &idx))
            {
                idx = std::clamp(idx, 0, static_cast<int>(N) - 1);
                v = static_cast<DIRECTION>(idx);
                was_modified = true;
            }
            ImGui::SameLine();
            ImGui::Text("%s selected", enum_literals::DIRECTION_LITERAL[static_cast<std::size_t>(idx)].get<0>());

        },
        [&](Label::HAlign& v)
        {
            constexpr std::size_t N = enum_literals::HALIGN_LITERAL.size();
            int idx = std::clamp(static_cast<int>(v), 0, static_cast<int>(N) - 1);

            ImGui::SetNextItemWidth(100.f);
            if (ImGui::InputInt("##key", &idx))
            {
                idx = std::clamp(idx, 0, static_cast<int>(N) - 1);
                v = static_cast<Label::HAlign>(idx);
                was_modified = true;
            }
            ImGui::SameLine();
            ImGui::Text("%s selected", enum_literals::HALIGN_LITERAL[static_cast<std::size_t>(idx)].get<0>());

        },
        [&](Label::VAlign& v)
        {
            constexpr std::size_t N = enum_literals::VALIGN_LITERAL.size();
            int idx = std::clamp(static_cast<int>(v), 0, static_cast<int>(N) - 1);

            ImGui::SetNextItemWidth(100.f);
            if (ImGui::InputInt("##key", &idx))
            {
                idx = std::clamp(idx, 0, static_cast<int>(N) - 1);
                v = static_cast<Label::VAlign>(idx);
                was_modified = true;
            }
            ImGui::SameLine();
            ImGui::Text("%s selected", enum_literals::VALIGN_LITERAL[static_cast<std::size_t>(idx)].get<0>());
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
            static constexpr std::array<const char*, 3> names{"NoSeparator", "Horizontal", "Vertical"};
            int type_idx = std::clamp(static_cast<int>(v.type), 0, static_cast<int>(names.size()) - 1);

            ImGui::SetNextItemWidth(INPUT_DEFAULT_WIDTH);
            if(ImGui::InputInt("##sep_type", &type_idx))
            {
                type_idx = std::clamp(type_idx, 0, static_cast<int>(names.size()) - 1);
                v.type = static_cast<Frame::F_SeparatorType>(type_idx);
                was_modified = true;
            }
            ImGui::SameLine();
            ImGui::Text("%s selected", names[static_cast<std::size_t>(type_idx)]);

            ImGui::SetNextItemWidth(INPUT_DEFAULT_WIDTH);
            if(ImGui::InputFloat("##sep_ratio", &v.ratio)) was_modified = true;
            ImGui::SameLine();
            ImGui::TextUnformatted("ratio");

            ImGui::SetNextItemWidth(INPUT_DEFAULT_WIDTH);
            if(ImGui::InputFloat("##sep_dist", &v.dist)) was_modified = true;
            ImGui::SameLine();
            ImGui::TextUnformatted("dist");
        },
        [&](Cell* v)
        {
            if(v) ImGui::Text("Cell* (%p)", static_cast<const void*>(v));
            else ImGui::TextUnformatted("Cell* = nullptr");
        },
        [&](sf::String& v)
        {
            std::string str = v.toAnsiString();
            std::snprintf(row.info_text.data(), row.info_text.size(), "%s", str.c_str());
            ImGui::SetNextItemWidth(row.info_is_path ? INPUT_TEXT_WIDTH + 90.f : INPUT_TEXT_WIDTH);
            if(ImGui::InputText("##key", row.info_text.data(), row.info_text.size()), 
                            row.info_is_path ? ImGuiInputTextFlags_EnterReturnsTrue : ImGuiInputTextFlags_None)
            {
                v = sf::String(row.info_text.data());
                was_modified = true;
            }
            if(row.info_is_path)
            {
                ImGui::SameLine();
                ImGui::BeginDisabled(m_dialog.is_open.load());
                if(ImGui::Button("...##browse")) openFileDialog(row.info_key, "Open Image", IMG_EXTENSIONS, 3, "Image files");
                ImGui::EndDisabled();

                if(m_dialogResultPath && row.info_key == m_dialog.req_key && m_selectToken == m_dialog.req_token)
                {
                    v = sf::String(*m_dialogResultPath);
                    was_modified = true;
                    m_dialogResultPath.reset();
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
            ImGui::SetNextItemWidth(INPUT_DEFAULT_WIDTH);
            if(ImGui::InputFloat("##key (deg)", &d))
            {
                v = sf::degrees(d);
                was_modified = true;
            }
        },
        [&](sf::Vector2f& v)
        {
            float xy[2]{v.x, v.y};
            ImGui::SetNextItemWidth(INPUT_VECTOR_WIDTH);
            if(ImGui::InputFloat2("##key", xy, "%.1f"))
            {
                v = sf::Vector2f(xy[0], xy[1]);
                was_modified = true;
            }
        },
        [&](sf::Vector2u& v)
        {
            int xy[2]{static_cast<int>(v.x), static_cast<int>(v.y)};
            ImGui::SetNextItemWidth(INPUT_VECTOR_WIDTH);
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
            ImGui::SetNextItemWidth(INPUT_VECTOR_WIDTH);
            if(ImGui::InputInt2("##key", xy))
            {
                v = sf::Vector2i(xy[0], xy[1]);
                was_modified = true;
            }
        },
        [&](std::optional<sf::Vector2u>& v)
        {
            if(v.has_value()) ImGui::Text("(%u, %u)", v->x, v->y);
            else ImGui::TextUnformatted("Nan");
        }
        },*row.info_val);
    
    if(!CAN_BE_MODIFIED) ImGui::EndDisabled();


    // value was updated?
    if(was_modified && CAN_BE_MODIFIED)
    {
        m_selected.widget_info_ptr.info_Set(m_selected.widget_info_ptr.m_wrappedWidget, row.info_key, *row.info_val);
        refreshValues();
    }
}

void InfoWrapper::renderBindsSection()
{
    textHeader("Bindings", true, true, HEADER_2);

    auto it = m_widgetMem.find(m_selected.widget_ptr);
    if(it == m_widgetMem.end()) return;   //!< just in case

    BuiltinBindFlags& flags_ref = it->second.flags;
    for(const auto& bind_literal : enum_literals::QUICK_BINDINGS_LITERAL)
    {
        // is flag set?
        bool state = isBuiltinFlagSet(flags_ref, bind_literal.get<1>());
        if(ImGui::Checkbox(bind_literal.get<0>(), &state))
        {
            flags_ref = state ? (flags_ref | bind_literal.get<1>()) :   // set
                                (flags_ref & ~bind_literal.get<1>());   // reset
        }
    }
}

void InfoWrapper::renderThemeSection()
{
    textHeader("Theme", true, true, HEADER_2);

    auto it = m_widgetMem.find(m_selected.widget_ptr);
    if(it == m_widgetMem.end()) return;

    constexpr std::size_t N_THEMES = enum_literals::THEME_LITERAL.size();
    std::shared_ptr<WidgetTheme>& curr_theme = it->second.theme;
    int curr_theme_idx = -1;

    // get cached theme
    for(std::size_t i = 0; i < N_THEMES; ++i)
    {
        if(enum_literals::THEME_LITERAL[i].get<1>() == curr_theme)
        {
            curr_theme_idx = static_cast<int>(i); 
            break; 
        }
    }

    const char* theme_label = (curr_theme_idx >= 0) ?
                enum_literals::THEME_LITERAL[static_cast<std::size_t>(curr_theme_idx)].get<0>() :
                "None";

    ImGui::SetNextItemWidth(200.f);
    if(ImGui::BeginCombo("##theme_select", theme_label))
    {
        // selected None option
        if(ImGui::Selectable("None", curr_theme_idx == -1))
        {
            if(curr_theme)
            {
                m_selected.widget_ptr->unlinkTheme();
                curr_theme.reset();
            }
        }

        // other options
        for(std::size_t i = 0; i < N_THEMES; ++i)
        {
            bool is_selected = (static_cast<int>(i) == curr_theme_idx);
            if(ImGui::Selectable(enum_literals::THEME_LITERAL[i].get<0>(), is_selected))
            {
                // prevent re-linking
                if(enum_literals::THEME_LITERAL[i].get<1>() != curr_theme)
                {
                    m_selected.widget_ptr->linkTheme(enum_literals::THEME_LITERAL[i].get<1>());
                    curr_theme = enum_literals::THEME_LITERAL[i].get<1>();
                }
            }
            // select
            if(is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    ImGui::TextDisabled(curr_theme ? "Theme Linked" : "No theme is currently linked");
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
            m_dialog.directory_path = result ? result : "";
        }
        m_dialog.path_found.store(result != nullptr);
        m_dialog.is_open.store(false);
    });
}

void InfoWrapper::pullDialogPath()
{
    if(m_dialog.path_found.exchange(false))
    {
        std::lock_guard<std::mutex> lock(m_dialog.guard);
        m_dialogResultPath = m_dialog.directory_path;
        m_dialog.directory_path.clear();
    }
}

void InfoWrapper::renderShadersSection()
{
    textHeader("Shaders", true, true, HEADER_2);
    auto it = m_widgetMem.find(m_selected.widget_ptr);
    if(it == m_widgetMem.end()) return;

    renderShaderControl("Clip Shader", "clip_shader", it->second.clip_shader_ui,
                         &Widget::loadClipShader, &Widget::resetClipShader);
    ImGui::Spacing();
    renderShaderControl("Color Shader", "color_shader", it->second.color_shader_ui,
                         &Widget::loadColorShader, &Widget::resetColorShader);
}

void InfoWrapper::renderShaderControl(const char* label, const char* id_suffix, ShaderUiState& state,
                                       bool (Widget::*loadFn)(const std::string_view&, Widget::W_LoadType),
                                       void (Widget::*resetFn)(const std::string_view&, Widget::W_LoadType))
{
    ImGui::PushID(id_suffix);
    ImGui::TextUnformatted(label);

    ImGui::TextUnformatted("Available in Memory:");
    for(std::size_t i = 0; i < enum_literals::BUILTIN_SHADERS_LITERAL.size(); ++i)
    {
        ImGui::SameLine();
        if(ImGui::Button(enum_literals::BUILTIN_SHADERS_LITERAL[i].get<0>()))
            (m_selected.widget_ptr->*loadFn)(enum_literals::BUILTIN_SHADERS_LITERAL[i].get<1>(), Widget::W_LoadType::Memory);
    }

    ImGui::TextUnformatted("From file:");
    ImGui::SetNextItemWidth(200.f);
    ImGui::InputTextWithHint("##path", "shader file path", state.file_path.data(), state.file_path.size(), ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();
    ImGui::BeginDisabled(m_dialog.is_open.load());
    if(ImGui::Button("...##browse")) openFileDialog(id_suffix, "Open Shader", SHADER_EXTENSIONS, 2, "Shader files");
    ImGui::EndDisabled();

    if(m_dialogResultPath && m_dialog.req_key == id_suffix && m_selectToken == m_dialog.req_token)
    {
        std::snprintf(state.file_path.data(), state.file_path.size(), "%s", m_dialogResultPath->c_str());
        m_dialogResultPath.reset();
    }

    bool has_path = state.file_path[0] != '\0';
    ImGui::SameLine();
    ImGui::BeginDisabled(!has_path);
    if(ImGui::Button("Load##file")) (m_selected.widget_ptr->*loadFn)(std::string_view(state.file_path.data()), Widget::W_LoadType::File);
    ImGui::EndDisabled();

    ImGui::Spacing();
    if(ImGui::Button("Reset")) (m_selected.widget_ptr->*resetFn)("", Widget::W_LoadType::File);

    ImGui::PopID();
}
}

#endif