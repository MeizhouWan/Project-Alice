#pragma once

namespace ui {


class wargoal_cancel_button : public button_element_base {
public:
	void button_action(sys::state& state) noexcept override {
		send(state, parent, element_selection_wrapper<dcon::cb_type_id>{});
	}
};


class wargoal_type_item_icon : public image_element_base {
public:
	void on_update(sys::state& state) noexcept override {
		auto content = retrieve<dcon::cb_type_id>(state, parent);
		frame = (dcon::fatten(state.world, content).get_sprite_index() - 1);
	}
};

class wargoal_type_item_button : public tinted_button_element_base {
public:
	void button_action(sys::state& state) noexcept override {
		dcon::cb_type_id content = retrieve<dcon::cb_type_id>(state, parent);
		dcon::cb_type_id selected = parent ? retrieve<dcon::cb_type_id>(state, parent->parent) : dcon::cb_type_id{};
		if(content == selected)
			send(state, parent, element_selection_wrapper<dcon::cb_type_id>{dcon::cb_type_id{}});
		else
			send(state, parent, element_selection_wrapper<dcon::cb_type_id>{content});
	}

	void on_update(sys::state& state) noexcept override {
		dcon::cb_type_id content = retrieve<dcon::cb_type_id>(state, parent);
		dcon::nation_id target = retrieve<dcon::nation_id>(state, parent);
		auto target_state = retrieve<dcon::state_definition_id>(state, parent);

		auto fat_id = dcon::fatten(state.world, content);
		set_button_text(state, text::produce_simple_string(state, fat_id.get_name()));

		auto other_cbs = state.world.nation_get_available_cbs(state.local_player_nation);
		auto can_declare_with_wg = [&]() {
			if((state.world.cb_type_get_type_bits(content) & military::cb_flag::always) != 0) {
				return true;
			}
			for(auto& fabbed : other_cbs) {
				if(fabbed.cb_type == content && fabbed.target == target)
					return true;
			}
			return false;
			}();
		auto w = military::find_war_between(state, state.local_player_nation, target);
		bool can_use = military::cb_conditions_satisfied(state, state.local_player_nation, target, content) && (can_declare_with_wg || w);

		disabled = !can_use;
		if(disabled) {
			color = sys::pack_color(255, 255, 255);
			return;
		}

		auto war = retrieve<dcon::war_id>(state, parent);
		auto cb_infamy = !war
			? (military::has_truce_with(state, state.local_player_nation, target)
				? military::truce_break_cb_infamy(state, content, target)
				: military::cb_infamy(state, content, target, target_state))
			: military::cb_addition_infamy_cost(state, war, content, state.local_player_nation, target, target_state);
		if(state.world.nation_get_infamy(state.local_player_nation) + cb_infamy >= state.defines.badboy_limit) {
			color = sys::pack_color(255, 196, 196);
		} else {
			color = sys::pack_color(255, 255, 255);
		}
	}

	tooltip_behavior has_tooltip(sys::state& state) noexcept override {
		return tooltip_behavior::variable_tooltip;
	}
	void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
		dcon::cb_type_id content = retrieve<dcon::cb_type_id>(state, parent);
		dcon::nation_id target = retrieve<dcon::nation_id>(state, parent);
		auto war = retrieve<dcon::war_id>(state, parent);
		auto cb_infamy = !war
			? (military::has_truce_with(state, state.local_player_nation, target)
				? military::truce_break_cb_infamy(state, content, target)
				: 0.f)
			: military::cb_addition_infamy_cost(state, war, content, state.local_player_nation, target);
		if(state.world.nation_get_infamy(state.local_player_nation) + cb_infamy >= state.defines.badboy_limit) {
			text::add_line(state, contents, "alice_tt_wg_infamy_limit");
		}

		auto fat_id = dcon::fatten(state.world, content);
		if(fat_id.get_can_use()) {
			text::add_line(state, contents, "tt_can_use_nation");
			trigger_description(state, contents, fat_id.get_can_use(), trigger::to_generic(target), trigger::to_generic(state.local_player_nation), trigger::to_generic(target));
		}
		if(auto allowed_substates = fat_id.get_allowed_substate_regions(); allowed_substates) {
			text::add_line_with_condition(state, contents, "is_substate", state.world.nation_get_is_substate(target));
			if(state.world.nation_get_is_substate(target)) {
				auto ruler = state.world.overlord_get_ruler(state.world.nation_get_overlord_as_subject(target));
				trigger_description(state, contents, allowed_substates, trigger::to_generic(ruler), trigger::to_generic(state.local_player_nation), trigger::to_generic(state.local_player_nation));
			}
		}
		text::add_line(state, contents, "et_on_add");
		effect_description(state, contents, fat_id.get_on_add(), trigger::to_generic(state.local_player_nation), trigger::to_generic(state.local_player_nation), trigger::to_generic(target),
			uint32_t(state.current_date.value), uint32_t((state.local_player_nation.index() << 7) ^ target.index() ^ (fat_id.id.index() << 3)));
		text::add_line(state, contents, "et_on_po_accepted");
		auto windx = 0;//wargoal.index()
		effect_description(state, contents, fat_id.get_on_po_accepted(), trigger::to_generic(target), trigger::to_generic(state.local_player_nation), trigger::to_generic(state.local_player_nation),
			uint32_t((state.current_date.value << 8) ^ target.index()), uint32_t(state.local_player_nation.index() ^ (windx << 3)));

		if(auto allowed_states = fat_id.get_allowed_states(); allowed_states) {
			bool described = false;
			for(auto si : state.world.nation_get_state_ownership(target)) {
				if(trigger::evaluate(state, allowed_states, trigger::to_generic(si.get_state().id), trigger::to_generic(state.local_player_nation), trigger::to_generic(state.local_player_nation))) {
					ui::trigger_description(state, contents, allowed_states, trigger::to_generic(si.get_state().id), trigger::to_generic(state.local_player_nation), trigger::to_generic(state.local_player_nation));
					described = true;
					break;
				}
			}
			if(!described) {
				ui::trigger_description(state, contents, allowed_states, -1, trigger::to_generic(state.local_player_nation), trigger::to_generic(state.local_player_nation));
			}
		}
	}
};

class wargoal_type_item : public listbox_row_element_base<dcon::cb_type_id> {
public:
	std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
		if(name == "wargoal_icon") {
			auto ptr = make_element_by_type<wargoal_type_item_icon>(state, id);
			//ptr->base_data.position.x += 16; // Nudge
			return ptr;
		} else if(name == "select_goal_invalid") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "select_goal") {
			auto ptr = make_element_by_type<wargoal_type_item_button>(state, id);
			//ptr->base_data.position.x += 16; // Nudge
			return ptr;
		} else {
			return nullptr;
		}
	}
};

class wargoal_type_listbox : public listbox_element_base<wargoal_type_item, dcon::cb_type_id> {
protected:
	std::string_view get_row_element_name() override {
		return "wargoal_item";
	}

public:
	void on_update(sys::state& state) noexcept override {
		row_contents.clear();

		auto selected_cb = retrieve<dcon::cb_type_id>(state, parent);
		if(selected_cb) {
			row_contents.push_back(selected_cb);
			update(state);
			return;
		}

		dcon::nation_id content = retrieve<dcon::nation_id>(state, parent);
		auto w = military::find_war_between(state, state.local_player_nation, content);

		if(w) { // war is ongoing
			for(auto cb_type : state.world.in_cb_type) {
				// prevent duplicate war goals
				if(!military::cb_requires_selection_of_a_liberatable_tag(state, cb_type) && !military::cb_requires_selection_of_a_state(state, cb_type) && !military::cb_requires_selection_of_a_valid_nation(state, cb_type)) {
					if(military::war_goal_would_be_duplicate(state, state.local_player_nation, w, content, cb_type, dcon::state_definition_id{}, dcon::national_identity_id{}, dcon::nation_id{}))
						continue;
				}

				if((state.world.cb_type_get_type_bits(cb_type) & military::cb_flag::always) == 0 && military::cb_conditions_satisfied(state, state.local_player_nation, content, cb_type)) {
					bool cb_fabbed = false;
					for(auto& fab_cb : state.world.nation_get_available_cbs(state.local_player_nation)) {
						if(fab_cb.cb_type == cb_type && fab_cb.target == content) {
							cb_fabbed = true;
							break;
						}
					}
					if(!cb_fabbed) {
						if((state.world.cb_type_get_type_bits(cb_type) & military::cb_flag::is_not_constructing_cb) != 0)
							continue; // can only add a constructable cb this way

						auto totalpop = state.world.nation_get_demographics(state.local_player_nation, demographics::total);
						auto jingoism_perc = totalpop > 0 ? state.world.nation_get_demographics(state.local_player_nation, demographics::to_key(state, state.culture_definitions.jingoism)) / totalpop : 0.0f;
						if(state.world.war_get_is_great(w)) {
							if(jingoism_perc >= state.defines.wargoal_jingoism_requirement * state.defines.gw_wargoal_jingoism_requirement_mod) {
								row_contents.push_back(cb_type);
								continue;
							}
						} else {
							if(jingoism_perc >= state.defines.wargoal_jingoism_requirement) {
								row_contents.push_back(cb_type);
								continue;
							}
						}
					} else {
						row_contents.push_back(cb_type);
						continue;
					}
				} else { // this is an always CB
					if(military::cb_conditions_satisfied(state, state.local_player_nation, content, cb_type) && military::can_add_always_cb_to_war(state, state.local_player_nation, content, cb_type, w)) {
						row_contents.push_back(cb_type);
						continue;
					}
				}
			}
		} else { // this is a declare war action
			// Display all CB types sorting available ones first
			// Buttons for unavailable CB types will be disabled
			auto other_cbs = state.world.nation_get_available_cbs(state.local_player_nation);
			
			std::vector<dcon::cb_type_fat_id> cb_types;

			for(auto cb : state.world.in_cb_type) {
				cb_types.push_back(cb);
			}

			std::sort(cb_types.begin(), cb_types.end(), [&](dcon::cb_type_fat_id& a, dcon::cb_type_fat_id& b) {

				bool can_use_a = military::cb_conditions_satisfied(state, state.local_player_nation, content, a) && [&]() {
					if((a.get_type_bits() & military::cb_flag::always) != 0) {
						return true;
					}
					for(auto& fabbed : other_cbs) {
						if(fabbed.cb_type == a && fabbed.target == content)
							return true;
					}
					return false;
					}();

				bool can_use_b = military::cb_conditions_satisfied(state, state.local_player_nation, content, b) && [&]() {
					if((b.get_type_bits() & military::cb_flag::always) != 0) {
						return true;
					}
					for(auto& fabbed : other_cbs) {
						if(fabbed.cb_type == b && fabbed.target == content)
							return true;
					}
					return false;
					}();

				if(can_use_a != can_use_b) {
					return can_use_a;
				}
				else {
					return a.id.index() < b.id.index();
				}
			});

			for(auto el : cb_types) {
				row_contents.push_back(el);
			}

		}

		update(state);
	}
};

