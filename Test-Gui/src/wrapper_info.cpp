#include "wrapper_info.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>

#ifdef WRAPPER_DEFINED

namespace sfml_widgets
{
InfoWrapper::InfoWrapper(sf::RenderWindow& window, const std::string& title)
    : ImGuiWrapper(window, title)
{
}

void InfoWrapper::enableFps(bool show_fps)
{
    m_revealFps = show_fps;
}

void InfoWrapper::resetUI()
{
    unselectWidget();
    for(std::optional<Widget_t>& widget : m_widgets) widget.reset();
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
            unselectWidget();
            widget.reset();
            --m_widgetsCount;
            return;
        }
    }
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
    ImGui::Text("Edit");

    // fps
    if(m_revealFps)
    {
        char fpsText[12];
        std::snprintf(fpsText, sizeof(fpsText), "FPS: %.1f", ImGui::GetIO().Framerate);
        float textWidth = ImGui::CalcTextSize(fpsText).x;
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - textWidth);
        ImGui::TextUnformatted(fpsText);
    }

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


    /* Selected widget section */
    if(!m_selected.widget_ptr) return;

    ImGui::Separator();
    ImGui::Text("Selected Widget");

    ImGui::PushID(m_selectToken);   //!< avoids value inheritence between previous and new selected widgets

    // building child window
    float row_height = ImGui::GetTextLineHeightWithSpacing() * 2.2f; // label line + input line, roughly
    float desired = m_rows.empty() ? row_height : static_cast<float>(m_rows.size()) * row_height;
    float child_height = std::min(desired, 260.f);
    
    ImGui::BeginChild("info_scroll", ImVec2(0, child_height), true);
    for(Info& row : m_rows)
    {
        ImGui::PushID(row.info_key.c_str());
        ImGui::TextUnformatted(row.info_key.c_str());
        renderRow(row);
        ImGui::Spacing();
        ImGui::PopID(); // for key
    }
    ImGui::EndChild();  // bye child

    ImGui::PopID(); // for token
}

void InfoWrapper::refreshUI()
{
    if(!m_selected.widget_info_ptr) return;
    m_selected.widget_info_ptr.info_Fetch(m_selected.widget_info_ptr.m_wrappedWidget, m_rows);  //!< m_rows fully refreshed
}

void InfoWrapper::refreshValues()
{
    if(!m_selected.widget_info_ptr) return;
    m_selected.widget_info_ptr.info_Update(m_selected.widget_info_ptr.m_wrappedWidget, m_rows); //!< only value refreshed for each key with getter
}