class wargoal_setup_window : public window_element_base {
public:
	std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
		if(name == "wargoal_list") {
			return make_element_by_type<wargoal_type_listbox>(state, id);
		} else if(name == "cancel_select") {
			return make_element_by_type<wargoal_cancel_button>(state, id);
		} else {
			return nullptr;
		}
	}
};

class wargoal_country_item_button : public button_element_base {
public:
	void button_action(sys::state& state) noexcept override {
		dcon::national_identity_id content = retrieve<dcon::national_identity_id>(state, parent);
		send(state, parent, element_selection_wrapper<dcon::national_identity_id>{ content });
	}

	void on_update(sys::state& state) noexcept override {
		dcon::national_identity_id content = retrieve<dcon::national_identity_id>(state, parent);
		set_button_text(state, text::produce_simple_string(state, dcon::fatten(state.world, content).get_name()));
	}

	tooltip_behavior has_tooltip(sys::state& state) noexcept override {
		return tooltip_behavior::variable_tooltip;
	}
	void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
		dcon::cb_type_id cb = retrieve<dcon::cb_type_id>(state, parent);
		if(auto allowed_countries = state.world.cb_type_get_allowed_countries(cb); allowed_countries) {
			dcon::nation_id target = retrieve<dcon::nation_id>(state, parent);
			auto holder = state.world.national_identity_get_nation_from_identity_holder(retrieve<dcon::national_identity_id>(state, parent));
			trigger_description(state, contents, allowed_countries, trigger::to_generic(target), trigger::to_generic(state.local_player_nation), trigger::to_generic(holder));
		}
	}
};

class wargoal_country_item : public listbox_row_element_base<dcon::national_identity_id> {
public:
	std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
		if(name == "select_country") {
			return make_element_by_type<wargoal_country_item_button>(state, id);
		} else {
			return nullptr;
		}
	}
};

class wargoal_country_listbox : public listbox_element_base<wargoal_country_item, dcon::national_identity_id> {
protected:
	std::string_view get_row_element_name() override {
		return "wargoal_country_item";
	}

public:
	void on_update(sys::state& state) noexcept override {
		row_contents.clear();

		dcon::nation_id target = retrieve<dcon::nation_id>(state, parent);
		auto actor = state.local_player_nation;
		dcon::cb_type_id cb = retrieve<dcon::cb_type_id>(state, parent);
		auto war = military::find_war_between(state, actor, target);

		dcon::trigger_key allowed_countries = state.world.cb_type_get_allowed_countries(cb);
		if(!allowed_countries) {
			update(state);
			return;
		}

		for(auto n : state.world.in_nation) {
			if(n != actor && trigger::evaluate(state, allowed_countries, trigger::to_generic(target), trigger::to_generic(actor), trigger::to_generic(n.id))) {
				auto id = state.world.nation_get_identity_from_identity_holder(n);
				if(!war) {
					row_contents.push_back(id);
				} else if(military::cb_requires_selection_of_a_state(state, cb)) {
					row_contents.push_back(id);
				} else {
					if(military::cb_requires_selection_of_a_liberatable_tag(state, cb)) {
						if(!military::war_goal_would_be_duplicate(state, state.local_player_nation, war, target, cb, dcon::state_definition_id{}, id, dcon::nation_id{})) {
							row_contents.push_back(id);
						}
					} else {
						if(!military::war_goal_would_be_duplicate(state, state.local_player_nation, war, target, cb, dcon::state_definition_id{}, dcon::national_identity_id{}, n)) {
							row_contents.push_back(id);
						}
					}
				}
			}
		}

		update(state);
	}
};

class wargoal_cancel_country_select : public button_element_base {
public:
	void button_action(sys::state& state) noexcept override {
		send(state, parent, element_selection_wrapper<dcon::national_identity_id>{dcon::national_identity_id{}});
	}
};

class wargoal_country_select_window : public window_element_base {
public:
	std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
		if(name == "country_list") {
			return make_element_by_type<wargoal_country_listbox>(state, id);
		} else if(name == "cancel_select") {
			return make_element_by_type<wargoal_cancel_country_select>(state, id);
		} else {
			return nullptr;
		}
	}
};


class wargoal_add_prestige : public color_text_element {
public:
	void on_update(sys::state& state) noexcept override {
		auto cb = retrieve<dcon::cb_type_id>(state, parent);
		auto target = retrieve<dcon::nation_id>(state, parent);
		auto source = state.local_player_nation;

		dcon::nation_id real_target = target;

		auto target_ol_rel = state.world.nation_get_overlord_as_subject(target);
		if(state.world.overlord_get_ruler(target_ol_rel))
			real_target = state.world.overlord_get_ruler(target_ol_rel);

		if(cb) {
			if(auto w = military::find_war_between(state, source, target); w) {
				color = text::text_color::white;
				set_text(state, "0.0");
			} else if(military::has_truce_with(state, source, real_target)) {
				auto cb_prestige_loss = military::truce_break_cb_prestige_cost(state, cb);

				color = text::text_color::red;
				set_text(state, text::format_float(cb_prestige_loss, 1));
			} else {
				color = text::text_color::white;
				set_text(state, "0.0");
			}
		} else {
			color = text::text_color::white;
			set_text(state, "0.0");
		}
	}
};
class wargoal_add_infamy : public color_text_element {
public:
	void on_update(sys::state& state) noexcept override {
		auto cb = retrieve<dcon::cb_type_id>(state, parent);
		auto target = retrieve<dcon::nation_id>(state, parent);
		auto target_state = retrieve<dcon::state_definition_id>(state, parent);

		auto source = state.local_player_nation;

		dcon::nation_id real_target = target;

		auto target_ol_rel = state.world.nation_get_overlord_as_subject(target);
		if(state.world.overlord_get_ruler(target_ol_rel))
			real_target = state.world.overlord_get_ruler(target_ol_rel);

		if(cb) {
			if(auto w = military::find_war_between(state, source, target); w) {
				auto cb_infamy = military::cb_addition_infamy_cost(state, w, cb, source, target, target_state);

				color = text::text_color::red;
				set_text(state, text::format_float(cb_infamy, 1));
			} else if(military::has_truce_with(state, source, real_target)) {
				auto cb_infamy = military::truce_break_cb_infamy(state, cb, target, target_state);

				color = text::text_color::red;
				set_text(state, text::format_float(cb_infamy, 1));
			} else {
				auto cb_infamy = military::war_declaration_infamy_cost(state, cb, source, target, target_state);

				if(cb_infamy > 0.f) {
					color = text::text_color::red;
				}
				else {
					color = text::text_color::white;
				}
				set_text(state, text::format_float(cb_infamy, 1));
			}
		} else {
			color = text::text_color::white;
			set_text(state, "0.0");
		}
	}
};
class wargoal_add_militancy : public color_text_element {
public:
	void on_update(sys::state& state) noexcept override {
		auto cb = retrieve<dcon::cb_type_id>(state, parent);
		auto target = retrieve<dcon::nation_id>(state, parent);
		auto source = state.local_player_nation;

		dcon::nation_id real_target = target;

		auto target_ol_rel = state.world.nation_get_overlord_as_subject(target);
		if(state.world.overlord_get_ruler(target_ol_rel))
			real_target = state.world.overlord_get_ruler(target_ol_rel);

		if(cb) {
			if(auto w = military::find_war_between(state, source, target); w) {
				color = text::text_color::white;
				set_text(state, "0.0");
			} else if(military::has_truce_with(state, source, real_target)) {
				auto cb_militancy = military::truce_break_cb_militancy(state, cb);

				color = text::text_color::red;
				set_text(state, text::format_float(cb_militancy, 1));
			} else {
				color = text::text_color::white;
				set_text(state, "0.0");
			}
		} else {
			color = text::text_color::white;
			set_text(state, "0.0");
		}
	}
};

class wargoal_add_header : public simple_body_text {
public:
	void on_create(sys::state& state) noexcept override {
		simple_body_text::on_create(state);
		set_text(state, text::produce_simple_string(state, "wg_result_1"));
	}
};

class diplomacy_wargoal_add_window : public window_element_base {
public:
	std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
		if(name == "wargoal_gain_effect_text") {
			return make_element_by_type<wargoal_add_header>(state, id);
		} else if(name == "prestige_icon") {
			return make_element_by_type<image_element_base>(state, id);
		} else if(name == "prestige") {
			return make_element_by_type<wargoal_add_prestige>(state, id);
		} else if(name == "infamy_icon") {
			return make_element_by_type<image_element_base>(state, id);
		} else if(name == "infamy") {
			return make_element_by_type<wargoal_add_infamy>(state, id);
		} else if(name == "militancy_icon") {
			return make_element_by_type<image_element_base>(state, id);
		} else if(name == "militancy") {
			return make_element_by_type<wargoal_add_militancy>(state, id);
		} else {
			return nullptr;
		}
	}
};

class fixed_zero : public simple_text_element_base {
public:
	void on_create(sys::state& state) noexcept override {
		simple_text_element_base::on_create(state);
		set_text(state, "0.0");
	}
};


class wargoal_success_prestige : public color_text_element {
public:
	void on_update(sys::state& state) noexcept override {
		auto cb = retrieve<dcon::cb_type_id>(state, parent);
		auto source = state.local_player_nation;

		if(cb) {
			float prestige_gain = military::successful_cb_prestige(state, cb, source);
			if(prestige_gain > 0) {
				color = text::text_color::green;
				set_text(state, text::format_float(prestige_gain, 1));
			} else {
				color = text::text_color::white;
				set_text(state, "0.0");
			}
		} else {
			color = text::text_color::white;
			set_text(state, "0.0");
		}
	}
};

class wargoal_success_header : public simple_body_text {
public:
	void on_create(sys::state& state) noexcept override {
		simple_body_text::on_create(state);
		set_text(state, text::produce_simple_string(state, "wg_result_2"));
	}
};

class diplomacy_wargoal_success_window : public window_element_base {
public:
	std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
		if(name == "wargoal_gain_effect_text") {
			return make_element_by_type<wargoal_success_header>(state, id);
		} else if(name == "prestige_icon") {
			return make_element_by_type<image_element_base>(state, id);
		} else if(name == "prestige") {
			return make_element_by_type<wargoal_success_prestige>(state, id);
		} else if(name == "infamy_icon") {
			return make_element_by_type<image_element_base>(state, id);
		} else if(name == "infamy") {
			return make_element_by_type<fixed_zero>(state, id);
		} else if(name == "militancy_icon") {
			return make_element_by_type<image_element_base>(state, id);
		} else if(name == "militancy") {
			return make_element_by_type<fixed_zero>(state, id);
		} else {
			return nullptr;
		}
	}
};

class wargoal_failure_prestige : public color_text_element {
public:
	void on_update(sys::state& state) noexcept override {
		auto cb = retrieve<dcon::cb_type_id>(state, parent);
		auto source = state.local_player_nation;
		if(cb) {
			float prestige_loss = std::min(state.defines.war_failed_goal_prestige_base, state.defines.war_failed_goal_prestige * nations::prestige_score(state, source)) * state.world.cb_type_get_penalty_factor(cb);

			if(prestige_loss < 0) {
				color = text::text_color::red;
				set_text(state, text::format_float(prestige_loss, 1));
			} else {
				color = text::text_color::white;
				set_text(state, "0.0");
			}
		} else {
			color = text::text_color::white;
			set_text(state, "0.0");
		}
	}
};

class wargoal_failure_militancy : public color_text_element {
public:
	void on_update(sys::state& state) noexcept override {
		auto cb = retrieve<dcon::cb_type_id>(state, parent);
		if(cb) {
			auto pop_militancy = state.defines.war_failed_goal_militancy * state.world.cb_type_get_penalty_factor(cb);

			if(pop_militancy > 0) {
				color = text::text_color::red;
				set_text(state, text::format_float(pop_militancy, 1));
			} else {
				color = text::text_color::white;
				set_text(state, "0.0");
			}
		} else {
			color = text::text_color::white;
			set_text(state, "0.0");
		}
	}
};

class wargoal_failure_header : public simple_body_text {
public:
	void on_create(sys::state& state) noexcept override {
		simple_body_text::on_create(state);
		set_text(state, text::produce_simple_string(state, "wg_result_3"));
	}
};

class diplomacy_wargoal_failure_window : public window_element_base {
public:
	std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
		if(name == "wargoal_gain_effect_text") {
			return make_element_by_type<wargoal_failure_header>(state, id);
		} else if(name == "prestige_icon") {
			return make_element_by_type<image_element_base>(state, id);
		} else if(name == "prestige") {
			return make_element_by_type<wargoal_failure_prestige>(state, id);
		} else if(name == "infamy_icon") {
			return make_element_by_type<image_element_base>(state, id);
		} else if(name == "infamy") {
			return make_element_by_type<fixed_zero>(state, id);
		} else if(name == "militancy_icon") {
			return make_element_by_type<image_element_base>(state, id);
		} else if(name == "militancy") {
			return make_element_by_type<wargoal_failure_militancy>(state, id);
		} else {
			return nullptr;
		}
	}
};

class diplomacy_declare_war_title : public simple_text_element_base {
public:
	void on_update(sys::state& state) noexcept override {
		auto war = retrieve<dcon::war_id>(state, parent);
		if(!war) {
			set_text(state, text::produce_simple_string(state, "wartitle"));
		} else {
			set_text(state, text::produce_simple_string(state, "wargoaltitle"));
		}
	}
};

struct check_wg_completion {
	bool done = false;
};

enum diplomacy_declare_war_run_state {
	none, call_allies, run_conference
};

class diplomacy_declare_war_agree_button : public button_element_base {
public:
	void on_update(sys::state& state) noexcept override {
		auto wg_ready = retrieve<check_wg_completion>(state, parent).done;
		if(!wg_ready) {
			disabled = true;
			return;
		}

		dcon::nation_id n = retrieve<dcon::nation_id>(state, parent);
		dcon::state_definition_id s = retrieve<dcon::state_definition_id>(state, parent);
		dcon::national_identity_id ni = retrieve<dcon::national_identity_id>(state, parent);
		dcon::cb_type_id c = retrieve<dcon::cb_type_id>(state, parent);

		if(military::are_at_war(state, state.local_player_nation, n)) {
			dcon::war_id w = military::find_war_between(state, state.local_player_nation, n);
			disabled = !command::can_add_war_goal(state, state.local_player_nation, w, n, c, s, ni,
					state.world.national_identity_get_nation_from_identity_holder(ni));
		} else {
			disabled = !command::can_declare_war(state, state.local_player_nation, n, c, s, ni,
					state.world.national_identity_get_nation_from_identity_holder(ni));
		}
	}

	void button_action(sys::state& state) noexcept override {
		dcon::nation_id n = retrieve<dcon::nation_id>(state, parent);
		dcon::state_definition_id s = retrieve<dcon::state_definition_id>(state, parent);
		dcon::national_identity_id ni = retrieve<dcon::national_identity_id>(state, parent);
		dcon::cb_type_id c = retrieve<dcon::cb_type_id>(state, parent);

		auto checkboxes = retrieve<diplomacy_declare_war_run_state>(state, parent);
		if(military::are_at_war(state, state.local_player_nation, n)) {
			dcon::war_id w = military::find_war_between(state, state.local_player_nation, n);
			command::add_war_goal(state, state.local_player_nation, w, n, c, s, ni,
					state.world.national_identity_get_nation_from_identity_holder(ni));
		} else {
			command::declare_war(state, state.local_player_nation, n, c, s, ni,
					state.world.national_identity_get_nation_from_identity_holder(ni),
				checkboxes == diplomacy_declare_war_run_state::call_allies,
				checkboxes == diplomacy_declare_war_run_state::run_conference);
		}
		parent->set_visible(state, false);
	}

	tooltip_behavior has_tooltip(sys::state& state) noexcept override {
		return tooltip_behavior::variable_tooltip;
	}

	void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
		auto wg_ready = retrieve<check_wg_completion>(state, parent).done;
		if(!wg_ready) {
			text::add_line_with_condition(state, contents, "wg_not_ready", false);
			return;
		}

		dcon::nation_id n = retrieve<dcon::nation_id>(state, parent);
		dcon::state_definition_id s = retrieve<dcon::state_definition_id>(state, parent);
		dcon::national_identity_id ni = retrieve<dcon::national_identity_id>(state, parent);
		dcon::cb_type_id c = retrieve<dcon::cb_type_id>(state, parent);