void InfoWrapper::renderRow(Info& row)
{
    constexpr float input_default_width = 100.f;
    constexpr float input_vector_width = 160.f;
    constexpr float input_text_width   = 220.f;
    const bool modifiable_value = row.info_has_setter;

    // for rows with a setter only
    if(!row.info_val.has_value() && modifiable_value)
    {
        // there is no getter to display the current value (control method like .remove, .insert, ect...)
        // so we return here either way
        ImGui::SetNextItemWidth(input_text_width);
        if(ImGui::InputTextWithHint("##key", "Any Value", row.info_text.data(), row.info_text.size(),
                ImGuiInputTextFlags_EnterReturnsTrue))  //!< only evaluate after pressing Enter
        {
            trySetFromText(row.info_key, row.info_text.data());
        }
        return;
    }

    bool modified = false;

    if(!modifiable_value) ImGui::BeginDisabled();

    // display/modify values
    std::visit(visitor{
        [&](bool& v)
        {
            modified = ImGui::Checkbox("##key", &v);
        },
        [&](char& v)
        {
            row.info_text[0] = v;
            row.info_text[1] = '\0';
            ImGui::SetNextItemWidth(40.f);
            if(ImGui::InputText("##key", row.info_text.data(), 2))
            {
                v = row.info_text[0];
                modified = true;
            }
        },
        [&](float& v)
        {
            ImGui::SetNextItemWidth(input_default_width);
            modified = ImGui::InputFloat("##key", &v);
        },
        [&](unsigned int& v)
        {
            ImGui::SetNextItemWidth(input_default_width);
            modified = ImGui::InputScalar("##key", ImGuiDataType_U32, &v);
        },
        [&](std::size_t& v)
        {
            ImGui::SetNextItemWidth(input_default_width);
            modified = ImGui::InputScalar("##key", ImGuiDataType_U64, &v);
        },
        [&](std::string& v)
        {
            std::snprintf(row.info_text.data(), row.info_text.size(), "%s", v.c_str());
            ImGui::SetNextItemWidth(input_text_width);
            if(ImGui::InputText("##key", row.info_text.data(), row.info_text.size()))
            {
                v = row.info_text.data();
                modified = true;
            }
        },
        [&](Button::SIDE& v)
        {
            static constexpr std::array<const char*, 3> names{"Center", "Left", "Right"};
            modified = renderIndexedEnum(v, names);
        },
        [&](Frame::F_SeparatorType& v)
        {
            static constexpr std::array<const char*, 3> names{"NoSeparator", "Horizontal", "Vertical"};
            modified = renderIndexedEnum(v, names);
        },
        [&](Frame::F_AfterPreProcess& v)
        {
            static constexpr std::array<const char*, 3> names{"NO_SKIP", "SKIP_CALLBACK", "SKIP_ALL"};
            modified = renderIndexedEnum(v, names);
        },
        [&](DIRECTION& v)
        {
            static constexpr std::array<const char*, 4> names{"TOP", "BOTTOM", "LEFT", "RIGHT"};
            modified = renderIndexedEnum(v, names);
        },
        [&](Label::HAlign& v)
        {
            static constexpr std::array<const char*, 3> names{"Left", "Center", "Right"};
            modified = renderIndexedEnum(v, names);
        },
        [&](Label::VAlign& v)
        {
            static constexpr std::array<const char*, 3> names{"Top", "Middle", "Bottom"};
            modified = renderIndexedEnum(v, names);
        },
        [&](sf::Text::Style& v)
        {
            int flags = static_cast<int>(v);
            bool is_modified = false;
            is_modified |= ImGui::CheckboxFlags("Bold", &flags, sf::Text::Bold);
            ImGui::SameLine();
            is_modified |= ImGui::CheckboxFlags("Italic", &flags, sf::Text::Italic);
            ImGui::SameLine();
            is_modified |= ImGui::CheckboxFlags("Underlined", &flags, sf::Text::Underlined);
            ImGui::SameLine();
            is_modified |= ImGui::CheckboxFlags("StrikeThrough", &flags, sf::Text::StrikeThrough);

            if(is_modified)
            {
                v = static_cast<sf::Text::Style>(flags);
                modified = true;
            }
        },
        [&](Frame::F_SeparatorInfo& v)
        {
            static constexpr std::array<const char*, 3> names{"NoSeparator", "Horizontal", "Vertical"};
            int type_idx = std::clamp(static_cast<int>(v.type), 0, static_cast<int>(names.size()) - 1);

            ImGui::SetNextItemWidth(input_default_width);
            if(ImGui::InputInt("##sep_type", &type_idx))
            {
                type_idx = std::clamp(type_idx, 0, static_cast<int>(names.size()) - 1);
                v.type = static_cast<Frame::F_SeparatorType>(type_idx);
                modified = true;
            }
            ImGui::SameLine();
            ImGui::Text("%s selected", names[static_cast<std::size_t>(type_idx)]);

            ImGui::SetNextItemWidth(input_default_width);
            if(ImGui::InputFloat("##sep_ratio", &v.ratio)) modified = true;
            ImGui::SameLine();
            ImGui::TextUnformatted("ratio");

            ImGui::SetNextItemWidth(input_default_width);
            if(ImGui::InputFloat("##sep_dist", &v.dist)) modified = true;
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
            ImGui::SetNextItemWidth(input_text_width);
            if(ImGui::InputText("##key", row.info_text.data(), row.info_text.size()))
            {
                v = sf::String(row.info_text.data());
                modified = true;
            }
        },
        [&](sf::FloatRect& v)
        {
            ImGui::Text("pos(%.1f, %.1f) size(%.1f, %.1f)", v.position.x, v.position.y, v.size.x, v.size.y);
        },
        [&](sf::Angle& v)
        {
            float d = v.asDegrees();
            ImGui::SetNextItemWidth(input_default_width);
            if(ImGui::InputFloat("##key (deg)", &d))
            {
                v = sf::degrees(d);
                modified = true;
            }
        },
        [&](sf::Vector2f& v)
        {
            float xy[2]{v.x, v.y};
            ImGui::SetNextItemWidth(input_vector_width);
            if(ImGui::InputFloat2("##key", xy))
            {
                v = sf::Vector2f(xy[0], xy[1]);
                modified = true;
            }
        },
        [&](sf::Vector2u& v)
        {
            int xy[2]{static_cast<int>(v.x), static_cast<int>(v.y)};
            ImGui::SetNextItemWidth(input_vector_width);
            if(ImGui::InputInt2("##key", xy))
            {
                v = sf::Vector2u(static_cast<unsigned int>(std::max(0, xy[0])),
                                 static_cast<unsigned int>(std::max(0, xy[1])));
                modified = true;
            }
        },
        [&](sf::Vector2i& v)
        {
            int xy[2]{v.x, v.y};
            ImGui::SetNextItemWidth(input_vector_width);
            if(ImGui::InputInt2("##key", xy))
            {
                v = sf::Vector2i(xy[0], xy[1]);
                modified = true;
            }
        },
        [&](std::optional<sf::Vector2u>& v)
        {
            if(v.has_value()) ImGui::Text("(%u, %u)", v->x, v->y);
            else ImGui::TextUnformatted("(none)");
        }
        },*row.info_val);
    
    if(!modifiable_value) ImGui::EndDisabled();


    // value was updated
    if(modified && modifiable_value)
    {
        m_selected.widget_info_ptr.info_Set(m_selected.widget_info_ptr.m_wrappedWidget, row.info_key, *row.info_val);
        refreshValues();
    }
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
}

#endif