		if(command::can_declare_war(state, state.local_player_nation, n, c, s, ni,
			state.world.national_identity_get_nation_from_identity_holder(ni))) {
			auto box = text::open_layout_box(contents, 0);
			text::localised_format_box(state, contents, box, std::string_view("valid_wartarget"));
			text::close_layout_box(contents, box);
			text::add_line_with_condition(state, contents, "alice_condition_diplo_points", !(state.world.nation_get_is_player_controlled(state.local_player_nation) && state.world.nation_get_diplomatic_points(state.local_player_nation) < state.defines.declarewar_diplomatic_cost), text::variable_type::x, int64_t(state.defines.declarewar_diplomatic_cost));
		} else {
			dcon::war_id w = military::find_war_between(state, state.local_player_nation, n);
			auto box = text::open_layout_box(contents, 0);
			if(military::are_allied_in_war(state, state.local_player_nation, n)) {
				text::localised_format_box(state, contents, box, std::string_view("invalid_wartarget_shared_war"));
			}
			auto rel = state.world.get_diplomatic_relation_by_diplomatic_pair(state.local_player_nation, n);
			if(state.world.diplomatic_relation_get_are_allied(rel)) {
				text::localised_format_box(state, contents, box, std::string_view("no_war_allied"));
			}
			text::close_layout_box(contents, box);
			text::add_line_with_condition(state, contents, "alice_condition_diplo_points", !(state.world.nation_get_is_player_controlled(state.local_player_nation) && state.world.nation_get_diplomatic_points(state.local_player_nation) < state.defines.addwargoal_diplomatic_cost), text::variable_type::x, int64_t(state.defines.addwargoal_diplomatic_cost));

			bool is_attacker = military::is_attacker(state, w, state.local_player_nation);
			bool target_in_war = false;
			for(auto par : state.world.war_get_war_participant(w)) {
				if(par.get_nation() == n) {
					text::add_line_with_condition(state, contents, "alice_wg_condition_4", !(par.get_is_attacker() == is_attacker));
					target_in_war = true;
					break;
				}
			}
			text::add_line_with_condition(state, contents, "alice_wg_condition_1", !(!is_attacker && military::defenders_have_status_quo_wargoal(state, w)));
			text::add_line_with_condition(state, contents, "alice_wg_condition_2", bool(!target_in_war));
			text::add_line_with_condition(state, contents, "alice_wg_condition_3", !(military::war_goal_would_be_duplicate(state, state.local_player_nation, w, n, c, s, ni, state.world.national_identity_get_nation_from_identity_holder(ni))));

			if((state.world.cb_type_get_type_bits(c) & military::cb_flag::always) == 0) {
				bool cb_fabbed = false;
				for(auto& fab_cb : state.world.nation_get_available_cbs(state.local_player_nation)) {
					if(fab_cb.cb_type == c && fab_cb.target == n) {
						cb_fabbed = true;
						break;
					}
				}
				if(!cb_fabbed) {
					text::add_line_with_condition(state, contents, "alice_wg_condition_7", !((state.world.cb_type_get_type_bits(c) & military::cb_flag::is_not_constructing_cb) != 0));
					auto totalpop = state.world.nation_get_demographics(state.local_player_nation, demographics::total);
					auto jingoism_perc = totalpop > 0 ? state.world.nation_get_demographics(state.local_player_nation, demographics::to_key(state, state.culture_definitions.jingoism)) / totalpop : 0.0f;
					if(state.world.war_get_is_great(w)) {
						text::add_line_with_condition(state, contents, "alice_wg_condition_6", jingoism_perc >= state.defines.gw_wargoal_jingoism_requirement_mod,
							text::variable_type::need, text::fp_two_places{ state.defines.gw_wargoal_jingoism_requirement_mod },
							text::variable_type::value, text::fp_two_places{ jingoism_perc });
					} else {
						text::add_line_with_condition(state, contents, "alice_wg_condition_6", jingoism_perc >= state.defines.wargoal_jingoism_requirement,
							text::variable_type::need, text::fp_two_places{ state.defines.wargoal_jingoism_requirement },
							text::variable_type::value, text::fp_two_places{ jingoism_perc });
					}
				}
			}
		}
		text::add_line_with_condition(state, contents, "alice_wg_condition_5", military::cb_instance_conditions_satisfied(state, state.local_player_nation, n, c, s, ni, state.world.national_identity_get_nation_from_identity_holder(ni)));

		if(auto can_use = state.world.cb_type_get_can_use(c); can_use) {
			text::add_line(state, contents, "alice_wg_usage_trigger");
			ui::trigger_description(state, contents, can_use, trigger::to_generic(n), trigger::to_generic(state.local_player_nation), trigger::to_generic(n));
		}
	}
};


int32_t calculate_partial_score(sys::state& state, dcon::nation_id target, dcon::cb_type_id id, dcon::state_definition_id state_def, dcon::national_identity_id second_nation) {
	int32_t cost = -1;

	auto war = military::find_war_between(state, state.local_player_nation, target);
	if(!military::cb_requires_selection_of_a_state(state, id) && !military::cb_requires_selection_of_a_liberatable_tag(state, id) && !military::cb_requires_selection_of_a_valid_nation(state, id)) {

		cost = military::peace_cost(state, military::find_war_between(state, state.local_player_nation, target), id, state.local_player_nation, target, dcon::nation_id{}, dcon::state_definition_id{}, dcon::national_identity_id{});
	} else if(military::cb_requires_selection_of_a_state(state, id)) {

		if(state_def) {
			if(military::cb_requires_selection_of_a_liberatable_tag(state, id)) {
				if(!second_nation) {
					return -1;
				}
			} else if(military::cb_requires_selection_of_a_valid_nation(state, id)) {
				if(!second_nation) {
					return -1;
				}
			}

			cost = 0;

			// for each state ...
			if(war) {
				auto is_attacker = military::is_attacker(state, war, state.local_player_nation);
				for(auto si : state.world.in_state_instance) {
					if(si.get_definition() == state_def) {
						auto wr = military::get_role(state, war, si.get_nation_from_state_ownership());
						if((is_attacker && wr == military::war_role::defender) || (!is_attacker && wr == military::war_role::attacker)) {
							if(military::cb_requires_selection_of_a_liberatable_tag(state, id)) {
								cost += military::peace_cost(state, war, id, state.local_player_nation, si.get_nation_from_state_ownership(), dcon::nation_id{}, state_def, second_nation);
							} else if(military::cb_requires_selection_of_a_valid_nation(state, id)) {
								cost += military::peace_cost(state, war, id, state.local_player_nation, si.get_nation_from_state_ownership(), state.world.national_identity_get_nation_from_identity_holder(second_nation), state_def, dcon::national_identity_id{});
							} else {
								cost += military::peace_cost(state, war, id, state.local_player_nation, si.get_nation_from_state_ownership(), dcon::nation_id{}, state_def, dcon::national_identity_id{});
							}
						}
					}
				}
			} else {
				for(auto si : state.world.in_state_instance) {
					if(si.get_definition() == state_def) {
						auto n = si.get_nation_from_state_ownership();
						auto no = n.get_overlord_as_subject().get_ruler();
						if(n == target || no == target) {
							if(military::cb_requires_selection_of_a_liberatable_tag(state, id)) {
								cost += military::peace_cost(state, dcon::war_id{}, id, state.local_player_nation, si.get_nation_from_state_ownership(), dcon::nation_id{}, state_def, second_nation);
							} else if(military::cb_requires_selection_of_a_valid_nation(state, id)) {
								cost += military::peace_cost(state, dcon::war_id{}, id, state.local_player_nation, si.get_nation_from_state_ownership(), state.world.national_identity_get_nation_from_identity_holder(second_nation), state_def, dcon::national_identity_id{});
							} else {
								cost += military::peace_cost(state, dcon::war_id{}, id, state.local_player_nation, si.get_nation_from_state_ownership(), dcon::nation_id{}, state_def, dcon::national_identity_id{});
							}
						}
					}
				}
			}
		}
	} else if(military::cb_requires_selection_of_a_liberatable_tag(state, id)) {
		if(second_nation) {
			cost = military::peace_cost(state, military::find_war_between(state, state.local_player_nation, target), id, state.local_player_nation, target, dcon::nation_id{}, dcon::state_definition_id{}, second_nation);
		}
	} else if(military::cb_requires_selection_of_a_valid_nation(state, id)) {
		if(second_nation) {
			cost = military::peace_cost(state, military::find_war_between(state, state.local_player_nation, target), id, state.local_player_nation, target, state.world.national_identity_get_nation_from_identity_holder(second_nation), dcon::state_definition_id{}, dcon::national_identity_id{});
		}
	}
	return cost;
}

class diplomacy_declare_war_description1 : public simple_multiline_body_text {
public:
	void populate_layout(sys::state& state, text::endless_layout& contents) noexcept override {
		auto id = retrieve<dcon::cb_type_id>(state, parent);
		auto fat_cb = dcon::fatten(state.world, id);
		auto box = text::open_layout_box(contents);
		auto target = retrieve<dcon::nation_id>(state, parent);

		text::substitution_map sub;

		if(id) {
			text::add_to_substitution_map(sub, text::variable_type::goal, fat_cb.get_name());
			text::localised_format_box(state, contents, box, std::string_view("add_wargoal_wargoal"), sub);
		} else {
			auto str = text::produce_simple_string(state, "nocasusbelli");
			text::add_to_substitution_map(sub, text::variable_type::goal, std::string_view(str));
			text::localised_format_box(state, contents, box, std::string_view("add_wargoal_wargoal"), sub);
		}
		text::close_layout_box(contents, box);

		if(id) {
			auto state_def = retrieve<dcon::state_definition_id>(state, parent);
			auto second_nation = retrieve<dcon::national_identity_id>(state, parent);
			auto cost = calculate_partial_score(state, target, id, state_def, second_nation);
			if(cost != -1) {
				text::add_line(state, contents, "add_wargoal_peace_cost", text::variable_type::cost, int64_t(cost));
			}
		}

	}
};

class diplomacy_declare_war_description2 : public simple_multiline_body_text {
public:
	void populate_layout(sys::state& state, text::endless_layout& contents) noexcept override {
		auto id = retrieve<dcon::cb_type_id>(state, parent);

		auto content = retrieve<dcon::nation_id>(state, parent);
		auto sdef = retrieve<dcon::state_definition_id>(state, parent);
		auto nid = retrieve<dcon::national_identity_id>(state, parent);
		auto fat_cb = dcon::fatten(state.world, id);
		auto box = text::open_layout_box(contents);

		if(id) {
			text::substitution_map sub;
			text::add_to_substitution_map(sub, text::variable_type::recipient, content); // Target Nation
			text::add_to_substitution_map(sub, text::variable_type::third, nid); // Third Party Country
			text::add_to_substitution_map(sub, text::variable_type::actor, state.local_player_nation);
			text::add_to_substitution_map(sub, text::variable_type::state, sdef);
			text::add_to_substitution_map(sub, text::variable_type::region,sdef);
			text::add_to_layout_box(state, contents, box, fat_cb.get_long_desc(), sub);
		}
		text::close_layout_box(contents, box);
	}
};

class diplomacy_declare_war_call_allies_checkbox : public button_element_base {
public:
	bool show = true;

	void button_action(sys::state& state) noexcept override {
		if(parent) {
			auto content = retrieve<diplomacy_declare_war_run_state>(state, parent);
			auto checked = content == diplomacy_declare_war_run_state::call_allies;
			Cyto::Any b_payload = element_selection_wrapper<diplomacy_declare_war_run_state>{ diplomacy_declare_war_run_state::call_allies };

			if(checked) {
				b_payload = element_selection_wrapper<diplomacy_declare_war_run_state>{ diplomacy_declare_war_run_state::none };
			}
			parent->impl_get(state, b_payload);
		}
	}

	void on_update(sys::state& state) noexcept override {
		auto content = retrieve<diplomacy_declare_war_run_state>(state, parent);
		auto checked = content == diplomacy_declare_war_run_state::call_allies;
		frame = checked ? 1 : 0;
		auto war = retrieve<dcon::war_id>(state, parent);
		show = !bool(war);
	}

	void render(sys::state& state, int32_t x, int32_t y) noexcept override {
		if(show)
			button_element_base::render(state, x, y);
	}
	tooltip_behavior has_tooltip(sys::state& state) noexcept override {
		return show ? tooltip_behavior::variable_tooltip : tooltip_behavior::no_tooltip;
	}
	void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
		if(!show)
			return;

		auto target = retrieve<dcon::nation_id>(state, parent);

		bool has_allies = false;
		for(auto dr : state.world.nation_get_diplomatic_relation(state.local_player_nation)) {
			if(dr.get_are_allied()) {
				auto other = dr.get_related_nations(0) == state.local_player_nation ? dr.get_related_nations(1) : dr.get_related_nations(0);
				if(other.get_is_player_controlled()) {
					text::add_line(state, contents, "att_call_is_human", text::variable_type::x, other.id);
				} else {
					if(target == other || military::are_in_common_war(state, target, other) || military::are_at_war(state, target, other) || nations::are_allied(state, target, other)) {
						text::add_line(state, contents, "att_call_will_decline", text::variable_type::x, other.id);
					} else {
						bool will_join = false;

						if(military::can_use_cb_against(state, other, target))
							will_join = true;
						if(state.world.nation_get_ai_rival(other) == target)
							will_join = true;

						// TODO: check subjects, other nations that can be expected to defensively answer the CTA

						if(will_join) {
							text::add_line(state, contents, "att_call_will_accept", text::variable_type::x, other.id);
						} else {
							text::add_line(state, contents, "att_call_will_decline", text::variable_type::x, other.id);
						}
					}
				}
				has_allies = true;
			}
		}
		if(!has_allies) {
			text::add_line(state, contents, "att_call_no_allies");
		}
	}
};

class diplomacy_declare_war_run_conference_checkbox : public button_element_base {
public:
	bool show = true;

	void button_action(sys::state& state) noexcept override {
		if(parent) {
			auto content = retrieve<diplomacy_declare_war_run_state>(state, parent);
			auto checked = content == diplomacy_declare_war_run_state::run_conference;

			Cyto::Any b_payload = element_selection_wrapper<diplomacy_declare_war_run_state>{ diplomacy_declare_war_run_state::run_conference };

			if(checked) {
				b_payload = element_selection_wrapper<diplomacy_declare_war_run_state>{ diplomacy_declare_war_run_state::none };
			}
			parent->impl_get(state, b_payload);
		}
	}

	void on_update(sys::state& state) noexcept override {
		auto content = retrieve<diplomacy_declare_war_run_state>(state, parent);
		auto checked = content == diplomacy_declare_war_run_state::run_conference;
		disabled = state.current_crisis_state != sys::crisis_state::inactive || state.network_mode == sys::network_mode_type::single_player;
		if(disabled) {
			checked = false;
		}

		frame = checked ? 1 : 0;
		auto war = retrieve<dcon::war_id>(state, parent);
		show = !bool(war);
	}

	void render(sys::state& state, int32_t x, int32_t y) noexcept override {
		if(show)
			button_element_base::render(state, x, y);
	}
	tooltip_behavior has_tooltip(sys::state& state) noexcept override {
		return show ? tooltip_behavior::variable_tooltip : tooltip_behavior::no_tooltip;
	}
	void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
		if(!show)
			return;
	}
};

class diplomacy_declare_war_call_allies_text : public simple_text_element_base {
public:
	bool show = true;

	void on_update(sys::state& state) noexcept override {
		auto war = retrieve<dcon::war_id>(state, parent);
		show = !bool(war);
	}

	void render(sys::state& state, int32_t x, int32_t y) noexcept override {
		if(show)
			simple_text_element_base::render(state, x, y);
	}
};

class cancel_pick_wg_button : public button_element_base {
public:
	void button_action(sys::state& state) noexcept override {
		if(parent) {
			parent->set_visible(state, false);
		}
		game_scene::switch_scene(state, game_scene::scene_id::in_game_basic);
	}
};

class diplomacy_declare_war_dialog : public window_element_base { // eu3dialogtype
private:
	wargoal_setup_window* wargoal_setup_win = nullptr;
	wargoal_country_select_window* wargoal_country_win = nullptr;

	dcon::cb_type_id cb_to_use;
	dcon::state_definition_id target_state;
	dcon::national_identity_id target_country;
	diplomacy_declare_war_run_state checkboxes_state;
	bool wargoal_decided_upon = false;

	void select_mode(sys::state& state) {
		sys::state_selection_data seldata;
		seldata.single_state_select = true;
		dcon::nation_id target = retrieve<dcon::nation_id>(state, parent);
		auto actor = state.local_player_nation;
		dcon::cb_type_id cb = cb_to_use;
		auto war = military::find_war_between(state, actor, target);
		auto secondary_tag = target_country;
		auto allowed_substate_regions = state.world.cb_type_get_allowed_substate_regions(cb);

		// Add target states to the selection
		if ((state.world.cb_type_get_type_bits(cb) & military::cb_flag::always) == 0) {
			for(auto available_cb : state.world.nation_get_available_cbs(state.local_player_nation)) {
				if(available_cb.cb_type != cb || available_cb.target != target || !available_cb.target_state) {
					continue;
				}

				auto def = available_cb.target_state;
				if(!military::war_goal_would_be_duplicate(state, state.local_player_nation, war, target, cb, def, dcon::national_identity_id{}, dcon::nation_id{})) {
					seldata.selectable_states.push_back(def);
				}
			}
		}
		// Country has "blank check" CB
		if (seldata.selectable_states.empty()) {
			if(allowed_substate_regions) {
				for(auto v : state.world.nation_get_overlord_as_ruler(target)) {
					if(v.get_subject().get_is_substate()) {
						for(auto si : state.world.nation_get_state_ownership(target)) {
							if(trigger::evaluate(state, allowed_substate_regions, trigger::to_generic(si.get_state().id), trigger::to_generic(actor), trigger::to_generic(actor))) {
								auto def = si.get_state().get_definition().id;
								if(std::find(seldata.selectable_states.begin(), seldata.selectable_states.end(), def) == seldata.selectable_states.end()) {
									if(!military::war_goal_would_be_duplicate(state, state.local_player_nation, war, v.get_subject(), cb, def, dcon::national_identity_id{}, dcon::nation_id{})) {
										seldata.selectable_states.push_back(def);
									}
								}
							}
						}
					}
				}
			} else {
				auto allowed_states = state.world.cb_type_get_allowed_states(cb);
				if(auto ac = state.world.cb_type_get_allowed_countries(cb); ac) {
					auto in_nation = state.world.national_identity_get_nation_from_identity_holder(secondary_tag);
					for(auto si : state.world.nation_get_state_ownership(target)) {
						if(trigger::evaluate(state, allowed_states, trigger::to_generic(si.get_state().id), trigger::to_generic(actor), trigger::to_generic(in_nation))) {
							auto def = si.get_state().get_definition().id;
							if(!military::war_goal_would_be_duplicate(state, state.local_player_nation, war, target, cb, def, secondary_tag, dcon::nation_id{})) {
								seldata.selectable_states.push_back(def);
							}
						}
					}
				} else {
					for(auto si : state.world.nation_get_state_ownership(target)) {
						if(trigger::evaluate(state, allowed_states, trigger::to_generic(si.get_state().id), trigger::to_generic(actor), trigger::to_generic(actor))) {
							auto def = si.get_state().get_definition().id;
							if(!military::war_goal_would_be_duplicate(state, state.local_player_nation, war, target, cb, def, dcon::national_identity_id{}, dcon::nation_id{})) {
								seldata.selectable_states.push_back(def);
							}
						}
					}
				}
			}
		}
		seldata.on_select = [&](sys::state& state, dcon::state_definition_id sdef) {
			target_state = sdef;
			wargoal_decided_upon = true;
			wargoal_setup_win->set_visible(state, true);
			wargoal_country_win->set_visible(state, false);
			impl_on_update(state);
		};
		seldata.on_cancel = [&](sys::state& state) {
			target_state = dcon::state_definition_id{};
			if(military::cb_requires_selection_of_a_valid_nation(state, cb_to_use) || military::cb_requires_selection_of_a_liberatable_tag(state, cb_to_use)) {
				wargoal_setup_win->set_visible(state, false);
				wargoal_country_win->set_visible(state, true);
			} else {
				cb_to_use = dcon::cb_type_id{};
				wargoal_decided_upon = false;
				wargoal_setup_win->set_visible(state, true);
				wargoal_country_win->set_visible(state, false);
			}
			impl_on_update(state);
		};
		state.start_state_selection(seldata);
	}
public:
	void reset_window(sys::state& state) {
		cb_to_use = dcon::cb_type_id{};
		target_state = dcon::state_definition_id{};
		target_country = dcon::national_identity_id{};
		checkboxes_state = diplomacy_declare_war_run_state::none;
		wargoal_decided_upon = false;
		wargoal_setup_win->set_visible(state, true);
		wargoal_country_win->set_visible(state, false);
	}

	std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
		if(name == "background") {
			auto ptr = make_element_by_type<draggable_target>(state, id);
			ptr->base_data.size = base_data.size; // Nudge size for proper sizing
			return ptr;
		} else if(name == "diplo_declarewar_bg") {
			return make_element_by_type<image_element_base>(state, id);
		} else if(name == "leftshield") {
			return make_element_by_type<nation_player_flag>(state, id);
		} else if(name == "rightshield") {
			return make_element_by_type<flag_button>(state, id);
		} else if(name == "title") {
			return make_element_by_type<diplomacy_declare_war_title>(state, id);
		} else if(name == "description") {
			return make_element_by_type<diplomacy_declare_war_description1>(state, id);
		} else if(name == "wargoal_add_effect") {
			return make_element_by_type<diplomacy_wargoal_add_window>(state, id);
		} else if(name == "wargoal_success_effect") {
			return make_element_by_type<diplomacy_wargoal_success_window>(state, id);
		} else if(name == "wargoal_failure_effect") {
			return make_element_by_type<diplomacy_wargoal_failure_window>(state, id);
		} else if(name == "description2") {
			return make_element_by_type<diplomacy_declare_war_description2>(state, id);
		} else if(name == "acceptance") {
			return make_element_by_type<simple_text_element_base>(state, id);
		} else if(name == "call_allies_checkbox") {
			return make_element_by_type<diplomacy_declare_war_call_allies_checkbox>(state, id);
		} else if(name == "run_conference_checkbox") {
			return make_element_by_type<diplomacy_declare_war_run_conference_checkbox>(state, id);
		} else if(name == "call_allies_text") {
			return make_element_by_type<diplomacy_declare_war_call_allies_text>(state, id);
		} else if(name == "agreebutton") {
			return make_element_by_type<diplomacy_declare_war_agree_button>(state, id);
		} else if(name == "declinebutton") {
			return make_element_by_type<cancel_pick_wg_button>(state, id);
		} else if(name == "wargoal_setup") {
			auto ptr = make_element_by_type<wargoal_setup_window>(state, id);
			wargoal_setup_win = ptr.get();
			ptr->set_visible(state, true);
			return ptr;
		} else if(name == "wargoal_state_select") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "wargoal_country_select") {
			auto ptr = make_element_by_type<wargoal_country_select_window>(state, id);
			wargoal_country_win = ptr.get();
			ptr->set_visible(state, false);
			return ptr;
		} else {
			return nullptr;
		}
	}

	message_result get(sys::state& state, Cyto::Any& payload) noexcept override {
		if(payload.holds_type<dcon::war_id>()) {
			const dcon::nation_id n = retrieve<dcon::nation_id>(state, parent);
			const dcon::war_id w = military::find_war_between(state, state.local_player_nation, n);
			payload.emplace<dcon::war_id>(w);
			return message_result::consumed;
		} else if(payload.holds_type< check_wg_completion>()) {
			payload.emplace<check_wg_completion>(check_wg_completion{ wargoal_decided_upon });
			return message_result::consumed;
		} else if(payload.holds_type<element_selection_wrapper<dcon::cb_type_id>>()) {
			cb_to_use = any_cast<element_selection_wrapper<dcon::cb_type_id>>(payload).data;
			if(!cb_to_use) {
				wargoal_decided_upon = false;
			} else if(military::cb_requires_selection_of_a_valid_nation(state, cb_to_use) || military::cb_requires_selection_of_a_liberatable_tag(state, cb_to_use)) {
				wargoal_setup_win->set_visible(state, false);
				wargoal_country_win->set_visible(state, true);
			} else if(military::cb_requires_selection_of_a_state(state, cb_to_use)) {
				wargoal_setup_win->set_visible(state, true);
				wargoal_country_win->set_visible(state, false);
				select_mode(state);
			} else {
				wargoal_decided_upon = true;
				wargoal_setup_win->set_visible(state, true);
				wargoal_country_win->set_visible(state, false);
			}
			impl_on_update(state);
			return message_result::consumed;
		} else if(payload.holds_type<element_selection_wrapper<dcon::national_identity_id>>()) {
			target_country = any_cast<element_selection_wrapper<dcon::national_identity_id>>(payload).data;
			if(target_country) { // goto next step
				if(military::cb_requires_selection_of_a_state(state, cb_to_use)) {
					wargoal_setup_win->set_visible(state, true);
					wargoal_country_win->set_visible(state, false);
					select_mode(state);
				} else {
					wargoal_decided_upon = true;
					wargoal_setup_win->set_visible(state, true);
					wargoal_country_win->set_visible(state, false);
				}
			} else {
				wargoal_decided_upon = false;
				cb_to_use = dcon::cb_type_id{};
				wargoal_setup_win->set_visible(state, true);
				wargoal_country_win->set_visible(state, false);
			}
			impl_on_update(state);
			return message_result::consumed;
		} else if(payload.holds_type<element_selection_wrapper<diplomacy_declare_war_run_state>>()) {
			checkboxes_state = any_cast<element_selection_wrapper<diplomacy_declare_war_run_state>>(payload).data;
			impl_on_update(state);
			return message_result::consumed;
		} else if(payload.holds_type<dcon::cb_type_id>()) {
			payload.emplace<dcon::cb_type_id>(cb_to_use);
			return message_result::consumed;
		} else if(payload.holds_type<dcon::state_definition_id>()) {
			payload.emplace<dcon::state_definition_id>(target_state);
			return message_result::consumed;
		} else if(payload.holds_type<dcon::national_identity_id>()) {
			payload.emplace<dcon::national_identity_id>(target_country);
			return message_result::consumed;
		} else if(payload.holds_type<diplomacy_declare_war_run_state>()) {
			payload.emplace<diplomacy_declare_war_run_state>(checkboxes_state);
			return message_result::consumed;
		}
		return message_result::unseen;
	}
};

struct get_target {
	dcon::nation_id n;
};
struct get_offer_to {
	dcon::nation_id n;
};
struct set_target {
	dcon::nation_id n;
};

class wargoal_offer_description1 : public simple_multiline_body_text {
public:
	void populate_layout(sys::state& state, text::endless_layout& contents) noexcept override {
		auto id = retrieve<dcon::cb_type_id>(state, parent);
		auto fat_cb = dcon::fatten(state.world, id);
		auto box = text::open_layout_box(contents);
		auto target = retrieve<get_target>(state, parent).n;

		text::substitution_map sub;

		if(id) {
			text::add_to_substitution_map(sub, text::variable_type::goal, fat_cb.get_name());
			text::localised_format_box(state, contents, box, std::string_view("add_wargoal_wargoal"), sub);
		} else {
			auto str = text::produce_simple_string(state, "nocasusbelli");
			text::add_to_substitution_map(sub, text::variable_type::goal, std::string_view(str));
			text::localised_format_box(state, contents, box, std::string_view("add_wargoal_wargoal"), sub);
		}
		text::close_layout_box(contents, box);

		if(id) {
			auto state_def = retrieve<dcon::state_definition_id>(state, parent);
			auto second_nation = retrieve<dcon::national_identity_id>(state, parent);
			auto cost = calculate_partial_score(state, target, id, state_def, second_nation);
			if(cost != -1) {
				text::add_line(state, contents, "add_wargoal_peace_cost", text::variable_type::cost, int64_t(cost));
			}
		}

	}
};

class wargoal_offer_description2 : public simple_multiline_body_text {
public:
	void populate_layout(sys::state& state, text::endless_layout& contents) noexcept override {
		auto id = retrieve<dcon::cb_type_id>(state, parent);

		auto content = retrieve<get_target>(state, parent).n;
		auto dest = retrieve<get_offer_to>(state, parent).n;
		auto staat = retrieve<dcon::state_definition_id>(state, parent);
		auto nacion = retrieve<dcon::national_identity_id>(state, parent);

		auto fat_cb = dcon::fatten(state.world, id);

		auto box = text::open_layout_box(contents);

		if(id) {
			text::substitution_map sub;
			text::add_to_substitution_map(sub, text::variable_type::recipient, content); // Target Nation
			text::add_to_substitution_map(sub, text::variable_type::third, nacion); // Third Party Country
			text::add_to_substitution_map(sub, text::variable_type::actor, dest);
			text::add_to_substitution_map(sub, text::variable_type::state,  staat);
			text::add_to_substitution_map(sub, text::variable_type::region, staat);

			text::add_to_layout_box(state, contents, box, fat_cb.get_long_desc(), sub);
		}

		text::close_layout_box(contents, box);
	}
};


class wargoal_offer_agree_button : public button_element_base {
public:
	void on_update(sys::state& state) noexcept override {
		auto wg_ready = retrieve<check_wg_completion>(state, parent).done;
		if(!wg_ready) {
			disabled = true;
			return;
		}
		auto ni = retrieve<dcon::national_identity_id>(state, parent);
		disabled = !command::can_invite_to_crisis(state, state.local_player_nation, retrieve<get_offer_to>(state, parent).n, retrieve<get_target>(state, parent).n, retrieve<dcon::cb_type_id>(state, parent), retrieve<dcon::state_definition_id>(state, parent), ni, state.world.national_identity_get_nation_from_identity_holder(ni));
	}

	void button_action(sys::state& state) noexcept override {
		auto ni = retrieve<dcon::national_identity_id>(state, parent);
		command::invite_to_crisis(state, state.local_player_nation, retrieve<get_offer_to>(state, parent).n, retrieve<get_target>(state, parent).n, retrieve<dcon::cb_type_id>(state, parent), retrieve<dcon::state_definition_id>(state, parent), ni, state.world.national_identity_get_nation_from_identity_holder(ni));
		parent->set_visible(state, false);
	}

	tooltip_behavior has_tooltip(sys::state& state) noexcept override {
		return tooltip_behavior::variable_tooltip;
	}

	void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
		auto wg_ready = retrieve<check_wg_completion>(state, parent).done;
		if(!wg_ready) {
			text::add_line_with_condition(state, contents, "wg_not_ready", false);
			return;
		}
	}
};

class wargoal_offer_title : public simple_text_element_base {
public:
	void on_create(sys::state& state) noexcept override {
		simple_text_element_base::on_create(state);
		set_text(state, text::produce_simple_string(state, "crisis_offertitle"));
	}
};


class wargoal_offer_add_infamy : public color_text_element {
public:
	void on_update(sys::state& state) noexcept override {
		auto cb = retrieve<dcon::cb_type_id>(state, parent);
		auto target = retrieve<get_target>(state, parent).n;
		auto target_state = retrieve<dcon::state_definition_id>(state, parent);
		auto offered = retrieve<get_offer_to>(state, parent).n;

		auto infamy = military::crisis_cb_addition_infamy_cost(state, cb, offered, target, target_state);

		if(infamy > 0) {
			color = text::text_color::red;
			set_text(state, text::format_float(infamy, 1));
		} else {
			color = text::text_color::white;
			set_text(state, "0.0");
		}
	}
};


class wargoal_offer_add_window : public window_element_base {
public:
	std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
		if(name == "wargoal_gain_effect_text") {
			return make_element_by_type<wargoal_add_header>(state, id);
		} else if(name == "prestige_icon") {
			return make_element_by_type<image_element_base>(state, id);
		} else if(name == "prestige") {
			return make_element_by_type<fixed_zero>(state, id);
		} else if(name == "infamy_icon") {
			return make_element_by_type<image_element_base>(state, id);
		} else if(name == "infamy") {
			return make_element_by_type<wargoal_offer_add_infamy>(state, id);
		} else if(name == "militancy_icon") {
			return make_element_by_type<image_element_base>(state, id);
		} else if(name == "militancy") {
			return make_element_by_type<fixed_zero>(state, id);
		} else {
			return nullptr;
		}
	}
};

class wargoal_offer_type_listbox : public listbox_element_base<wargoal_type_item, dcon::cb_type_id> {
protected:
	std::string_view get_row_element_name() override {
		return "wargoal_item";
	}

public:
	void on_update(sys::state& state) noexcept override {
		row_contents.clear();

		auto selected_cb = retrieve<dcon::cb_type_id>(state, parent);
		if(selected_cb) {
			row_contents.push_back(selected_cb);
			update(state);
			return;
		}

		dcon::nation_id actor = retrieve<get_offer_to>(state, parent).n;
		dcon::nation_id content = retrieve<get_target>(state, parent).n;

		for(auto cb_type : state.world.in_cb_type) {
			
			if(military::cb_conditions_satisfied(state, actor, content, cb_type)) {
				if((cb_type.get_type_bits() & military::cb_flag::always) != 0)
					row_contents.push_back(cb_type);
				else if((cb_type.get_type_bits() & military::cb_flag::is_not_constructing_cb) == 0)
					row_contents.push_back(cb_type);
			}
		}

		update(state);
	}
};

class wargoal_offer_setup_window : public window_element_base {
public:
	std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
		if(name == "country_list") {
			return make_element_by_type<wargoal_offer_type_listbox>(state, id);
		} else if(name == "cancel_select") {
			return make_element_by_type<wargoal_cancel_button>(state, id);
		} else {
			return nullptr;
		}
	}
};


class wargoal_target_country_item_button : public button_element_base {
public:
	void button_action(sys::state& state) noexcept override {
		auto n = retrieve<dcon::nation_id>(state, parent);
		send(state, parent, set_target{ n });
	}

	void on_update(sys::state& state) noexcept override {
		auto n = retrieve<dcon::nation_id>(state, parent);
		set_button_text(state, text::produce_simple_string(state, text::get_name(state, n)));
	}
	tooltip_behavior has_tooltip(sys::state& state) noexcept override {
		return tooltip_behavior::variable_tooltip;
	}
	void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
		dcon::cb_type_id cb = retrieve<dcon::cb_type_id>(state, parent);
		if(auto allowed_countries = state.world.cb_type_get_allowed_countries(cb); allowed_countries) {
			dcon::nation_id target = retrieve<dcon::nation_id>(state, parent);
			auto holder = state.world.national_identity_get_nation_from_identity_holder(retrieve<dcon::national_identity_id>(state, parent));
			trigger_description(state, contents, allowed_countries, trigger::to_generic(target), trigger::to_generic(state.local_player_nation), trigger::to_generic(holder));
		}
	}
};

class wargoal_target_country_item : public listbox_row_element_base<dcon::nation_id> {
public:
	std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
		if(name == "select_country") {
			return make_element_by_type<wargoal_target_country_item_button>(state, id);
		} else {
			return nullptr;
		}
	}
};

class wargoal_target_country_listbox : public listbox_element_base<wargoal_target_country_item, dcon::nation_id> {
protected:
	std::string_view get_row_element_name() override {
		return "wargoal_country_item";
	}

public:
	void on_update(sys::state& state) noexcept override {
		row_contents.clear();
		for(auto& par : state.crisis_participants) {
			if(!par.id) {
				break; // not in crisis
			}
			if(!par.merely_interested) {
				if(state.local_player_nation == state.primary_crisis_attacker && !par.supports_attacker)
					row_contents.push_back(par.id);
				else if(state.local_player_nation == state.primary_crisis_defender && par.supports_attacker)
					row_contents.push_back(par.id);
			}
		}

		update(state);
	}
};

class wargoal_target_country_select_window : public window_element_base {
public:
	std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
		if(name == "wargoal_list") {
			return make_element_by_type<wargoal_target_country_listbox>(state, id);
		} else {
			return nullptr;
		}
	}
};

class wargoal_offer_country_listbox : public listbox_element_base<wargoal_country_item, dcon::national_identity_id> {
protected:
	std::string_view get_row_element_name() override {
		return "wargoal_country_item";
	}

public:
	void on_update(sys::state& state) noexcept override {
		row_contents.clear();

		dcon::nation_id target = retrieve<get_target>(state, parent).n;
		auto actor = retrieve<get_offer_to>(state, parent).n;
		dcon::cb_type_id cb = retrieve<dcon::cb_type_id>(state, parent);

		dcon::trigger_key allowed_countries = state.world.cb_type_get_allowed_countries(cb);
		if(!allowed_countries) {
			update(state);
			return;
		}

		for(auto n : state.world.in_nation) {
			if(n != actor && trigger::evaluate(state, allowed_countries, trigger::to_generic(target), trigger::to_generic(actor), trigger::to_generic(n.id))) {
				row_contents.push_back(state.world.nation_get_identity_from_identity_holder(n));
			}
		}
		update(state);
	}
};

class wargoal_offer_country_select_window : public window_element_base {
public:
	std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
		if(name == "country_list") {
			return make_element_by_type<wargoal_offer_country_listbox>(state, id);
		} else if(name == "cancel_select") {
			return make_element_by_type<wargoal_cancel_country_select>(state, id);
		} else {
			return nullptr;
		}
	}
};

class offer_war_goal_dialog : public window_element_base {
private:
	wargoal_offer_setup_window* wargoal_setup_win = nullptr;
	wargoal_offer_country_select_window* wargoal_country_win = nullptr;
	wargoal_target_country_select_window* wargoal_target_win = nullptr;

	dcon::nation_id offer_made_to;
	dcon::nation_id wargoal_against;
	dcon::cb_type_id cb_to_use;
	dcon::state_definition_id target_state;
	dcon::national_identity_id target_country;
	bool will_call_allies = false;
	bool wargoal_decided_upon = false;

	void select_mode(sys::state& state) {
		sys::state_selection_data seldata;
		seldata.single_state_select = true;
		// Populate selectable states...
		dcon::nation_id target = wargoal_against;
		auto actor = state.local_player_nation;
		dcon::cb_type_id cb = cb_to_use;
		auto secondary_tag = target_country;
		auto allowed_substate_regions = state.world.cb_type_get_allowed_substate_regions(cb);
		if(allowed_substate_regions) {
			auto in_nation = state.world.national_identity_get_nation_from_identity_holder(secondary_tag);
			for(auto v : state.world.nation_get_overlord_as_ruler(target)) {
				if(v.get_subject().get_is_substate()) {
					for(auto si : state.world.nation_get_state_ownership(target)) {
						if(trigger::evaluate(state, allowed_substate_regions, trigger::to_generic(si.get_state().id), trigger::to_generic(actor), trigger::to_generic(actor)) &&
							command::can_invite_to_crisis(state, actor, offer_made_to, wargoal_against, cb_to_use, si.get_state().get_definition(), target_country, in_nation)) {
							seldata.selectable_states.push_back(si.get_state().get_definition().id);
						}
					}
				}
			}
		} else {
			auto allowed_states = state.world.cb_type_get_allowed_states(cb);
			if(auto allowed_countries = state.world.cb_type_get_allowed_countries(cb); allowed_countries) {
				auto in_nation = state.world.national_identity_get_nation_from_identity_holder(secondary_tag);
				for(auto si : state.world.nation_get_state_ownership(target)) {
					if(trigger::evaluate(state, allowed_states, trigger::to_generic(si.get_state().id), trigger::to_generic(actor), trigger::to_generic(in_nation)) &&
						command::can_invite_to_crisis(state, actor, offer_made_to, wargoal_against, cb_to_use, si.get_state().get_definition(), target_country, in_nation)) {
						seldata.selectable_states.push_back(si.get_state().get_definition().id);
					}
				}
			} else {
				auto in_nation = state.world.national_identity_get_nation_from_identity_holder(secondary_tag);
				for(auto si : state.world.nation_get_state_ownership(target)) {
					if(trigger::evaluate(state, allowed_states, trigger::to_generic(si.get_state().id), trigger::to_generic(actor), trigger::to_generic(actor)) &&
						command::can_invite_to_crisis(state, actor, offer_made_to, wargoal_against, cb_to_use, si.get_state().get_definition(), target_country, in_nation)) {
						seldata.selectable_states.push_back(si.get_state().get_definition().id);
					}
				}
			}
		}
		seldata.on_select = [&](sys::state& state, dcon::state_definition_id sdef) {
			target_state = sdef;
			wargoal_decided_upon = true;
			wargoal_setup_win->set_visible(state, true);
			wargoal_country_win->set_visible(state, false);
			impl_on_update(state);
			};
		seldata.on_cancel = [&](sys::state& state) {
			target_state = dcon::state_definition_id{};
			if(military::cb_requires_selection_of_a_valid_nation(state, cb_to_use) || military::cb_requires_selection_of_a_liberatable_tag(state, cb_to_use)) {
				wargoal_setup_win->set_visible(state, false);
				//wargoal_state_win->set_visible(state, false);
				wargoal_country_win->set_visible(state, true);
			} else {
				cb_to_use = dcon::cb_type_id{};
				wargoal_decided_upon = false;
				wargoal_setup_win->set_visible(state, true);
				//wargoal_state_win->set_visible(state, false);
				wargoal_country_win->set_visible(state, false);
			}
			impl_on_update(state);
		};
		state.start_state_selection(seldata);
	}
public:
	void reset_window(sys::state& state, dcon::nation_id offer_to) {
		wargoal_against = dcon::nation_id{};
		offer_made_to = offer_to;
		cb_to_use = dcon::cb_type_id{};
		target_state = dcon::state_definition_id{};
		target_country = dcon::national_identity_id{};
		will_call_allies = false;
		wargoal_decided_upon = false;

		wargoal_setup_win->set_visible(state, false);
		//wargoal_state_win->set_visible(state, false);
		wargoal_country_win->set_visible(state, false);
		wargoal_target_win->set_visible(state, true);
	}

	std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
		if(name == "background") {
			auto ptr = make_element_by_type<draggable_target>(state, id);
			ptr->base_data.size = base_data.size; // Nudge size for proper sizing
			return ptr;
		} else if(name == "leftshield") {
			return make_element_by_type<nation_player_flag>(state, id);
		} else if(name == "rightshield") {
			return make_element_by_type<flag_button>(state, id);
		} else if(name == "title") {
			return make_element_by_type<wargoal_offer_title>(state, id);
		} else if(name == "description") {
			return make_element_by_type<wargoal_offer_description1>(state, id);
		} else if(name == "wargoal_add_effect") {
			return make_element_by_type<wargoal_offer_add_window>(state, id);
		} else if(name == "wargoal_success_effect" || name == "wargoal_failure_effect") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "description2") {
			return make_element_by_type<wargoal_offer_description2>(state, id);
		} else if(name == "acceptance") {
			return make_element_by_type<simple_text_element_base>(state, id);
		} else if(name == "call_allies_checkbox") {
			return make_element_by_type<invisible_element>(state, id);
		} if(name == "call_allies_checkbox") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "run_conference_checkbox") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "agreebutton") {
			return make_element_by_type<wargoal_offer_agree_button>(state, id);
		} else if(name == "declinebutton") {
			return make_element_by_type<cancel_pick_wg_button>(state, id);
		} else if(name == "wargoal_setup") {
			auto ptr = make_element_by_type<wargoal_target_country_select_window>(state, id);
			wargoal_target_win = ptr.get();
			ptr->set_visible(state, true);
			return ptr;
		} else if(name == "wargoal_state_select") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "wargoal_country_select") {
			{
				auto ptr = make_element_by_type<wargoal_offer_setup_window>(state, id);
				wargoal_setup_win = ptr.get();
				ptr->set_visible(state, false);
				add_child_to_front(std::move(ptr));
			}
			{
				auto ptr = make_element_by_type<wargoal_offer_country_select_window>(state, id);
				wargoal_country_win = ptr.get();
				ptr->set_visible(state, false);
				return ptr;
			}
		} else {
			return nullptr;
		}
	}

	message_result get(sys::state& state, Cyto::Any& payload) noexcept override {
		if(payload.holds_type<dcon::nation_id>()) {
			payload.emplace<dcon::nation_id>(offer_made_to);
		} else if(payload.holds_type<get_target>()) {
			payload.emplace<get_target>(get_target{ wargoal_against });
		} else if(payload.holds_type<get_offer_to>()) {
			payload.emplace<get_offer_to>(get_offer_to{ offer_made_to });
		} else if(payload.holds_type< check_wg_completion>()) {
			payload.emplace<check_wg_completion>(check_wg_completion{ wargoal_decided_upon });
			return message_result::consumed;
		} else if(payload.holds_type<element_selection_wrapper<dcon::cb_type_id>>()) {
			cb_to_use = any_cast<element_selection_wrapper<dcon::cb_type_id>>(payload).data;
			if(!cb_to_use) {
				wargoal_against = dcon::nation_id{};
				cb_to_use = dcon::cb_type_id{};
				target_state = dcon::state_definition_id{};
				target_country = dcon::national_identity_id{};
				wargoal_decided_upon = false;

				wargoal_setup_win->set_visible(state, false);
				//wargoal_state_win->set_visible(state, false);
				wargoal_country_win->set_visible(state, false);
				wargoal_target_win->set_visible(state, true);
			} else if(military::cb_requires_selection_of_a_valid_nation(state, cb_to_use) || military::cb_requires_selection_of_a_liberatable_tag(state, cb_to_use)) {
				wargoal_setup_win->set_visible(state, false);
				//wargoal_state_win->set_visible(state, false);
				wargoal_country_win->set_visible(state, true);
				wargoal_target_win->set_visible(state, false);
			} else if(military::cb_requires_selection_of_a_state(state, cb_to_use)) {
				wargoal_setup_win->set_visible(state, false);
				//wargoal_state_win->set_visible(state, true);
				wargoal_country_win->set_visible(state, false);
				wargoal_target_win->set_visible(state, false);
				select_mode(state);
			} else {
				wargoal_decided_upon = true;
				wargoal_setup_win->set_visible(state, true);
				//wargoal_state_win->set_visible(state, false);
				wargoal_country_win->set_visible(state, false);
				wargoal_target_win->set_visible(state, false);
			}
			impl_on_update(state);
			return message_result::consumed;
		} else if(payload.holds_type<element_selection_wrapper<dcon::national_identity_id>>()) {
			target_country = any_cast<element_selection_wrapper<dcon::national_identity_id>>(payload).data;

			if(target_country) { // goto next step
				if(military::cb_requires_selection_of_a_state(state, cb_to_use)) {
					wargoal_setup_win->set_visible(state, true);
					//wargoal_state_win->set_visible(state, true);
					wargoal_country_win->set_visible(state, false);
					wargoal_target_win->set_visible(state, true);
					select_mode(state);
				} else {
					wargoal_setup_win->set_visible(state, true);
					//wargoal_state_win->set_visible(state, false);
					wargoal_country_win->set_visible(state, false);
					wargoal_target_win->set_visible(state, false);
					wargoal_decided_upon = true;
				}
			} else {
				wargoal_decided_upon = false;
				cb_to_use = dcon::cb_type_id{};
				wargoal_setup_win->set_visible(state, true);
				//wargoal_state_win->set_visible(state, false);
				wargoal_country_win->set_visible(state, false);
				wargoal_target_win->set_visible(state, false);
			}

			impl_on_update(state);
			return message_result::consumed;
		} else if(payload.holds_type<set_target>()) {
			wargoal_against = any_cast<set_target>(payload).n;
			if(wargoal_against) {
				wargoal_decided_upon = false;
				cb_to_use = dcon::cb_type_id{};
				wargoal_setup_win->set_visible(state, true);
				//wargoal_state_win->set_visible(state, false);
				wargoal_country_win->set_visible(state, false);
				wargoal_target_win->set_visible(state, false);
			}
			return message_result::consumed;
		} else if(payload.holds_type<dcon::cb_type_id>()) {
			payload.emplace<dcon::cb_type_id>(cb_to_use);
			return message_result::consumed;
		} else if(payload.holds_type<dcon::state_definition_id>()) {
			payload.emplace<dcon::state_definition_id>(target_state);
			return message_result::consumed;
		} else if(payload.holds_type<dcon::national_identity_id>()) {
			payload.emplace<dcon::national_identity_id>(target_country);
			return message_result::consumed;
		} else if(payload.holds_type<bool>()) {
			payload.emplace<bool>(will_call_allies);
			return message_result::consumed;
		}
		return message_result::unseen;
	}
};

}
