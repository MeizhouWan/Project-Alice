#include "parsers_declarations.hpp"
#include "system_state.hpp"
#include "rebels.hpp"
#include "fonts.hpp"
#include "demographics.hpp"

namespace parsers {

scenario_building_context::scenario_building_context(sys::state& state) : gfx_context(state, state.ui_defs), state(state) { }

void religion_def::icon(association_type, int32_t v, error_handler& err, int32_t line, religion_context& context) {
	context.outer_context.state.world.religion_set_icon(context.id, uint8_t(v));
}

void religion_def::color(color_from_3f v, error_handler& err, int32_t line, religion_context& context) {
	context.outer_context.state.world.religion_set_color(context.id, v.value);
}

void religion_def::pagan(association_type, bool v, error_handler& err, int32_t line, religion_context& context) {
	context.outer_context.state.world.religion_set_is_pagan(context.id, v);
}

void religion_def::nation_modifier(modifier_base v, error_handler& err, int32_t line, religion_context& context) {
	if(v.next_to_add_n != 0) {
		auto mod = context.outer_context.state.world.create_modifier();
		context.outer_context.state.world.modifier_set_national_values(mod, v.peek_national_mod());
		context.outer_context.state.world.modifier_set_icon(mod, uint8_t(v.icon_index));
		context.outer_context.state.world.modifier_set_name(mod, context.outer_context.state.world.religion_get_name(context.id));
		context.outer_context.state.world.religion_set_nation_modifier(context.id, mod);
	}
}

void names_list::free_value(std::string_view text, error_handler& err, int32_t line, names_context& context) {
	auto new_id = context.outer_context.state.add_unit_name(text);
	if(context.first_names) {
		context.outer_context.state.world.culture_get_first_names(context.id).push_back(new_id);
	} else {
		context.outer_context.state.world.culture_get_last_names(context.id).push_back(new_id);
	}
}

void culture::color(color_from_3i v, error_handler& err, int32_t line, culture_context& context) {
	context.outer_context.state.world.culture_set_color(context.id, v.value);
}

void culture::radicalism(association_type, int32_t v, error_handler& err, int32_t line, culture_context& context) {
	context.outer_context.state.world.culture_set_radicalism(context.id, int8_t(v));
}

void culture_group::leader(association_type, std::string_view name, error_handler& err, int32_t line,
		culture_group_context& context) {

	if(auto it = context.outer_context.map_of_leader_graphics.find(std::string(name)); it != context.outer_context.map_of_leader_graphics.end()) {
		context.outer_context.state.world.culture_group_set_leader(context.id, it->second);
	} else {
		err.accumulated_errors +=
				"Unknown leader type " + std::string(name) + " in file " + err.file_name + " line " + std::to_string(line) + "\n";
	}
}

void culture_group::is_overseas(association_type, bool v, error_handler& err, int32_t line, culture_group_context& context) {
	context.outer_context.state.world.culture_group_set_is_overseas(context.id, v);
}

void culture_group::union_tag(association_type, uint32_t v, error_handler& err, int32_t line, culture_group_context& context) {
	auto nat_tag = context.outer_context.map_of_ident_names.find(v);
	if(nat_tag != context.outer_context.map_of_ident_names.end())
		context.outer_context.state.world.force_create_cultural_union_of(nat_tag->second, context.id);
	else
		err.accumulated_errors += "Unknown national tag in file " + err.file_name + " line " + std::to_string(line) + "\n";
}

void good::money(association_type, bool v, error_handler& err, int32_t line, good_context& context) {
	if(v) {
		if(context.outer_context.money_set) {
			context.outer_context.state.world.commodity_set_money_rgo(context.id, true);
		} else {
			context.outer_context.state.world.commodity_set_color(economy::money,
					context.outer_context.state.world.commodity_get_color(context.id));
			context.outer_context.state.world.commodity_set_cost(economy::money,
					context.outer_context.state.world.commodity_get_cost(context.id));
			context.outer_context.state.world.commodity_set_commodity_group(economy::money,
					context.outer_context.state.world.commodity_get_commodity_group(context.id));
			context.outer_context.state.world.commodity_set_name(economy::money,
					context.outer_context.state.world.commodity_get_name(context.id));
			context.outer_context.state.world.commodity_set_is_available_from_start(economy::money,
					context.outer_context.state.world.commodity_get_is_available_from_start(context.id));
			context.outer_context.state.world.commodity_set_money_rgo(economy::money, true);

			for(auto& pr : context.outer_context.map_of_commodity_names) {
				if(pr.second == context.id) {
					pr.second = economy::money;
					break;
				}
			}
			context.id = economy::money;
			context.outer_context.state.world.pop_back_commodity();
			context.outer_context.money_set = true;
		}
	}
}

void good::color(color_from_3i v, error_handler& err, int32_t line, good_context& context) {
	context.outer_context.state.world.commodity_set_color(context.id, v.value);
}

void good::cost(association_type, float v, error_handler& err, int32_t line, good_context& context) {
	context.outer_context.state.world.commodity_set_cost(context.id, v);
}

void good::available_from_start(association_type, bool b, error_handler& err, int32_t line, good_context& context) {
	context.outer_context.state.world.commodity_set_is_available_from_start(context.id, b);
}

void good::is_local(association_type, bool b, error_handler& err, int32_t line, good_context& context) {
	context.outer_context.state.world.commodity_set_is_local(context.id, b);
}

void good::overseas_penalty(association_type, bool b, error_handler& err, int32_t line, good_context& context) {
	context.outer_context.state.world.commodity_set_overseas_penalty(context.id, b);
}

void good::uses_potentials(association_type, bool b, error_handler& err, int32_t line, good_context& context) {
	context.outer_context.state.world.commodity_set_uses_potentials(context.id, b);
}

void good::finish(good_context& context) {
	++context.outer_context.number_of_commodities_seen;
	context.outer_context.state.world.commodity_set_icon(context.id, uint8_t(context.outer_context.number_of_commodities_seen));
}

void issue::next_step_only(association_type, bool value, error_handler& err, int32_t line, issue_context& context) {
	context.outer_context.state.world.issue_set_is_next_step_only(context.id, value);
}

void issue::administrative(association_type, bool value, error_handler& err, int32_t line, issue_context& context) {
	context.outer_context.state.world.issue_set_is_administrative(context.id, value);
}

void issue::next_step_only(association_type, bool value, error_handler& err, int32_t line, reform_context& context) {
	context.outer_context.state.world.reform_set_is_next_step_only(context.id, value);
}

void issue::administrative(association_type, bool value, error_handler& err, int32_t line, reform_context& context) {
	err.accumulated_errors +=
			"Error, only issues can be administrative (" + err.file_name + " line " + std::to_string(line) + ")\n";
}

void government_type::election(association_type, bool value, error_handler& err, int32_t line, government_type_context& context) {
	context.outer_context.state.world.government_type_set_has_elections(context.id, value);
}

void government_type::appoint_ruling_party(association_type, bool value, error_handler& err, int32_t line,
		government_type_context& context) {
	context.outer_context.state.world.government_type_set_can_appoint_ruling_party(context.id, value);
}

void government_type::duration(association_type, int32_t value, error_handler& err, int32_t line,
		government_type_context& context) {
	context.outer_context.state.world.government_type_set_duration(context.id, int8_t(value));
}

void government_type::flagtype(association_type, std::string_view value, error_handler& err, int32_t line, government_type_context& context) {

	auto key = context.outer_context.state.lookup_key(value);
	if(!key) {
		key = context.outer_context.state.add_key_utf8(value);
	}

	dcon::government_flag_id found_flag{ };
	context.outer_context.state.world.for_each_government_flag([&](auto id) {
		if(context.outer_context.state.world.government_flag_get_filename(id) == key) {
			found_flag = id;
		}
	});

	if(!found_flag) {
		found_flag = context.outer_context.state.world.create_government_flag();
		context.outer_context.state.world.government_flag_set_filename(found_flag, key);
	}

	context.outer_context.state.world.government_type_set_flag(context.id, found_flag);
}

void government_type::any_value(std::string_view text, association_type, bool value, error_handler& err, int32_t line,
		government_type_context& context) {
	if(value) {
		auto found_ideology = context.outer_context.map_of_ideologies.find(std::string(text));
		if(found_ideology != context.outer_context.map_of_ideologies.end()) {
			context.outer_context.state.world.government_type_get_ideologies_allowed(context.id) |=
					::culture::to_bits(found_ideology->second.id);
		} else {
			err.accumulated_errors +=
					"Unknown ideology " + std::string(text) + " in file " + err.file_name + " line " + std::to_string(line) + "\n";
		}
	}
}

void cb_list::free_value(std::string_view text, error_handler& err, int32_t line, scenario_building_context& context) {
	//dcon::cb_type_id new_id = context.state.world.create_cb_type();
	//context.map_of_cb_types.insert_or_assign(std::string(text), pending_cb_content{token_generator{}, new_id});
}

void trait::organisation(association_type, float value, error_handler& err, int32_t line, trait_context& context) {
	context.outer_context.state.world.leader_trait_set_organisation(context.id, value);
}

void trait::morale(association_type, float value, error_handler& err, int32_t line, trait_context& context) {
	context.outer_context.state.world.leader_trait_set_morale(context.id, value);
}

void trait::attack(association_type, float value, error_handler& err, int32_t line, trait_context& context) {
	context.outer_context.state.world.leader_trait_set_attack(context.id, value);
}

void trait::defence(association_type, float value, error_handler& err, int32_t line, trait_context& context) {
	context.outer_context.state.world.leader_trait_set_defense(context.id, value);
}

void trait::reconnaissance(association_type, float value, error_handler& err, int32_t line, trait_context& context) {
	context.outer_context.state.world.leader_trait_set_reconnaissance(context.id, value);
}

void trait::speed(association_type, float value, error_handler& err, int32_t line, trait_context& context) {
	context.outer_context.state.world.leader_trait_set_speed(context.id, value);
}

void trait::experience(association_type, float value, error_handler& err, int32_t line, trait_context& context) {
	context.outer_context.state.world.leader_trait_set_experience(context.id, value);
}

void trait::reliability(association_type, float value, error_handler& err, int32_t line, trait_context& context) {
	context.outer_context.state.world.leader_trait_set_reliability(context.id, value);
}

void sea_list::free_value(int32_t value, error_handler& err, int32_t line, scenario_building_context& context) {
	if(size_t(value - 1) > context.prov_id_to_original_id_map.size()) {
		err.accumulated_errors +=
				"Province id " + std::to_string(value) + " is too large (" + err.file_name + " line " + std::to_string(line) + ")\n";
	} else {
		context.prov_id_to_original_id_map[dcon::province_id(dcon::province_id::value_base_t(value - 1))].is_sea = true;
	}
}

void state_definition::free_value(int32_t value, error_handler& err, int32_t line, state_def_building_context& context) {
	if(size_t(value) >= context.outer_context.original_id_to_prov_id_map.size()) {
		err.accumulated_errors += "Province id " + std::to_string(value) + " is too large (" + err.file_name + " line " + std::to_string(line) + ")\n";
	} else {
		auto prov = context.outer_context.original_id_to_prov_id_map[value];
		if(prov) {
			context.provinces.push_back(prov);
		} else {
			err.accumulated_warnings += "Province id " + std::to_string(value) + " was not found (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	}
}

void continent_provinces::free_value(int32_t value, error_handler& err, int32_t line, continent_building_context& context) {
	if(size_t(value) >= context.outer_context.original_id_to_prov_id_map.size()) {
		err.accumulated_errors += "Province id " + std::to_string(value) + " is too large (" + err.file_name + " line " + std::to_string(line) + ")\n";
	} else {
		auto province_id = context.outer_context.original_id_to_prov_id_map[value];
		if(province_id) {
			if(context.outer_context.state.world.province_get_continent(province_id)) {
				err.accumulated_warnings += "Province " + std::to_string(context.outer_context.prov_id_to_original_id_map.safe_get(province_id).id) + " (" + std::to_string(value) + ")" + " assigned to multiple continents (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
			context.outer_context.state.world.province_set_continent(province_id, context.id);
		}
	}
}

void continent_definition::free_value(int32_t value, error_handler& err, int32_t line, continent_building_context& context) {
	if(size_t(value) >= context.outer_context.original_id_to_prov_id_map.size()) {
		err.accumulated_errors += "Province id " + std::to_string(value) + " is too large (" + err.file_name + " line " + std::to_string(line) + ")\n";
	} else {
		auto province_id = context.outer_context.original_id_to_prov_id_map[value];
		if(province_id) {
			if(context.outer_context.state.world.province_get_continent(province_id)) {
				err.accumulated_warnings += "Province " + std::to_string(context.outer_context.prov_id_to_original_id_map.safe_get(province_id).id) + " (" + std::to_string(value) + ")" + " assigned to multiple continents (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
			context.outer_context.state.world.province_set_continent(province_id, context.id);
		}
	}
}

void climate_definition::free_value(int32_t value, error_handler& err, int32_t line, climate_building_context& context) {
	if(size_t(value) >= context.outer_context.original_id_to_prov_id_map.size()) {
		err.accumulated_errors +=
				"Province id " + std::to_string(value) + " is too large (" + err.file_name + " line " + std::to_string(line) + ")\n";
	} else {
		auto province_id = context.outer_context.original_id_to_prov_id_map[value];
		if(province_id) {
			if(context.outer_context.state.world.province_get_climate(province_id)) {
				err.accumulated_warnings += "Province " + std::to_string(context.outer_context.prov_id_to_original_id_map.safe_get(province_id).id) + " (" + std::to_string(value) + ")" + " assigned to multiple climates (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
			context.outer_context.state.world.province_set_climate(province_id, context.id);
		}
	}
}

void tech_folder_list::free_value(std::string_view name, error_handler& err, int32_t line, tech_group_context& context) {
	auto name_id = text::find_or_add_key(context.outer_context.state, name, false);
	auto cindex = context.outer_context.state.culture_definitions.tech_folders.size();
	context.outer_context.state.culture_definitions.tech_folders.push_back(::culture::folder_info{name_id, context.category});
	context.outer_context.map_of_tech_folders.insert_or_assign(std::string(name), int32_t(cindex));
}

void commodity_set::any_value(std::string_view name, association_type, float value, error_handler& err, int32_t line,
		scenario_building_context& context) {
	auto found_commodity = context.map_of_commodity_names.find(std::string(name));
	if(found_commodity != context.map_of_commodity_names.end()) {
		if(num_added < int32_t(economy::commodity_set::set_size)) {
			commodity_amounts[num_added] = value;
			commodity_type[num_added] = found_commodity->second;
			++num_added;
		} else {
			err.accumulated_errors += "Too many items in a commodity set, in file " + err.file_name + " line " + std::to_string(line) + "\n";
		}
	} else {
		err.accumulated_errors += "Unknown commodity " + std::string(name) + " in file " + err.file_name + " line " + std::to_string(line) + "\n";
	}
}

void unit_definition::finish(scenario_building_context&) {
	// minimum discipline for land units
	if(is_land) {
		if(discipline_or_evasion <= 0.0f)
			discipline_or_evasion = 1.0f;
	}
}

void party::ideology(association_type, std::string_view text, error_handler& err, int32_t line, party_context& context) {
	if(auto it = context.outer_context.map_of_ideologies.find(std::string(text));
			it != context.outer_context.map_of_ideologies.end()) {
		context.outer_context.state.world.political_party_set_ideology(context.id, it->second.id);
	} else {
		err.accumulated_errors += std::string(text) + " is not a valid ideology (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void party::name(association_type, std::string_view text, error_handler& err, int32_t line, party_context& context) {
	auto name_id = text::find_or_add_key(context.outer_context.state, text, false);
	context.outer_context.state.world.political_party_set_name(context.id, name_id);
}

void party::start_date(association_type, sys::year_month_day ymd, error_handler& err, int32_t line, party_context& context) {
	auto date_tag = sys::date(ymd, context.outer_context.state.start_date);
	context.outer_context.state.world.political_party_set_start_date(context.id, date_tag);
}

void party::end_date(association_type, sys::year_month_day ymd, error_handler& err, int32_t line, party_context& context) {
	auto date_tag = sys::date(ymd, context.outer_context.state.start_date);
	context.outer_context.state.world.political_party_set_end_date(context.id, date_tag);
}

void party::finish(party_context& context) {
	context.outer_context.state.world.political_party_set_trigger(context.id, trigger);
}

void party::any_value(std::string_view issue, association_type, std::string_view option, error_handler& err, int32_t line,
		party_context& context) {
	if(auto it = context.outer_context.map_of_iissues.find(std::string(issue)); it != context.outer_context.map_of_iissues.end()) {
		if(auto oit = context.outer_context.map_of_ioptions.find(std::string(option)); oit != context.outer_context.map_of_ioptions.end()) {
			context.outer_context.state.world.political_party_set_party_issues(context.id, it->second, oit->second.id);
		} else {
			err.accumulated_errors +=
						std::string(option) + " is not a valid option name (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
		// context.outer_context.state.world.political_party_set_ideology(context.id, it->second.id);
	} else {
		err.accumulated_errors += std::string(issue) + " is not a valid issue name (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void unit_names_list::free_value(std::string_view text, error_handler& err, int32_t line, unit_names_context& context) {
	if(text.length() <= 2)
		return;

	auto existing_count = context.outer_context.state.world.national_identity_get_unit_names_count(context.id, context.u_id);
	if(existing_count == 0) {
		auto first_id = context.outer_context.state.add_unit_name(text);
		context.outer_context.state.world.national_identity_set_unit_names_first(context.id, context.u_id, first_id);
		context.outer_context.state.world.national_identity_set_unit_names_count(context.id, context.u_id, uint8_t(1));
	} else {
		context.outer_context.state.add_unit_name(text);
		context.outer_context.state.world.national_identity_set_unit_names_count(context.id, context.u_id,
				uint8_t(existing_count + 1));
	}
}

void pop_history_definition::culture(association_type, std::string_view value, error_handler& err, int32_t line,
		pop_history_province_context& context) {
	if(auto it = context.outer_context.map_of_culture_names.find(std::string(value));
			it != context.outer_context.map_of_culture_names.end()) {
		cul_id = it->second;
	} else {
		err.accumulated_errors +=
				"Invalid culture " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void pop_history_definition::religion(association_type, std::string_view value, error_handler& err, int32_t line,
		pop_history_province_context& context) {
	if(auto it = context.outer_context.map_of_religion_names.find(std::string(value));
			it != context.outer_context.map_of_religion_names.end()) {
		rel_id = it->second;
	} else {
		err.accumulated_errors +=
				"Invalid religion " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void pop_history_definition::rebel_type(association_type, std::string_view value, error_handler& err, int32_t line,
		pop_history_province_context& context) {
	if(auto it = context.outer_context.map_of_rebeltypes.find(std::string(value));
			it != context.outer_context.map_of_rebeltypes.end()) {
		reb_id = it->second.id;
	} else {
		err.accumulated_errors +=
				"Invalid rebel type " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void pop_province_list::any_group(std::string_view type, pop_history_definition const& def, error_handler& err, int32_t line,
		pop_history_province_context& context) {
	dcon::pop_type_id ptype;
	if(auto it = context.outer_context.map_of_poptypes.find(std::string(type)); it != context.outer_context.map_of_poptypes.end()) {
		ptype = it->second;
	} else {
		err.accumulated_errors +=
				"Invalid pop type " + std::string(type) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
	for(auto pops_by_location : context.outer_context.state.world.province_get_pop_location(context.id)) {
		auto pop_id = pops_by_location.get_pop();
		if(pop_id.get_culture() == def.cul_id && pop_id.get_poptype() == ptype && pop_id.get_religion() == def.rel_id) {
			pop_id.get_size() += float(def.size);
			return; // done with this pop
		}
	}
	// no existing pop matched -- make a new pop
	auto new_pop = fatten(context.outer_context.state.world, context.outer_context.state.world.create_pop());
	new_pop.set_culture(def.cul_id);
	new_pop.set_religion(def.rel_id);
	new_pop.set_size(float(def.size));
	new_pop.set_poptype(ptype);
	// set the default literacy as 10% if it is not overriden later by "literacy" or "on_state_culture_literacy" in the country history file.
	pop_demographics::set_literacy(context.outer_context.state, new_pop, 0.1f);
	pop_demographics::set_militancy(context.outer_context.state, new_pop, def.militancy);
	// new_pop.set_rebel_group(def.reb_id);

	auto pop_owner = context.outer_context.state.world.province_get_nation_from_province_ownership(context.id);
	if(def.reb_id) {
		if(pop_owner) {
			auto existing_faction = rebel::get_faction_by_type(context.outer_context.state, pop_owner, def.reb_id);
			if(existing_faction) {
				context.outer_context.state.world.try_create_pop_rebellion_membership(new_pop, existing_faction);
			} else {
				auto new_faction = fatten(context.outer_context.state.world, context.outer_context.state.world.create_rebel_faction());
				new_faction.set_type(def.reb_id);
				context.outer_context.state.world.try_create_rebellion_within(new_faction, pop_owner);
				context.outer_context.state.world.try_create_pop_rebellion_membership(new_pop, new_faction);
			}
		} else {
			err.accumulated_warnings += "Rebel specified on a province without owner (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	}

	context.outer_context.state.world.force_create_pop_location(new_pop, context.id);
}

void poptype_file::sprite(association_type, int32_t value, error_handler& err, int32_t line, poptype_context& context) {
	context.outer_context.state.world.pop_type_set_sprite(context.id, uint8_t(value));
}

void poptype_file::color(color_from_3i cvalue, error_handler& err, int32_t line, poptype_context& context) {
	context.outer_context.state.world.pop_type_set_color(context.id, cvalue.value);
}

void poptype_file::is_artisan(association_type, bool value, error_handler& err, int32_t line, poptype_context& context) {
	if(value)
		context.outer_context.state.culture_definitions.artisans = context.id;
}

void poptype_file::strata(association_type, std::string_view value, error_handler& err, int32_t line, poptype_context& context) {
	if(is_fixed_token_ci(value.data(), value.data() + value.length(), "rich"))
		context.outer_context.state.world.pop_type_set_strata(context.id, uint8_t(::culture::pop_strata::rich));
	else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "middle"))
		context.outer_context.state.world.pop_type_set_strata(context.id, uint8_t(::culture::pop_strata::middle));
	else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "poor"))
		context.outer_context.state.world.pop_type_set_strata(context.id, uint8_t(::culture::pop_strata::poor));
	else {
		err.accumulated_errors +=
				"Invalid pop strata " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void poptype_file::unemployment(association_type, bool value, error_handler& err, int32_t line, poptype_context& context) {
	context.outer_context.state.world.pop_type_set_has_unemployment(context.id, value);
}

void poptype_file::is_slave(association_type, bool value, error_handler& err, int32_t line, poptype_context& context) {
	if(value)
		context.outer_context.state.culture_definitions.slaves = context.id;
}

void poptype_file::allowed_to_vote(association_type, bool value, error_handler& err, int32_t line, poptype_context& context) {
	if(value == false) {
		context.outer_context.state.world.pop_type_set_voting_forbidden(context.id, true);
	}
}

void poptype_file::can_be_recruited(association_type, bool value, error_handler& err, int32_t line, poptype_context& context) {
	if(value)
		context.outer_context.state.culture_definitions.soldiers = context.id;
}

void poptype_file::state_capital_only(association_type, bool value, error_handler& err, int32_t line, poptype_context& context) {
	context.outer_context.state.world.pop_type_set_state_capital_only(context.id, value);
}

void poptype_file::leadership(association_type, int32_t value, error_handler& err, int32_t line, poptype_context& context) {
	context.outer_context.state.culture_definitions.officer_leadership_points = value;
	context.outer_context.state.culture_definitions.officers = context.id;
}

void poptype_file::research_optimum(association_type, float value, error_handler& err, int32_t line, poptype_context& context) {
	context.outer_context.state.world.pop_type_set_research_optimum(context.id, value);
}

void poptype_file::administrative_efficiency(association_type, bool value, error_handler& err, int32_t line,
		poptype_context& context) {
	if(value)
		context.outer_context.state.culture_definitions.bureaucrat = context.id;
}

void poptype_file::tax_eff(association_type, float value, error_handler& err, int32_t line, poptype_context& context) {
	context.outer_context.state.culture_definitions.bureaucrat_tax_efficiency = value;
}

void poptype_file::can_build(association_type, bool value, error_handler& err, int32_t line, poptype_context& context) {
	if(value)
		context.outer_context.state.culture_definitions.capitalists = context.id;
}

void poptype_file::research_points(association_type, float value, error_handler& err, int32_t line, poptype_context& context) {
	context.outer_context.state.world.pop_type_set_research_points(context.id, value);
}

void poptype_file::can_reduce_consciousness(association_type, bool value, error_handler& err, int32_t line,
		poptype_context& context) {
	if(value)
		context.outer_context.state.culture_definitions.clergy = context.id;
}

void poptype_file::workplace_input(association_type, float value, error_handler& err, int32_t line, poptype_context& context) {
	// discard
}

void poptype_file::workplace_output(association_type, float value, error_handler& err, int32_t line, poptype_context& context) {
	// discard
}

void poptype_file::equivalent(association_type, std::string_view value, error_handler& err, int32_t line,
		poptype_context& context) {
	if(value.length() > 0 && value[0] == 'f') {
		context.outer_context.state.culture_definitions.laborers = context.id;
	} else if(value.length() > 0 && value[0] == 'l') {
		context.outer_context.state.culture_definitions.farmers = context.id;
	}
}

void poptype_file::life_needs(commodity_array const& value, error_handler& err, int32_t line, poptype_context& context) {
	context.outer_context.state.world.for_each_commodity([&](dcon::commodity_id cid) {
		if(cid.index() < value.data.ssize())
			context.outer_context.state.world.pop_type_set_life_needs(context.id, cid, value.data[cid]);
	});
}

void poptype_file::everyday_needs(commodity_array const& value, error_handler& err, int32_t line, poptype_context& context) {
	context.outer_context.state.world.for_each_commodity([&](dcon::commodity_id cid) {
		if(cid.index() < value.data.ssize())
			context.outer_context.state.world.pop_type_set_everyday_needs(context.id, cid, value.data[cid]);
	});
}

void poptype_file::luxury_needs(commodity_array const& value, error_handler& err, int32_t line, poptype_context& context) {
	context.outer_context.state.world.for_each_commodity([&](dcon::commodity_id cid) {
		if(cid.index() < value.data.ssize())
			context.outer_context.state.world.pop_type_set_luxury_needs(context.id, cid, value.data[cid]);
	});
}

void poptype_file::life_needs_income(income const& value, error_handler& err, int32_t line, poptype_context& context) {
	// context.outer_context.state.world.pop_type_set_life_needs_income_weight(context.id, value.weight);
	context.outer_context.state.world.pop_type_set_life_needs_income_type(context.id, uint8_t(value.itype));
}

void poptype_file::everyday_needs_income(income const& value, error_handler& err, int32_t line, poptype_context& context) {
	// context.outer_context.state.world.pop_type_set_everyday_needs_income_weight(context.id, value.weight);
	context.outer_context.state.world.pop_type_set_everyday_needs_income_type(context.id, uint8_t(value.itype));
}

void poptype_file::luxury_needs_income(income const& value, error_handler& err, int32_t line, poptype_context& context) {
	// context.outer_context.state.world.pop_type_set_luxury_needs_income_weight(context.id, value.weight);
	context.outer_context.state.world.pop_type_set_luxury_needs_income_type(context.id, uint8_t(value.itype));
}

void individual_ideology::finish(individual_ideology_context& context) {
	if(!bool(context.outer_context.state.world.ideology_get_activation_date(context.id))) {
		context.outer_context.state.world.ideology_set_enabled(context.id, true);
	}
}

void individual_ideology::can_reduce_militancy(association_type, bool value, error_handler& err, int32_t line,
		individual_ideology_context& context) {
	if(value)
		context.outer_context.state.culture_definitions.conservative = context.id;
}

void individual_ideology::uncivilized(association_type, bool value, error_handler& err, int32_t line, individual_ideology_context& context) {
	context.outer_context.state.world.ideology_set_is_civilized_only(context.id, !value);
}

void individual_ideology::civilized(association_type, bool value, error_handler& err, int32_t line, individual_ideology_context& context) {
	context.outer_context.state.world.ideology_set_is_civilized_only(context.id, value);
}

void individual_ideology::color(color_from_3i cvalue, error_handler& err, int32_t line, individual_ideology_context& context) {
	context.outer_context.state.world.ideology_set_color(context.id, cvalue.value);
}

void individual_ideology::date(association_type, sys::year_month_day ymd, error_handler& err, int32_t line,
		individual_ideology_context& context) {
	auto date_tag = sys::date(ymd, context.outer_context.state.start_date);
	context.outer_context.state.world.ideology_set_activation_date(context.id, date_tag);
}

void individual_ideology::add_political_reform(dcon::value_modifier_key value, error_handler& err, int32_t line,
		individual_ideology_context& context) {
	context.outer_context.state.world.ideology_set_add_political_reform(context.id, value);
}

void individual_ideology::remove_political_reform(dcon::value_modifier_key value, error_handler& err, int32_t line,
		individual_ideology_context& context) {
	context.outer_context.state.world.ideology_set_remove_political_reform(context.id, value);
}

void individual_ideology::add_social_reform(dcon::value_modifier_key value, error_handler& err, int32_t line,
		individual_ideology_context& context) {
	context.outer_context.state.world.ideology_set_add_social_reform(context.id, value);
}

void individual_ideology::remove_social_reform(dcon::value_modifier_key value, error_handler& err, int32_t line,
		individual_ideology_context& context) {
	context.outer_context.state.world.ideology_set_remove_social_reform(context.id, value);
}

void individual_ideology::add_military_reform(dcon::value_modifier_key value, error_handler& err, int32_t line,
		individual_ideology_context& context) {
	context.outer_context.state.world.ideology_set_add_military_reform(context.id, value);
}

void individual_ideology::add_economic_reform(dcon::value_modifier_key value, error_handler& err, int32_t line,
		individual_ideology_context& context) {
	context.outer_context.state.world.ideology_set_add_economic_reform(context.id, value);
}

void cb_body::finish(individual_cb_context& context) {
	auto bits = context.outer_context.state.world.cb_type_get_type_bits(context.id);
	// This type of a wargoal will be used for liberate state crises
	if((bits & military::cb_flag::po_transfer_provinces) != 0 && (bits & military::cb_flag::all_allowed_states) == 0 && (bits & military::cb_flag::not_in_crisis) == 0) {
		context.outer_context.state.military_definitions.crisis_liberate = context.id;
	}
	// This type of a wargoal will be used for annex nation crises (restore order cb for example)
	// This isn't triggered in vanilla. Perhaps a bug.
	else if((bits & military::cb_flag::po_annex) != 0 && (bits & military::cb_flag::all_allowed_states) == 0 && (bits & military::cb_flag::not_in_crisis) == 0) {
		context.outer_context.state.military_definitions.crisis_annex = context.id;
	}
}

void cb_body::is_civil_war(association_type, bool value, error_handler& err, int32_t line, individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::is_civil_war;
		context.outer_context.state.military_definitions.standard_civil_war = context.id;
	}
}

void cb_body::months(association_type, int32_t value, error_handler& err, int32_t line, individual_cb_context& context) {
	context.outer_context.state.world.cb_type_set_months(context.id, uint8_t(value));
}

void cb_body::truce_months(association_type, int32_t value, error_handler& err, int32_t line, individual_cb_context& context) {
	context.outer_context.state.world.cb_type_set_truce_months(context.id, uint8_t(value));
}

void cb_body::sprite_index(association_type, int32_t value, error_handler& err, int32_t line, individual_cb_context& context) {
	context.outer_context.state.world.cb_type_set_sprite_index(context.id, uint8_t(value));
}

void cb_body::always(association_type, bool value, error_handler& err, int32_t line, individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::always;
	}
}

void cb_body::is_triggered_only(association_type, bool value, error_handler& err, int32_t line, individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::is_triggered_only;
	}
}

void cb_body::constructing_cb(association_type, bool value, error_handler& err, int32_t line, individual_cb_context& context) {
	if(!value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::is_not_constructing_cb;
	}
}

void cb_body::great_war_obligatory(association_type, bool value, error_handler& err, int32_t line,
		individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::great_war_obligatory;
		context.outer_context.state.military_definitions.standard_great_war = context.id;
	}
}

void cb_body::all_allowed_states(association_type, bool value, error_handler& err, int32_t line, individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::all_allowed_states;
	}
}

void cb_body::crisis(association_type, bool value, error_handler& err, int32_t line, individual_cb_context& context) {
	if(!value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::not_in_crisis;
	}
}

void cb_body::po_clear_union_sphere(association_type, bool value, error_handler& err, int32_t line,
		individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::po_clear_union_sphere;
	}
}

void cb_body::po_gunboat(association_type, bool value, error_handler& err, int32_t line, individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::po_gunboat;
	}
}

void cb_body::po_annex(association_type, bool value, error_handler& err, int32_t line, individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::po_annex;
	}
}

void cb_body::po_demand_state(association_type, bool value, error_handler& err, int32_t line, individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::po_demand_state;
	}
}

void cb_body::po_add_to_sphere(association_type, bool value, error_handler& err, int32_t line, individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::po_add_to_sphere;
	}
}

void cb_body::po_disarmament(association_type, bool value, error_handler& err, int32_t line, individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::po_disarmament;
	}
}

void cb_body::po_reparations(association_type, bool value, error_handler& err, int32_t line, individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::po_reparations;
	}
}

void cb_body::po_transfer_provinces(association_type, bool value, error_handler& err, int32_t line,
		individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::po_transfer_provinces;
	}
}

void cb_body::po_remove_prestige(association_type, bool value, error_handler& err, int32_t line, individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::po_remove_prestige;
	}
}

void cb_body::po_make_puppet(association_type, bool value, error_handler& err, int32_t line, individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::po_make_puppet;
	}
}

void cb_body::po_release_puppet(association_type, bool value, error_handler& err, int32_t line, individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::po_release_puppet;
	}
}

void cb_body::po_status_quo(association_type, bool value, error_handler& err, int32_t line, individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::po_status_quo;
	}
}

void cb_body::po_install_communist_gov_type(association_type, bool value, error_handler& err, int32_t line,
		individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::po_install_communist_gov_type;
	}
}

void cb_body::po_uninstall_communist_gov_type(association_type, bool value, error_handler& err, int32_t line,
		individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::po_uninstall_communist_gov_type;
	}
}

void cb_body::po_remove_cores(association_type, bool value, error_handler& err, int32_t line, individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::po_remove_cores;
	}
}

void cb_body::po_colony(association_type, bool value, error_handler& err, int32_t line, individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::po_colony;
		context.outer_context.state.military_definitions.crisis_colony = context.id;
	}
}

void cb_body::po_destroy_forts(association_type, bool value, error_handler& err, int32_t line, individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::po_destroy_forts;
	}
}

void cb_body::po_destroy_naval_bases(association_type, bool value, error_handler& err, int32_t line,
		individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::po_destroy_naval_bases;
	}
}

void cb_body::po_make_substate(association_type, bool value, error_handler& err, int32_t line, individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::po_make_substate;
	}
}

void cb_body::po_save_subjects(association_type, bool value, error_handler& err, int32_t line, individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::po_save_subjects;
	}
}

void cb_body::po_unequal_treaty(association_type, bool value, error_handler& err, int32_t line, individual_cb_context& context) {
	if(value) {
		context.outer_context.state.world.cb_type_get_type_bits(context.id) |= military::cb_flag::po_unequal_treaty;
	}
}

void cb_body::war_name(association_type, std::string_view value, error_handler& err, int32_t line,
		individual_cb_context& context) {
	context.outer_context.state.world.cb_type_set_war_name(context.id,
			text::find_or_add_key(context.outer_context.state, std::string("normal_") + std::string(value), false));
}

void cb_body::badboy_factor(association_type, float value, error_handler& err, int32_t line, individual_cb_context& context) {
	context.outer_context.state.world.cb_type_set_badboy_factor(context.id, value);
}

void cb_body::prestige_factor(association_type, float value, error_handler& err, int32_t line, individual_cb_context& context) {
	context.outer_context.state.world.cb_type_set_prestige_factor(context.id, value);
}

void cb_body::peace_cost_factor(association_type, float value, error_handler& err, int32_t line, individual_cb_context& context) {
	context.outer_context.state.world.cb_type_set_peace_cost_factor(context.id, value);
}

void cb_body::penalty_factor(association_type, float value, error_handler& err, int32_t line, individual_cb_context& context) {
	context.outer_context.state.world.cb_type_set_penalty_factor(context.id, value);
}

void cb_body::break_truce_prestige_factor(association_type, float value, error_handler& err, int32_t line,
		individual_cb_context& context) {
	context.outer_context.state.world.cb_type_set_break_truce_prestige_factor(context.id, value);
}

void cb_body::break_truce_infamy_factor(association_type, float value, error_handler& err, int32_t line,
		individual_cb_context& context) {
	context.outer_context.state.world.cb_type_set_break_truce_infamy_factor(context.id, value);
}

void cb_body::break_truce_militancy_factor(association_type, float value, error_handler& err, int32_t line,
		individual_cb_context& context) {
	context.outer_context.state.world.cb_type_set_break_truce_militancy_factor(context.id, value);
}

void cb_body::good_relation_prestige_factor(association_type, float value, error_handler& err, int32_t line,
		individual_cb_context& context) {
	context.outer_context.state.world.cb_type_set_good_relation_prestige_factor(context.id, value);
}

void cb_body::good_relation_infamy_factor(association_type, float value, error_handler& err, int32_t line,
		individual_cb_context& context) {
	context.outer_context.state.world.cb_type_set_good_relation_infamy_factor(context.id, value);
}

void cb_body::good_relation_militancy_factor(association_type, float value, error_handler& err, int32_t line,
		individual_cb_context& context) {
	context.outer_context.state.world.cb_type_set_good_relation_militancy_factor(context.id, value);
}

void cb_body::construction_speed(association_type, float value, error_handler& err, int32_t line,
		individual_cb_context& context) {
	context.outer_context.state.world.cb_type_set_construction_speed(context.id, value);
}

void cb_body::tws_battle_factor(association_type, float value, error_handler& err, int32_t line, individual_cb_context& context) {
	context.outer_context.state.world.cb_type_set_tws_battle_factor(context.id, value);
}

void cb_body::allowed_states(dcon::trigger_key value, error_handler& err, int32_t line, individual_cb_context& context) {
	context.outer_context.state.world.cb_type_set_allowed_states(context.id, value);
}

void cb_body::allowed_states_in_crisis(dcon::trigger_key value, error_handler& err, int32_t line,
		individual_cb_context& context) {
	context.outer_context.state.world.cb_type_set_allowed_states_in_crisis(context.id, value);
}

void cb_body::allowed_substate_regions(dcon::trigger_key value, error_handler& err, int32_t line,
		individual_cb_context& context) {
	context.outer_context.state.world.cb_type_set_allowed_substate_regions(context.id, value);
}

void cb_body::allowed_countries(dcon::trigger_key value, error_handler& err, int32_t line, individual_cb_context& context) {
	context.outer_context.state.world.cb_type_set_allowed_countries(context.id, value);
}

void cb_body::can_use(dcon::trigger_key value, error_handler& err, int32_t line, individual_cb_context& context) {
	context.outer_context.state.world.cb_type_set_can_use(context.id, value);
}

void cb_body::on_add(dcon::effect_key value, error_handler& err, int32_t line, individual_cb_context& context) {
	context.outer_context.state.world.cb_type_set_on_add(context.id, value);
}

void cb_body::on_po_accepted(dcon::effect_key value, error_handler& err, int32_t line, individual_cb_context& context) {
	context.outer_context.state.world.cb_type_set_on_po_accepted(context.id, value);
}

void option_rules::build_factory(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::build_factory;
}

void option_rules::expand_factory(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::expand_factory;
}

void option_rules::open_factory(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::open_factory;
}

void option_rules::destroy_factory(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::destroy_factory;
}

void option_rules::factory_priority(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::factory_priority;
}

void option_rules::can_subsidise(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::can_subsidise;
}

void option_rules::pop_build_factory(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::pop_build_factory;
}

void option_rules::pop_expand_factory(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::pop_expand_factory;
}

void option_rules::pop_open_factory(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::pop_open_factory;
}

void option_rules::delete_factory_if_no_input(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::delete_factory_if_no_input;
}

void option_rules::build_factory_invest(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::build_factory_invest;
}

void option_rules::expand_factory_invest(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::expand_factory_invest;
}

void option_rules::open_factory_invest(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::open_factory_invest;
}

void option_rules::build_railway_invest(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::build_railway_invest;
}

void option_rules::can_invest_in_pop_projects(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::can_invest_in_pop_projects;
}

void option_rules::pop_build_factory_invest(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::pop_build_factory_invest;
}

void option_rules::pop_expand_factory_invest(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::pop_expand_factory_invest;
}

void option_rules::pop_open_factory_invest(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::pop_open_factory_invest;
}

void option_rules::allow_foreign_investment(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::allow_foreign_investment;
}

void option_rules::slavery_allowed(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::slavery_allowed;
}

void option_rules::primary_culture_voting(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::primary_culture_voting;
}

void option_rules::culture_voting(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::culture_voting;
}

void option_rules::all_voting(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::all_voting;
}

void option_rules::largest_share(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::largest_share;
}

void option_rules::dhont(association_type, bool value, error_handler& err, int32_t line, individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::dhont;
}

void option_rules::sainte_laque(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::sainte_laque;
}

void option_rules::same_as_ruling_party(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::same_as_ruling_party;
}

void option_rules::rich_only(association_type, bool value, error_handler& err, int32_t line, individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::rich_only;
}

void option_rules::state_vote(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::state_vote;
}

void option_rules::population_vote(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::population_vote;
}

void option_rules::build_railway(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::build_railway;
}

void option_rules::build_bank(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::build_bank;
}

void option_rules::build_university(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.world.issue_option_get_rules(context.id) |= issue_rule::build_university;
}

void option_rules::build_factory(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::build_factory;
}

void option_rules::expand_factory(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::expand_factory;
}

void option_rules::open_factory(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::open_factory;
}

void option_rules::destroy_factory(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::destroy_factory;
}

void option_rules::factory_priority(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::factory_priority;
}

void option_rules::can_subsidise(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::can_subsidise;
}

void option_rules::pop_build_factory(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::pop_build_factory;
}

void option_rules::pop_expand_factory(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::pop_expand_factory;
}

void option_rules::pop_open_factory(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::pop_open_factory;
}

void option_rules::delete_factory_if_no_input(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::delete_factory_if_no_input;
}

void option_rules::build_factory_invest(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::build_factory_invest;
}

void option_rules::expand_factory_invest(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::expand_factory_invest;
}

void option_rules::open_factory_invest(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::open_factory_invest;
}

void option_rules::build_railway_invest(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::build_railway_invest;
}

void option_rules::can_invest_in_pop_projects(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::can_invest_in_pop_projects;
}

void option_rules::pop_build_factory_invest(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::pop_build_factory_invest;
}

void option_rules::pop_expand_factory_invest(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::pop_expand_factory_invest;
}

void option_rules::pop_open_factory_invest(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::pop_open_factory_invest;
}

void option_rules::allow_foreign_investment(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::allow_foreign_investment;
}

void option_rules::slavery_allowed(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::slavery_allowed;
}

void option_rules::primary_culture_voting(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::primary_culture_voting;
}

void option_rules::culture_voting(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::culture_voting;
}

void option_rules::all_voting(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::all_voting;
}

void option_rules::largest_share(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::largest_share;
}

void option_rules::dhont(association_type, bool value, error_handler& err, int32_t line, individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::dhont;
}

void option_rules::sainte_laque(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::sainte_laque;
}

void option_rules::same_as_ruling_party(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::same_as_ruling_party;
}

void option_rules::rich_only(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::rich_only;
}

void option_rules::state_vote(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::state_vote;
}

void option_rules::population_vote(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::population_vote;
}

void option_rules::build_railway(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::build_railway;
}

void option_rules::build_bank(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::build_bank;
}

void option_rules::build_university(association_type, bool value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	if(value)
		context.outer_context.state.world.reform_option_get_rules(context.id) |= issue_rule::build_university;
}

void issue_option_body::technology_cost(association_type, int32_t value, error_handler& err, int32_t line,
		individual_option_context& context) {

	err.accumulated_errors +=
			"Only uncivilized nation reforms may have a tech cost (" + err.file_name + " line " + std::to_string(line) + ")\n";
}

void issue_option_body::war_exhaustion_effect(association_type, float value, error_handler& err, int32_t line,
		individual_option_context& context) {
	context.outer_context.state.world.issue_option_set_war_exhaustion_effect(context.id, value);
}

void issue_option_body::administrative_multiplier(association_type, float value, error_handler& err, int32_t line,
		individual_option_context& context) {
	context.outer_context.state.world.issue_option_set_administrative_multiplier(context.id, value);
}

void issue_option_body::is_jingoism(association_type, bool value, error_handler& err, int32_t line,
		individual_option_context& context) {
	if(value)
		context.outer_context.state.culture_definitions.jingoism = context.id;
}

void issue_option_body::technology_cost(association_type, int32_t value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	context.outer_context.state.world.reform_option_set_technology_cost(context.id, value);
}

void issue_option_body::war_exhaustion_effect(association_type, float value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	context.outer_context.state.world.reform_option_set_war_exhaustion_effect(context.id, value);
}

void issue_option_body::administrative_multiplier(association_type, float value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	err.accumulated_errors +=
			"Error, only issues can have an administrative multiplier (" + err.file_name + " line " + std::to_string(line) + ")\n";
}

void issue_option_body::on_execute(on_execute_body const& value, error_handler& err, int32_t line,
		individual_option_context& context) {
	context.outer_context.state.world.issue_option_set_on_execute_trigger(context.id, value.trigger);
	context.outer_context.state.world.issue_option_set_on_execute_effect(context.id, value.effect);
}

void issue_option_body::on_execute(on_execute_body const& value, error_handler& err, int32_t line,
		individual_roption_context& context) {
	context.outer_context.state.world.reform_option_set_on_execute_trigger(context.id, value.trigger);
	context.outer_context.state.world.reform_option_set_on_execute_effect(context.id, value.effect);
}

void national_focus::railroads(association_type, float value, error_handler& err, int32_t line, national_focus_context& context) {
	context.outer_context.state.world.national_focus_set_railroads(context.id, value);
}

void national_focus::limit(dcon::trigger_key value, error_handler& err, int32_t line, national_focus_context& context) {
	context.outer_context.state.world.national_focus_set_limit(context.id, value);
}

void national_focus::has_flashpoint(association_type, bool value, error_handler& err, int32_t line,
		national_focus_context& context) {
	if(value)
		context.outer_context.state.national_definitions.flashpoint_focus = context.id;
}

void national_focus::flashpoint_tension(association_type, float value, error_handler& err, int32_t line,
		national_focus_context& context) {
	context.outer_context.state.national_definitions.flashpoint_amount = value;
}

void national_focus::ideology(association_type, std::string_view value, error_handler& err, int32_t line,
		national_focus_context& context) {
	if(auto it = context.outer_context.map_of_ideologies.find(std::string(value));
			it != context.outer_context.map_of_ideologies.end()) {
		context.outer_context.state.world.national_focus_set_ideology(context.id, it->second.id);
	} else {
		err.accumulated_errors +=
				"Invalid ideology " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void national_focus::loyalty_value(association_type, float value, error_handler& err, int32_t line,
		national_focus_context& context) {
	context.outer_context.state.world.national_focus_set_loyalty_value(context.id, value);
}

void national_focus::any_value(std::string_view label, association_type, float value, error_handler& err, int32_t line,
		national_focus_context& context) {
	std::string str_label{label};
	if(auto it = context.outer_context.map_of_poptypes.find(str_label); it != context.outer_context.map_of_poptypes.end()) {
		context.outer_context.state.world.national_focus_set_promotion_type(context.id, it->second);
		context.outer_context.state.world.national_focus_set_promotion_amount(context.id, value);
	} else if(auto itb = context.outer_context.map_of_commodity_names.find(str_label);
						itb != context.outer_context.map_of_commodity_names.end()) {
		context.outer_context.state.world.national_focus_set_production_focus(context.id, itb->second, value);
	} else {
		err.accumulated_errors +=
				"Invalid pop type / commodity " + str_label + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void main_pop_type_file::promotion_chance(dcon::value_modifier_key value, error_handler& err, int32_t line,
		scenario_building_context& context) {
	context.state.culture_definitions.promotion_chance = value;
}

void main_pop_type_file::demotion_chance(dcon::value_modifier_key value, error_handler& err, int32_t line,
		scenario_building_context& context) {
	context.state.culture_definitions.demotion_chance = value;
}

void main_pop_type_file::migration_chance(dcon::value_modifier_key value, error_handler& err, int32_t line,
		scenario_building_context& context) {
	context.state.culture_definitions.migration_chance = value;
}

void main_pop_type_file::colonialmigration_chance(dcon::value_modifier_key value, error_handler& err, int32_t line,
		scenario_building_context& context) {
	context.state.culture_definitions.colonialmigration_chance = value;
}

void main_pop_type_file::emigration_chance(dcon::value_modifier_key value, error_handler& err, int32_t line,
		scenario_building_context& context) {
	context.state.culture_definitions.emigration_chance = value;
}

void main_pop_type_file::assimilation_chance(dcon::value_modifier_key value, error_handler& err, int32_t line,
		scenario_building_context& context) {
	context.state.culture_definitions.assimilation_chance = value;
}

void main_pop_type_file::conversion_chance(dcon::value_modifier_key value, error_handler& err, int32_t line,
		scenario_building_context& context) {
	context.state.culture_definitions.conversion_chance = value;
}

void tech_rgo_goods_output::any_value(std::string_view label, association_type, float value, error_handler& err, int32_t line,
		tech_context& context) {
	if(auto it = context.outer_context.map_of_commodity_names.find(std::string(label));
			it != context.outer_context.map_of_commodity_names.end()) {
		context.outer_context.state.world.technology_get_rgo_goods_output(context.id)
				.push_back(sys::commodity_modifier{value, it->second});
	} else {
		err.accumulated_errors +=
				"Invalid commodity " + std::string(label) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void tech_fac_goods_output::any_value(std::string_view label, association_type, float value, error_handler& err, int32_t line,
		tech_context& context) {
	if(auto it = context.outer_context.map_of_commodity_names.find(std::string(label));
			it != context.outer_context.map_of_commodity_names.end()) {
		context.outer_context.state.world.technology_get_factory_goods_output(context.id)
				.push_back(sys::commodity_modifier{value, it->second});
	} else {
		err.accumulated_errors +=
				"Invalid commodity " + std::string(label) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void tech_rgo_size::any_value(std::string_view label, association_type, float value, error_handler& err, int32_t line, tech_context& context) {
	if(auto it = context.outer_context.map_of_commodity_names.find(std::string(label));
			it != context.outer_context.map_of_commodity_names.end()) {
		context.outer_context.state.world.technology_get_rgo_size(context.id).push_back(sys::commodity_modifier{value, it->second});
	} else {
		err.accumulated_errors +=
				"Invalid commodity " + std::string(label) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}
void tech_rgo_size::any_value(std::string_view label, association_type, float value, error_handler& err, int32_t line, invention_context& context) {
	if(auto it = context.outer_context.map_of_commodity_names.find(std::string(label));
			it != context.outer_context.map_of_commodity_names.end()) {
		context.outer_context.state.world.invention_get_rgo_size(context.id).push_back(sys::commodity_modifier{ value, it->second });
	} else {
		err.accumulated_errors +=
			"Invalid commodity " + std::string(label) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void technology_contents::any_group(std::string_view label, unit_modifier_body const& value, error_handler& err, int32_t line,
		tech_context& context) {
	if(auto it = context.outer_context.map_of_unit_types.find(std::string(label));
			it != context.outer_context.map_of_unit_types.end()) {
		sys::unit_modifier temp = value;
		temp.type = it->second;
		context.outer_context.state.world.technology_get_modified_units(context.id).push_back(temp);
	} else {
		err.accumulated_errors +=
				"Invalid unit type " + std::string(label) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void technology_contents::ai_chance(dcon::value_modifier_key value, error_handler& err, int32_t line, tech_context& context) {
	context.outer_context.state.world.technology_set_ai_chance(context.id, value);
}

void technology_contents::year(association_type, int32_t value, error_handler& err, int32_t line, tech_context& context) {
	context.outer_context.state.world.technology_set_year(context.id, int16_t(value));
}

void technology_contents::cost(association_type, int32_t value, error_handler& err, int32_t line, tech_context& context) {
	context.outer_context.state.world.technology_set_cost(context.id, value);
}

void technology_contents::leadership_cost(association_type, int32_t value, error_handler& err, int32_t line, tech_context& context) {
	context.outer_context.state.world.technology_set_leadership_cost(context.id, value);
}

void technology_contents::area(association_type, std::string_view value, error_handler& err, int32_t line,
		tech_context& context) {
	if(auto it = context.outer_context.map_of_tech_folders.find(std::string(value));
			it != context.outer_context.map_of_tech_folders.end()) {
		context.outer_context.state.world.technology_set_folder_index(context.id, uint8_t(it->second));
	} else {
		err.accumulated_errors += "Invalid technology folder name " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void technology_contents::colonial_points(association_type, int32_t value, error_handler& err, int32_t line,
		tech_context& context) {
	context.outer_context.state.world.technology_set_colonial_points(context.id, int16_t(value));
}

void technology_contents::activate_unit(association_type, std::string_view value, error_handler& err, int32_t line,
		tech_context& context) {
	if(auto it = context.outer_context.map_of_unit_types.find(std::string(value));
			it != context.outer_context.map_of_unit_types.end()) {
		context.outer_context.state.world.technology_set_activate_unit(context.id, it->second, true);
	} else {
		err.accumulated_errors +=
				"Invalid unit type " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void technology_contents::activate_building(association_type, std::string_view value, error_handler& err, int32_t line,
		tech_context& context) {
	for(auto t = economy::province_building_type::railroad; t != economy::province_building_type::last; t = economy::province_building_type(uint8_t(t) + 1)) {
		if(std::string(value) == economy::province_building_type_get_name(t)) {
			context.outer_context.state.world.technology_set_increase_building(context.id, t, true);
			return;
		}
	}

	if(auto it = context.outer_context.map_of_factory_names.find(std::string(value));
						it != context.outer_context.map_of_factory_names.end()) {
		context.outer_context.state.world.technology_set_activate_building(context.id, it->second, true);
	} else {
		err.accumulated_errors +=
				"Invalid factory type " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void technology_contents::any_value(std::string_view name, association_type, int32_t value, error_handler& err, int32_t line, tech_context& context) {
	if(has_fixed_prefix_ci(name.data(), name.data() + name.length(), "max_")) {
		for(auto t = economy::province_building_type::railroad; t != economy::province_building_type::last; t = economy::province_building_type(uint8_t(t) + 1)) {
			if(std::string(name).substr(4) == economy::province_building_type_get_name(t)) {
				if(value == 1) {
					context.outer_context.state.world.technology_set_increase_building(context.id, t, true);
				} else {
					err.accumulated_errors += "max_" + std::string(economy::province_building_type_get_name(t)) + " may only be 1 (" + err.file_name + " line " + std::to_string(line) + ")\n";
				}
				return;
			}
		}
	}
	err.accumulated_errors += "unknown technology key " + std::string(name) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";//err.unhandled_association_key();
}

void inv_rgo_goods_output::any_value(std::string_view label, association_type, float value, error_handler& err, int32_t line,
		invention_context& context) {
	if(auto it = context.outer_context.map_of_commodity_names.find(std::string(label));
			it != context.outer_context.map_of_commodity_names.end()) {
		context.outer_context.state.world.invention_get_rgo_goods_output(context.id)
				.push_back(sys::commodity_modifier{value, it->second});
	} else {
		err.accumulated_errors +=
				"Invalid commodity " + std::string(label) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void inv_fac_goods_output::any_value(std::string_view label, association_type, float value, error_handler& err, int32_t line,
		invention_context& context) {
	if(auto it = context.outer_context.map_of_commodity_names.find(std::string(label));
			it != context.outer_context.map_of_commodity_names.end()) {
		context.outer_context.state.world.invention_get_factory_goods_output(context.id)
				.push_back(sys::commodity_modifier{value, it->second});
	} else {
		err.accumulated_errors +=
				"Invalid commodity " + std::string(label) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void inv_fac_goods_throughput::any_value(std::string_view label, association_type, float value, error_handler& err, int32_t line,
		invention_context& context) {
	if(auto it = context.outer_context.map_of_commodity_names.find(std::string(label));
			it != context.outer_context.map_of_commodity_names.end()) {
		context.outer_context.state.world.invention_get_factory_goods_throughput(context.id)
				.push_back(sys::commodity_modifier{value, it->second});
	} else {
		err.accumulated_errors +=
				"Invalid commodity " + std::string(label) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void inv_rebel_org_gain::faction(association_type, std::string_view v, error_handler& err, int32_t line,
		invention_context& context) {
	if(is_fixed_token_ci(v.data(), v.data() + v.size(), "all")) {
		// do nothing
	} else if(auto it = context.outer_context.map_of_rebeltypes.find(std::string(v));
						it != context.outer_context.map_of_rebeltypes.end()) {
		faction_ = it->second.id;
	} else {
		err.accumulated_errors +=
				"Invalid rebel type " + std::string(v) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void inv_effect::any_group(std::string_view label, unit_modifier_body const& value, error_handler& err, int32_t line,
		invention_context& context) {
	if(auto it = context.outer_context.map_of_unit_types.find(std::string(label));
			it != context.outer_context.map_of_unit_types.end()) {
		sys::unit_modifier temp = value;
		temp.type = it->second;
		context.outer_context.state.world.invention_get_modified_units(context.id).push_back(temp);
	} else {
		err.accumulated_errors +=
				"Invalid unit type " + std::string(label) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void inv_effect::activate_unit(association_type, std::string_view value, error_handler& err, int32_t line,
		invention_context& context) {
	if(auto it = context.outer_context.map_of_unit_types.find(std::string(value));
			it != context.outer_context.map_of_unit_types.end()) {
		context.outer_context.state.world.invention_set_activate_unit(context.id, it->second, true);
	} else {
		err.accumulated_errors +=
				"Invalid unit type " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void inv_effect::activate_building(association_type, std::string_view value, error_handler& err, int32_t line, invention_context& context) {
	for(auto t = economy::province_building_type::railroad; t != economy::province_building_type::last; t = economy::province_building_type(uint8_t(t) + 1)) {
		if(std::string(value) == economy::province_building_type_get_name(t)) {
			context.outer_context.state.world.invention_set_increase_building(context.id, t, true);
			return;
		}
	}

	if(auto it = context.outer_context.map_of_factory_names.find(std::string(value));
			it != context.outer_context.map_of_factory_names.end()) {
		context.outer_context.state.world.invention_set_activate_building(context.id, it->second, true);
	} else {
		err.accumulated_errors +=
				"Invalid factory type " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void inv_effect::any_value(std::string_view name, association_type, int32_t value, error_handler& err, int32_t line, invention_context& context) {
	if(has_fixed_prefix_ci(name.data(), name.data() + name.length(), "max_")) {
		for(auto t = economy::province_building_type::railroad; t != economy::province_building_type::last; t = economy::province_building_type(uint8_t(t) + 1)) {
			if(std::string(name).substr(4) == economy::province_building_type_get_name(t)) {
				if(value == 1) {
					context.outer_context.state.world.invention_set_increase_building(context.id, t, true);
				} else {
					err.accumulated_errors += "max_" + std::string(economy::province_building_type_get_name(t)) + " may only be 1 (" + err.file_name + " line " + std::to_string(line) + ")\n";
				}
				return;
			}
		}
	}
	err.accumulated_errors += "unknown technology key " + std::string(name) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";//err.unhandled_association_key();
}

void inv_effect::shared_prestige(association_type, float value, error_handler& err, int32_t line, invention_context& context) {
	context.outer_context.state.world.invention_set_shared_prestige(context.id, value);
}
void inv_effect::plurality(association_type, float value, error_handler& err, int32_t line, invention_context& context) {
	context.outer_context.state.world.invention_set_plurality(context.id, value);
}
void technology_contents::plurality(association_type, float value, error_handler& err, int32_t line, tech_context& context) {
	context.outer_context.state.world.technology_set_plurality(context.id, value);
}
void inv_effect::colonial_points(association_type, int32_t value, error_handler& err, int32_t line, invention_context& context) {
	context.outer_context.state.world.invention_set_colonial_points(context.id, int16_t(value));
}

void inv_effect::enable_crime(association_type, std::string_view value, error_handler& err, int32_t line,
		invention_context& context) {
	if(auto it = context.outer_context.map_of_crimes.find(std::string(value)); it != context.outer_context.map_of_crimes.end()) {
		context.outer_context.state.world.invention_set_activate_crime(context.id, it->second.id, true);
	} else {
		err.accumulated_errors +=
				"Invalid crime " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void inv_effect::gas_attack(association_type, bool value, error_handler& err, int32_t line, invention_context& context) {
	context.outer_context.state.world.invention_set_enable_gas_attack(context.id, value);
}

void inv_effect::gas_defence(association_type, bool value, error_handler& err, int32_t line, invention_context& context) {
	context.outer_context.state.world.invention_set_enable_gas_defense(context.id, value);
}

void inv_effect::rebel_org_gain(inv_rebel_org_gain const& value, error_handler& err, int32_t line, invention_context& context) {
	context.outer_context.state.world.invention_get_rebel_org(context.id)
			.push_back(sys::rebel_org_modifier{value.value, value.faction_});
}

void invention_contents::shared_prestige(association_type, float value, error_handler& err, int32_t line, invention_context& context) {
	context.outer_context.state.world.invention_set_shared_prestige(context.id, value);
}

void invention_contents::limit(dcon::trigger_key value, error_handler& err, int32_t line, invention_context& context) {
	context.outer_context.state.world.invention_set_limit(context.id, value);
}

void invention_contents::chance(dcon::value_modifier_key value, error_handler& err, int32_t line, invention_context& context) {
	context.outer_context.state.world.invention_set_chance(context.id, value);
}

void invention_contents::effect(inv_effect const& value, error_handler& err, int32_t line, invention_context& context) {
	for(uint32_t i = 0; i < value.next_to_add_n; ++i) {
		if(next_to_add_n >= sys::national_modifier_definition::modifier_definition_size) {
			err.accumulated_errors +=
					"Too many modifiers attached to invention (" + err.file_name + " line " + std::to_string(line) + ")\n";
			break;
		}
		constructed_definition_n.offsets[next_to_add_n] = value.peek_national_mod().offsets[i];
		constructed_definition_n.values[next_to_add_n] = value.peek_national_mod().values[i];
		++next_to_add_n;
	}
	for(uint32_t i = 0; i < value.next_to_add_p; ++i) {
		if(next_to_add_p >= sys::provincial_modifier_definition::modifier_definition_size) {
			err.accumulated_errors +=
					"Too many modifiers attached to invention (" + err.file_name + " line " + std::to_string(line) + ")\n";
			break;
		}
		constructed_definition_p.offsets[next_to_add_p] = value.peek_province_mod().offsets[i];
		constructed_definition_p.values[next_to_add_p] = value.peek_province_mod().values[i];
		++next_to_add_p;
	}
}

void s_on_yearly_pulse::any_value(std::string_view chance, association_type, int32_t event, error_handler& err, int32_t line,
		scenario_building_context& context) {
	int32_t value = parse_int(chance, line, err);
	if(auto it = context.map_of_national_events.find(event); it != context.map_of_national_events.end()) {
		context.state.national_definitions.on_yearly_pulse.push_back(
				nations::fixed_event{int16_t(value), it->second.id, dcon::trigger_key{}});
	} else {
		auto id = context.state.world.create_national_event();
		context.map_of_national_events.insert_or_assign(event,
				pending_nat_event{id, trigger::slot_contents::nation, trigger::slot_contents::nation, trigger::slot_contents::empty});
		context.state.national_definitions.on_yearly_pulse.push_back(nations::fixed_event{int16_t(value), id, dcon::trigger_key{}});
	}
}

void s_on_quarterly_pulse::any_value(std::string_view chance, association_type, int32_t event, error_handler& err, int32_t line,
		scenario_building_context& context) {
	int32_t value = parse_int(chance, line, err);
	if(auto it = context.map_of_national_events.find(event); it != context.map_of_national_events.end()) {
		context.state.national_definitions.on_quarterly_pulse.push_back(
				nations::fixed_event{int16_t(value), it->second.id, dcon::trigger_key{}});
	} else {
		auto id = context.state.world.create_national_event();
		context.map_of_national_events.insert_or_assign(event,
				pending_nat_event{id, trigger::slot_contents::nation, trigger::slot_contents::nation, trigger::slot_contents::empty});
		context.state.national_definitions.on_quarterly_pulse.push_back(
				nations::fixed_event{int16_t(value), id, dcon::trigger_key{}});
	}
}

void s_on_battle_won::any_value(std::string_view chance, association_type, int32_t event, error_handler& err, int32_t line,
		scenario_building_context& context) {
	int32_t value = parse_int(chance, line, err);
	if(auto it = context.map_of_provincial_events.find(event); it != context.map_of_provincial_events.end()) {
		context.state.national_definitions.on_battle_won.push_back(
				nations::fixed_province_event{int16_t(value), it->second.id, dcon::trigger_key{}});
	} else {
		auto id = context.state.world.create_provincial_event();
		context.map_of_provincial_events.insert_or_assign(event,
				pending_prov_event{id, trigger::slot_contents::province, trigger::slot_contents::nation, trigger::slot_contents::nation});
		context.state.national_definitions.on_battle_won.push_back(
				nations::fixed_province_event{int16_t(value), id, dcon::trigger_key{}});
	}
}

void s_on_battle_lost::any_value(std::string_view chance, association_type, int32_t event, error_handler& err, int32_t line,
		scenario_building_context& context) {
	int32_t value = parse_int(chance, line, err);
	if(auto it = context.map_of_provincial_events.find(event); it != context.map_of_provincial_events.end()) {
		context.state.national_definitions.on_battle_lost.push_back(
				nations::fixed_province_event{int16_t(value), it->second.id, dcon::trigger_key{}});
	} else {
		auto id = context.state.world.create_provincial_event();
		context.map_of_provincial_events.insert_or_assign(event,
				pending_prov_event{id, trigger::slot_contents::province, trigger::slot_contents::nation, trigger::slot_contents::nation});
		context.state.national_definitions.on_battle_lost.push_back(
				nations::fixed_province_event{int16_t(value), id, dcon::trigger_key{}});
	}
}

void s_on_surrender::any_value(std::string_view chance, association_type, int32_t event, error_handler& err, int32_t line,
		scenario_building_context& context) {
	int32_t value = parse_int(chance, line, err);
	if(auto it = context.map_of_national_events.find(event); it != context.map_of_national_events.end()) {
		context.state.national_definitions.on_surrender.push_back(
				nations::fixed_event{int16_t(value), it->second.id, dcon::trigger_key{}});
	} else {
		auto id = context.state.world.create_national_event();
		context.map_of_national_events.insert_or_assign(event,
				pending_nat_event{id, trigger::slot_contents::nation, trigger::slot_contents::nation, trigger::slot_contents::nation});
		context.state.national_definitions.on_surrender.push_back(nations::fixed_event{int16_t(value), id, dcon::trigger_key{}});
	}
}

void s_on_new_great_nation::any_value(std::string_view chance, association_type, int32_t event, error_handler& err, int32_t line,
		scenario_building_context& context) {
	int32_t value = parse_int(chance, line, err);
	if(auto it = context.map_of_national_events.find(event); it != context.map_of_national_events.end()) {
		context.state.national_definitions.on_new_great_nation.push_back(
				nations::fixed_event{int16_t(value), it->second.id, dcon::trigger_key{}});
	} else {
		auto id = context.state.world.create_national_event();
		context.map_of_national_events.insert_or_assign(event,
				pending_nat_event{id, trigger::slot_contents::nation, trigger::slot_contents::nation, trigger::slot_contents::empty});
		context.state.national_definitions.on_new_great_nation.push_back(
				nations::fixed_event{int16_t(value), id, dcon::trigger_key{}});
	}
}

void s_on_lost_great_nation::any_value(std::string_view chance, association_type, int32_t event, error_handler& err, int32_t line,
		scenario_building_context& context) {
	int32_t value = parse_int(chance, line, err);
	if(auto it = context.map_of_national_events.find(event); it != context.map_of_national_events.end()) {
		context.state.national_definitions.on_lost_great_nation.push_back(
				nations::fixed_event{int16_t(value), it->second.id, dcon::trigger_key{}});
	} else {
		auto id = context.state.world.create_national_event();
		context.map_of_national_events.insert_or_assign(event,
				pending_nat_event{id, trigger::slot_contents::nation, trigger::slot_contents::nation, trigger::slot_contents::empty});
		context.state.national_definitions.on_lost_great_nation.push_back(
				nations::fixed_event{int16_t(value), id, dcon::trigger_key{}});
	}
}

void s_on_election_tick::any_value(std::string_view chance, association_type, int32_t event, error_handler& err, int32_t line,
		scenario_building_context& context) {
	int32_t value = parse_int(chance, line, err);
	if(auto it = context.map_of_national_events.find(event); it != context.map_of_national_events.end()) {
		context.state.national_definitions.on_election_tick.push_back(
				nations::fixed_election_event{ int16_t(value), it->second.id, dcon::trigger_key{}, dcon::issue_id{} });
	} else {
		auto id = context.state.world.create_national_event();
		context.map_of_national_events.insert_or_assign(event,
				pending_nat_event{id, trigger::slot_contents::nation, trigger::slot_contents::nation, trigger::slot_contents::empty});
		context.state.national_definitions.on_election_tick.push_back(nations::fixed_election_event{ int16_t(value), id, dcon::trigger_key{}, dcon::issue_id{} });
	}
}

void s_on_colony_to_state::any_value(std::string_view chance, association_type, int32_t event, error_handler& err, int32_t line,
		scenario_building_context& context) {
	int32_t value = parse_int(chance, line, err);
	if(auto it = context.map_of_national_events.find(event); it != context.map_of_national_events.end()) {
		context.state.national_definitions.on_colony_to_state.push_back(
				nations::fixed_event{int16_t(value), it->second.id, dcon::trigger_key{}});
	} else {
		auto id = context.state.world.create_national_event();
		context.map_of_national_events.insert_or_assign(event,
				pending_nat_event{id, trigger::slot_contents::state, trigger::slot_contents::nation, trigger::slot_contents::empty});
		context.state.national_definitions.on_colony_to_state.push_back(
				nations::fixed_event{int16_t(value), id, dcon::trigger_key{}});
	}
}

void s_on_state_conquest::any_value(std::string_view chance, association_type, int32_t event, error_handler& err, int32_t line,
		scenario_building_context& context) {
	int32_t value = parse_int(chance, line, err);
	if(auto it = context.map_of_national_events.find(event); it != context.map_of_national_events.end()) {
		context.state.national_definitions.on_state_conquest.push_back(
				nations::fixed_event{int16_t(value), it->second.id, dcon::trigger_key{}});
	} else {
		auto id = context.state.world.create_national_event();
		context.map_of_national_events.insert_or_assign(event,
				pending_nat_event{id, trigger::slot_contents::state, trigger::slot_contents::nation, trigger::slot_contents::empty});
		context.state.national_definitions.on_state_conquest.push_back(nations::fixed_event{int16_t(value), id, dcon::trigger_key{}});
	}
}

void s_on_colony_to_state_free_slaves::any_value(std::string_view chance, association_type, int32_t event, error_handler& err,
		int32_t line, scenario_building_context& context) {
	int32_t value = parse_int(chance, line, err);
	if(auto it = context.map_of_national_events.find(event); it != context.map_of_national_events.end()) {
		context.state.national_definitions.on_colony_to_state_free_slaves.push_back(
				nations::fixed_event{int16_t(value), it->second.id, dcon::trigger_key{}});
	} else {
		auto id = context.state.world.create_national_event();
		context.map_of_national_events.insert_or_assign(event,
				pending_nat_event{id, trigger::slot_contents::state, trigger::slot_contents::nation, trigger::slot_contents::empty});
		context.state.national_definitions.on_colony_to_state_free_slaves.push_back(
				nations::fixed_event{int16_t(value), id, dcon::trigger_key{}});
	}
}

void s_on_debtor_default::any_value(std::string_view chance, association_type, int32_t event, error_handler& err, int32_t line,
		scenario_building_context& context) {
	int32_t value = parse_int(chance, line, err);
	if(auto it = context.map_of_national_events.find(event); it != context.map_of_national_events.end()) {
		context.state.national_definitions.on_debtor_default.push_back(
				nations::fixed_event{int16_t(value), it->second.id, dcon::trigger_key{}});
	} else {
		auto id = context.state.world.create_national_event();
		context.map_of_national_events.insert_or_assign(event,
				pending_nat_event{id, trigger::slot_contents::nation, trigger::slot_contents::nation, trigger::slot_contents::nation});
		context.state.national_definitions.on_debtor_default.push_back(nations::fixed_event{int16_t(value), id, dcon::trigger_key{}});
	}
}

void s_on_debtor_default_small::any_value(std::string_view chance, association_type, int32_t event, error_handler& err,
		int32_t line, scenario_building_context& context) {
	int32_t value = parse_int(chance, line, err);
	if(auto it = context.map_of_national_events.find(event); it != context.map_of_national_events.end()) {
		context.state.national_definitions.on_debtor_default_small.push_back(
				nations::fixed_event{int16_t(value), it->second.id, dcon::trigger_key{}});
	} else {
		auto id = context.state.world.create_national_event();
		context.map_of_national_events.insert_or_assign(event,
				pending_nat_event{id, trigger::slot_contents::nation, trigger::slot_contents::nation, trigger::slot_contents::nation});
		context.state.national_definitions.on_debtor_default_small.push_back(
				nations::fixed_event{int16_t(value), id, dcon::trigger_key{}});
	}
}

void s_on_debtor_default_second::any_value(std::string_view chance, association_type, int32_t event, error_handler& err,
		int32_t line, scenario_building_context& context) {
	int32_t value = parse_int(chance, line, err);
	if(auto it = context.map_of_national_events.find(event); it != context.map_of_national_events.end()) {
		context.state.national_definitions.on_debtor_default_second.push_back(
				nations::fixed_event{int16_t(value), it->second.id, dcon::trigger_key{}});
	} else {
		auto id = context.state.world.create_national_event();
		context.map_of_national_events.insert_or_assign(event,
				pending_nat_event{id, trigger::slot_contents::nation, trigger::slot_contents::nation, trigger::slot_contents::nation});
		context.state.national_definitions.on_debtor_default_second.push_back(
				nations::fixed_event{int16_t(value), id, dcon::trigger_key{}});
	}
}

void s_on_civilize::any_value(std::string_view chance, association_type, int32_t event, error_handler& err, int32_t line,
		scenario_building_context& context) {
	int32_t value = parse_int(chance, line, err);
	if(auto it = context.map_of_national_events.find(event); it != context.map_of_national_events.end()) {
		context.state.national_definitions.on_civilize.push_back(
				nations::fixed_event{int16_t(value), it->second.id, dcon::trigger_key{}});
	} else {
		auto id = context.state.world.create_national_event();
		context.map_of_national_events.insert_or_assign(event,
				pending_nat_event{id, trigger::slot_contents::nation, trigger::slot_contents::nation, trigger::slot_contents::empty});
		context.state.national_definitions.on_civilize.push_back(nations::fixed_event{int16_t(value), id, dcon::trigger_key{}});
	}
}

void s_on_crisis_declare_interest::any_value(std::string_view chance, association_type, int32_t event, error_handler& err,
		int32_t line, scenario_building_context& context) {
	int32_t value = parse_int(chance, line, err);
	if(auto it = context.map_of_national_events.find(event); it != context.map_of_national_events.end()) {
		context.state.national_definitions.on_crisis_declare_interest.push_back(
				nations::fixed_event{int16_t(value), it->second.id, dcon::trigger_key{}});
	} else {
		auto id = context.state.world.create_national_event();
		context.map_of_national_events.insert_or_assign(event,
				pending_nat_event{id, trigger::slot_contents::nation, trigger::slot_contents::nation, trigger::slot_contents::empty});
		context.state.national_definitions.on_crisis_declare_interest.push_back(
				nations::fixed_event{int16_t(value), id, dcon::trigger_key{}});
	}
}

void s_on_election_started::any_value(std::string_view chance, association_type, int32_t event, error_handler& err,
		int32_t line, scenario_building_context& context) {
	int32_t value = parse_int(chance, line, err);
	if(auto it = context.map_of_national_events.find(event); it != context.map_of_national_events.end()) {
		context.state.national_definitions.on_election_started.push_back(
				nations::fixed_event{ int16_t(value), it->second.id, dcon::trigger_key{} });
	} else {
		auto id = context.state.world.create_national_event();
		context.map_of_national_events.insert_or_assign(event,
				pending_nat_event {id, trigger::slot_contents::nation, trigger::slot_contents::nation, trigger::slot_contents::empty});
		context.state.national_definitions.on_election_started.push_back(
				nations::fixed_event{ int16_t(value), id, dcon::trigger_key{} });
	}
}

void s_on_election_finished::any_value(std::string_view chance, association_type, int32_t event, error_handler& err,
		int32_t line, scenario_building_context& context) {
	int32_t value = parse_int(chance, line, err);
	if(auto it = context.map_of_national_events.find(event); it != context.map_of_national_events.end()) {
		context.state.national_definitions.on_election_finished.push_back(
				nations::fixed_event{ int16_t(value), it->second.id, dcon::trigger_key{} });
	} else {
		auto id = context.state.world.create_national_event();
		context.map_of_national_events.insert_or_assign(event,
				pending_nat_event {id, trigger::slot_contents::nation, trigger::slot_contents::nation, trigger::slot_contents::empty});
		context.state.national_definitions.on_election_finished.push_back(
				nations::fixed_event{ int16_t(value), id, dcon::trigger_key{} });
	}
}

void s_on_my_factories_nationalized::any_value(std::string_view chance, association_type, int32_t event, error_handler& err,
		int32_t line, scenario_building_context& context) {
	int32_t value = parse_int(chance, line, err);
	if(auto it = context.map_of_national_events.find(event); it != context.map_of_national_events.end()) {
		context.state.national_definitions.on_my_factories_nationalized.push_back(
				nations::fixed_event{int16_t(value), it->second.id, dcon::trigger_key{}});
	} else {
		auto id = context.state.world.create_national_event();
		context.map_of_national_events.insert_or_assign(event,
				pending_nat_event{id, trigger::slot_contents::nation, trigger::slot_contents::nation, trigger::slot_contents::nation});
		context.state.national_definitions.on_my_factories_nationalized.push_back(
				nations::fixed_event{int16_t(value), id, dcon::trigger_key{}});
	}
}

void rebel_gov_list::any_value(std::string_view from_gov, association_type, std::string_view to_gov, error_handler& err,
		int32_t line, rebel_context& context) {
	if(auto frit = context.outer_context.map_of_governments.find(std::string(from_gov)); frit != context.outer_context.map_of_governments.end()) {
		if(auto toit = context.outer_context.map_of_governments.find(std::string(to_gov)); toit != context.outer_context.map_of_governments.end()) {
			context.outer_context.state.world.rebel_type_set_government_change(context.id, frit->second, toit->second);
		} else {
			err.accumulated_errors +=
					"Invalid government " + std::string(to_gov) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	} else {
		err.accumulated_errors +=
				"Invalid government " + std::string(from_gov) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void rebel_body::icon(association_type, int32_t value, error_handler& err, int32_t line, rebel_context& context) {
	context.outer_context.state.world.rebel_type_set_icon(context.id, uint8_t(value));
}

void rebel_body::break_alliance_on_win(association_type, bool value, error_handler& err, int32_t line, rebel_context& context) {
	context.outer_context.state.world.rebel_type_set_break_alliance_on_win(context.id, value);
}

void rebel_body::area(association_type, std::string_view value, error_handler& err, int32_t line, rebel_context& context) {
	if(is_fixed_token_ci(value.data(), value.data() + value.length(), "none"))
		context.outer_context.state.world.rebel_type_set_area(context.id, uint8_t(::culture::rebel_area::none));
	else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "nation"))
		context.outer_context.state.world.rebel_type_set_area(context.id, uint8_t(::culture::rebel_area::nation));
	else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "culture"))
		context.outer_context.state.world.rebel_type_set_area(context.id, uint8_t(::culture::rebel_area::culture));
	else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "nation_culture"))
		context.outer_context.state.world.rebel_type_set_area(context.id, uint8_t(::culture::rebel_area::nation_culture));
	else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "nation_religion"))
		context.outer_context.state.world.rebel_type_set_area(context.id, uint8_t(::culture::rebel_area::nation_religion));
	else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "religion"))
		context.outer_context.state.world.rebel_type_set_area(context.id, uint8_t(::culture::rebel_area::religion));
	else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "culture_group"))
		context.outer_context.state.world.rebel_type_set_area(context.id, uint8_t(::culture::rebel_area::culture_group));
	else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "all"))
		context.outer_context.state.world.rebel_type_set_area(context.id, uint8_t(::culture::rebel_area::all));
	else {
		err.accumulated_errors +=
				"Invalid rebel area " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void rebel_body::defection(association_type, std::string_view value, error_handler& err, int32_t line, rebel_context& context) {
	if(is_fixed_token_ci(value.data(), value.data() + value.length(), "none"))
		context.outer_context.state.world.rebel_type_set_defection(context.id, uint8_t(::culture::rebel_defection::none));
	else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "ideology"))
		context.outer_context.state.world.rebel_type_set_defection(context.id, uint8_t(::culture::rebel_defection::ideology));
	else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "culture"))
		context.outer_context.state.world.rebel_type_set_defection(context.id, uint8_t(::culture::rebel_defection::culture));
	else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "any"))
		context.outer_context.state.world.rebel_type_set_defection(context.id, uint8_t(::culture::rebel_defection::any));
	else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "pan_nationalist"))
		context.outer_context.state.world.rebel_type_set_defection(context.id, uint8_t(::culture::rebel_defection::pan_nationalist));
	else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "religion"))
		context.outer_context.state.world.rebel_type_set_defection(context.id, uint8_t(::culture::rebel_defection::religion));
	else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "culture_group"))
		context.outer_context.state.world.rebel_type_set_defection(context.id, uint8_t(::culture::rebel_defection::culture_group));
	else {
		err.accumulated_errors +=
				"Invalid rebel defection " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void rebel_body::independence(association_type, std::string_view value, error_handler& err, int32_t line,
		rebel_context& context) {
	if(is_fixed_token_ci(value.data(), value.data() + value.length(), "none"))
		context.outer_context.state.world.rebel_type_set_independence(context.id, uint8_t(::culture::rebel_independence::none));
	else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "colonial"))
		context.outer_context.state.world.rebel_type_set_independence(context.id, uint8_t(::culture::rebel_independence::colonial));
	else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "culture"))
		context.outer_context.state.world.rebel_type_set_independence(context.id, uint8_t(::culture::rebel_independence::culture));
	else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "any"))
		context.outer_context.state.world.rebel_type_set_independence(context.id, uint8_t(::culture::rebel_independence::any));
	else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "pan_nationalist"))
		context.outer_context.state.world.rebel_type_set_independence(context.id,
				uint8_t(::culture::rebel_independence::pan_nationalist));
	else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "religion"))
		context.outer_context.state.world.rebel_type_set_independence(context.id, uint8_t(::culture::rebel_independence::religion));
	else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "culture_group"))
		context.outer_context.state.world.rebel_type_set_independence(context.id,
				uint8_t(::culture::rebel_independence::culture_group));
	else {
		err.accumulated_errors +=
				"Invalid rebel independence " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void rebel_body::defect_delay(association_type, int32_t value, error_handler& err, int32_t line, rebel_context& context) {
	context.outer_context.state.world.rebel_type_set_defect_delay(context.id, uint8_t(value));
}

void rebel_body::ideology(association_type, std::string_view value, error_handler& err, int32_t line, rebel_context& context) {
	if(auto it = context.outer_context.map_of_ideologies.find(std::string(value));
			it != context.outer_context.map_of_ideologies.end()) {
		context.outer_context.state.world.rebel_type_set_ideology(context.id, it->second.id);
	} else {
		err.accumulated_errors +=
				"Invalid ideology " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void rebel_body::allow_all_cultures(association_type, bool value, error_handler& err, int32_t line, rebel_context& context) {
	context.outer_context.state.world.rebel_type_set_culture_restriction(context.id, !value);
}
void rebel_body::allow_all_ideologies(association_type, bool value, error_handler& err, int32_t line, rebel_context& context) {
	context.outer_context.state.world.rebel_type_set_ideology_restriction(context.id, !value);
}
void rebel_body::allow_all_culture_groups(association_type, bool value, error_handler& err, int32_t line,
		rebel_context& context) {
	context.outer_context.state.world.rebel_type_set_culture_group_restriction(context.id, !value);
}

void rebel_body::occupation_mult(association_type, float value, error_handler& err, int32_t line, rebel_context& context) {
	context.outer_context.state.world.rebel_type_set_occupation_multiplier(context.id, uint8_t(value));
}

void rebel_body::will_rise(dcon::value_modifier_key value, error_handler& err, int32_t line, rebel_context& context) {
	context.outer_context.state.world.rebel_type_set_will_rise(context.id, value);
}

void rebel_body::spawn_chance(dcon::value_modifier_key value, error_handler& err, int32_t line, rebel_context& context) {
	context.outer_context.state.world.rebel_type_set_spawn_chance(context.id, value);
}

void rebel_body::movement_evaluation(dcon::value_modifier_key value, error_handler& err, int32_t line, rebel_context& context) {
	context.outer_context.state.world.rebel_type_set_movement_evaluation(context.id, value);
}

void rebel_body::siege_won_trigger(dcon::trigger_key value, error_handler& err, int32_t line, rebel_context& context) {
	context.outer_context.state.world.rebel_type_set_siege_won_trigger(context.id, value);
}

void rebel_body::demands_enforced_trigger(dcon::trigger_key value, error_handler& err, int32_t line, rebel_context& context) {
	context.outer_context.state.world.rebel_type_set_demands_enforced_trigger(context.id, value);
}

void rebel_body::siege_won_effect(dcon::effect_key value, error_handler& err, int32_t line, rebel_context& context) {
	context.outer_context.state.world.rebel_type_set_siege_won_effect(context.id, value);
}

void rebel_body::demands_enforced_effect(dcon::effect_key value, error_handler& err, int32_t line, rebel_context& context) {
	context.outer_context.state.world.rebel_type_set_demands_enforced_effect(context.id, value);
}

void decision::potential(dcon::trigger_key value, error_handler& err, int32_t line, decision_context& context) {
	context.outer_context.state.world.decision_set_potential(context.id, value);
	if(!value) {
		err.accumulated_warnings += "Empty potential for decision is implicit already (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void decision::allow(dcon::trigger_key value, error_handler& err, int32_t line, decision_context& context) {
	context.outer_context.state.world.decision_set_allow(context.id, value);
	if(!value) {
		err.accumulated_warnings += "Empty allow for decision is implicit already (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void decision::effect(dcon::effect_key value, error_handler& err, int32_t line, decision_context& context) {
	context.outer_context.state.world.decision_set_effect(context.id, value);
	if(!value) {
		err.accumulated_warnings += "Empty effect for decision is implicit already (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void decision::ai_will_do(dcon::value_modifier_key value, error_handler& err, int32_t line, decision_context& context) {
	context.outer_context.state.world.decision_set_ai_will_do(context.id, value);
	if(!value) {
		err.accumulated_warnings += "Empty ai_will_do for decision is implicit already (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void decision::finish(decision_context& context) {
	// "always = yes" assumed when no allow is specified
}

void decision::picture(association_type, std::string_view value, error_handler& err, int32_t line, decision_context& context) {
	auto root = get_root(context.outer_context.state.common_fs);
	auto gfx = open_directory(root, NATIVE("gfx"));
	auto pictures = open_directory(gfx, NATIVE("pictures"));
	auto decisions = open_directory(pictures, NATIVE("decisions"));
	if(!peek_file(decisions, simple_fs::utf8_to_native(value) + NATIVE(".dds")).has_value()
	&& !peek_file(decisions, simple_fs::utf8_to_native(value) + NATIVE(".tga")).has_value()
	&& !peek_file(decisions, simple_fs::utf8_to_native(value) + NATIVE(".png")).has_value()) {
		err.accumulated_warnings += "Picture " + std::string(value) + " does not exist " + " (" + err.file_name + ")\n";
		return; // Picture not found
	}

	std::string file_name = simple_fs::remove_double_backslashes(std::string("gfx\\pictures\\decisions\\") + std::string(value) + ".tga");
	if(auto it = context.outer_context.gfx_context.map_of_names.find(file_name); it != context.outer_context.gfx_context.map_of_names.end()) {
		context.outer_context.state.world.decision_set_image(context.id, it->second);
	} else {
		auto gfxindex = context.outer_context.state.ui_defs.gfx.size();
		context.outer_context.state.ui_defs.gfx.emplace_back();
		ui::gfx_object& new_obj = context.outer_context.state.ui_defs.gfx.back();
		auto new_id = dcon::gfx_object_id(uint16_t(gfxindex));

		context.outer_context.gfx_context.map_of_names.insert_or_assign(file_name, new_id);

		new_obj.number_of_frames = uint8_t(1);

		if(auto itb = context.outer_context.gfx_context.map_of_texture_names.find(file_name);
				itb != context.outer_context.gfx_context.map_of_texture_names.end()) {
			new_obj.primary_texture_handle = itb->second;
		} else {
			auto index = context.outer_context.state.ui_defs.textures.size();
			context.outer_context.state.ui_defs.textures.emplace_back(context.outer_context.state.add_key_win1252(file_name));
			new_obj.primary_texture_handle = dcon::texture_id(uint16_t(index));
			context.outer_context.gfx_context.map_of_texture_names.insert_or_assign(file_name, dcon::texture_id(uint16_t(index)));
		}
		new_obj.flags |= uint8_t(ui::object_type::generic_sprite);

		context.outer_context.state.world.decision_set_image(context.id, new_id);
	}
}

void oob_leader::name(association_type, std::string_view value, error_handler& err, int32_t line, oob_file_context& context) {
	name_ = context.outer_context.state.add_unit_name(value);
}

void oob_leader::date(association_type, sys::year_month_day value, error_handler& err, int32_t line, oob_file_context& context) {
	date_ = sys::date(value, context.outer_context.state.start_date);
}

void oob_army::name(association_type, std::string_view value, error_handler& err, int32_t line, oob_file_army_context& context) {
	context.outer_context.state.world.army_set_name(context.id, context.outer_context.state.add_unit_name(value));
}

void oob_army::location(association_type, int32_t value, error_handler& err, int32_t line, oob_file_army_context& context) {
	if(size_t(value) >= context.outer_context.original_id_to_prov_id_map.size()) {
		err.accumulated_errors +=
				"Province id " + std::to_string(value) + " is too large (" + err.file_name + " line " + std::to_string(line) + ")\n";
	} else {
		auto province_id = context.outer_context.original_id_to_prov_id_map[value];
		context.outer_context.state.world.force_create_army_location(context.id, province_id);
	}
}

void oob_army::leader(oob_leader const& value, error_handler& err, int32_t line, oob_file_army_context& context) {
	if(value.is_general) {
		auto l_id = fatten(context.outer_context.state.world, context.outer_context.state.world.create_leader());
		l_id.set_background(value.background_);
		l_id.set_personality(value.personality_);
		l_id.set_prestige(value.prestige);
		l_id.set_since(value.date_);
		l_id.set_name(value.name_);
		l_id.set_is_admiral(false);
		context.outer_context.state.world.force_create_leader_loyalty(context.nation_for, l_id);
		context.outer_context.state.world.force_create_army_leadership(context.id, l_id);
	} else {
		err.accumulated_errors += "Cannot attach an admiral to an army (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void oob_navy::name(association_type, std::string_view value, error_handler& err, int32_t line, oob_file_navy_context& context) {
	context.outer_context.state.world.navy_set_name(context.id, context.outer_context.state.add_unit_name(value));
}

void oob_navy::location(association_type, int32_t value, error_handler& err, int32_t line, oob_file_navy_context& context) {
	if(size_t(value) >= context.outer_context.original_id_to_prov_id_map.size()) {
		err.accumulated_errors +=
				"Province id " + std::to_string(value) + " is too large (" + err.file_name + " line " + std::to_string(line) + ")\n";
	} else {
		auto province_id = context.outer_context.original_id_to_prov_id_map[value];
		context.outer_context.state.world.force_create_navy_location(context.id, province_id);
	}
}
void oob_navy::leader(oob_leader const& value, error_handler& err, int32_t line, oob_file_navy_context& context) {
	if(!value.is_general) {
		auto l_id = fatten(context.outer_context.state.world, context.outer_context.state.world.create_leader());
		l_id.set_background(value.background_);
		l_id.set_personality(value.personality_);
		l_id.set_prestige(value.prestige);
		l_id.set_since(value.date_);
		l_id.set_name(value.name_);
		l_id.set_is_admiral(false);
		context.outer_context.state.world.force_create_leader_loyalty(context.nation_for, l_id);
		context.outer_context.state.world.force_create_navy_leadership(context.id, l_id);
	} else {
		err.accumulated_errors += "Cannot attach an general to a navy (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void oob_ship::name(association_type, std::string_view value, error_handler& err, int32_t line, oob_file_ship_context& context) {
	context.outer_context.state.world.ship_set_name(context.id, context.outer_context.state.add_unit_name(value));
}

void oob_ship::type(association_type, std::string_view value, error_handler& err, int32_t line, oob_file_ship_context& context) {
	if(auto it = context.outer_context.map_of_unit_types.find(std::string(value));
			it != context.outer_context.map_of_unit_types.end()) {
		context.outer_context.state.world.ship_set_type(context.id, it->second);
	} else {
		err.accumulated_errors +=
				"Invalid unit type " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void oob_regiment::name(association_type, std::string_view value, error_handler& err, int32_t line,
		oob_file_regiment_context& context) {
	context.outer_context.state.world.regiment_set_name(context.id, context.outer_context.state.add_unit_name(value));
}

void oob_regiment::type(association_type, std::string_view value, error_handler& err, int32_t line,
		oob_file_regiment_context& context) {
	if(auto it = context.outer_context.map_of_unit_types.find(std::string(value));
			it != context.outer_context.map_of_unit_types.end()) {
		context.outer_context.state.world.regiment_set_type(context.id, it->second);
	} else {
		err.accumulated_errors +=
				"Invalid unit type " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void oob_regiment::home(association_type, int32_t value, error_handler& err, int32_t line, oob_file_regiment_context& context) {
	if(size_t(value) >= context.outer_context.original_id_to_prov_id_map.size()) {
		err.accumulated_errors +=
				"Province id " + std::to_string(value) + " is too large (" + err.file_name + " line " + std::to_string(line) + ")\n";
	} else {
		auto province_id = context.outer_context.original_id_to_prov_id_map[value];
		for(auto pl : context.outer_context.state.world.province_get_pop_location(province_id)) {
			auto p = pl.get_pop();
			if(p.get_poptype() == context.outer_context.state.culture_definitions.soldiers) {
				context.outer_context.state.world.force_create_regiment_source(context.id, p);
				return;
			}
		}
		err.accumulated_warnings +=
				"No soldiers in province regiment comes from (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void oob_relationship::value(association_type, int32_t v, error_handler& err, int32_t line, oob_file_relation_context& context) {
	if(v < -200 || v > 200) {
		err.accumulated_warnings += "Relation value " + std::to_string(v) + " is not between [-200,-200] (" + err.file_name + " line " + std::to_string(line) + ")\n";
		v = std::clamp(v, -200, 200);
	}

	auto rel =
			context.outer_context.state.world.get_diplomatic_relation_by_diplomatic_pair(context.nation_for, context.nation_with);
	if(rel) {
		context.outer_context.state.world.diplomatic_relation_set_value(rel, float(v));
	} else if(v != 0) {
		auto new_rel = context.outer_context.state.world.force_create_diplomatic_relation(context.nation_for, context.nation_with);
		context.outer_context.state.world.diplomatic_relation_set_value(new_rel, float(v));
	}
}

void oob_relationship::level(association_type, int32_t v, error_handler& err, int32_t line, oob_file_relation_context& context) {
	auto rel = context.outer_context.state.world.get_gp_relationship_by_gp_influence_pair(context.nation_with, context.nation_for);
	auto status_level = [&]() {
		switch(v) {
		case 0:
			return nations::influence::level_hostile;
		case 1:
			return nations::influence::level_opposed;
		case 2:
			return nations::influence::level_neutral;
		case 3:
			return nations::influence::level_cordial;
		case 4:
			return nations::influence::level_friendly;
		case 5:
			return nations::influence::level_in_sphere;
		default:
			return nations::influence::level_neutral;
		}
	}();
	if(rel) {
		context.outer_context.state.world.gp_relationship_set_status(rel, status_level);
	} else if(status_level != nations::influence::level_neutral) {
		auto new_rel = context.outer_context.state.world.force_create_gp_relationship(context.nation_with, context.nation_for);
		context.outer_context.state.world.gp_relationship_set_status(new_rel, status_level);
	}
	if(uint32_t(v) > 5) {
		err.accumulated_warnings += "Influence level " + std::to_string(v) + " defaults to 'neutral' (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void oob_relationship::influence_value(association_type, float v, error_handler& err, int32_t line, oob_file_relation_context& context) {
	auto rel = context.outer_context.state.world.get_gp_relationship_by_gp_influence_pair(context.nation_with, context.nation_for);
	if(rel) {
		context.outer_context.state.world.gp_relationship_set_influence(rel, v);
	} else if(v != 0) {
		auto new_rel = context.outer_context.state.world.force_create_gp_relationship(context.nation_with, context.nation_for);
		context.outer_context.state.world.gp_relationship_set_influence(new_rel, v);
	}
	if(v < 0.f || v > 100.f) {
		err.accumulated_warnings += "Influence value " + std::to_string(v) + " is not between [0,100] (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void oob_relationship::truce_until(association_type, sys::year_month_day v, error_handler& err, int32_t line, oob_file_relation_context& context) {
	auto rel = context.outer_context.state.world.get_diplomatic_relation_by_diplomatic_pair(context.nation_with, context.nation_for);
	if(rel) {
		context.outer_context.state.world.diplomatic_relation_set_truce_until(rel, sys::date(v, context.outer_context.state.start_date));
	} else {
		auto new_rel = context.outer_context.state.world.force_create_diplomatic_relation(context.nation_with, context.nation_for);
		context.outer_context.state.world.diplomatic_relation_set_truce_until(new_rel, sys::date(v, context.outer_context.state.start_date));
	}
}

void oob_file::leader(oob_leader const& value, error_handler& err, int32_t line, oob_file_context& context) {
	if(value.is_general) {
		auto l_id = fatten(context.outer_context.state.world, context.outer_context.state.world.create_leader());
		l_id.set_background(value.background_);
		l_id.set_personality(value.personality_);
		l_id.set_prestige(value.prestige);
		l_id.set_since(value.date_);
		l_id.set_name(value.name_);
		l_id.set_is_admiral(false);
		context.outer_context.state.world.force_create_leader_loyalty(context.nation_for, l_id);
	} else {
		auto l_id = fatten(context.outer_context.state.world, context.outer_context.state.world.create_leader());
		l_id.set_background(value.background_);
		l_id.set_personality(value.personality_);
		l_id.set_prestige(value.prestige);
		l_id.set_since(value.date_);
		l_id.set_name(value.name_);
		l_id.set_is_admiral(true);
		context.outer_context.state.world.force_create_leader_loyalty(context.nation_for, l_id);
	}
}

void production_employee::poptype(association_type, std::string_view v, error_handler& err, int32_t line, production_context& context) {
	if(is_fixed_token_ci(v.data(), v.data() + v.length(), "artisan")) {
		type = context.outer_context.state.culture_definitions.artisans;
	} else if(auto it = context.outer_context.map_of_poptypes.find(std::string(v)); it != context.outer_context.map_of_poptypes.end()) {
		type = it->second;
	} else {
		err.accumulated_errors += "Invalid pop type " + std::string(v) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void alliance::first(association_type, std::string_view tag, error_handler& err, int32_t line, scenario_building_context& context) {
	if(tag.length() == 3) {
		if(auto it = context.map_of_ident_names.find(nations::tag_to_int(tag[0], tag[1], tag[2])); it != context.map_of_ident_names.end()) {
			first_ = context.state.world.national_identity_get_nation_from_identity_holder(it->second);
		} else {
			err.accumulated_errors +=
					"invalid tag " + std::string(tag) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	} else {
		err.accumulated_errors +=
				"invalid tag " + std::string(tag) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void alliance::second(association_type, std::string_view tag, error_handler& err, int32_t line, scenario_building_context& context) {
	if(tag.length() == 3) {
		if(auto it = context.map_of_ident_names.find(nations::tag_to_int(tag[0], tag[1], tag[2])); it != context.map_of_ident_names.end()) {
			second_ = context.state.world.national_identity_get_nation_from_identity_holder(it->second);
		} else {
			err.accumulated_errors +=
					"invalid tag " + std::string(tag) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	} else {
		err.accumulated_errors +=
				"invalid tag " + std::string(tag) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}
void alliance::start_date(association_type, sys::year_month_day ymd, error_handler& err, int32_t line, scenario_building_context& context) {
	if(sys::absolute_time_point(context.state.current_date.to_ymd(context.state.start_date)) < sys::absolute_time_point(ymd))
		invalid = true;
}
void alliance::end_date(association_type, sys::year_month_day ymd, error_handler& err, int32_t line, scenario_building_context& context) {
	if(sys::absolute_time_point(ymd) <= sys::absolute_time_point(context.state.current_date.to_ymd(context.state.start_date)))
		invalid = true;
}

void vassal_description::first(association_type, std::string_view tag, error_handler& err, int32_t line, scenario_building_context& context) {
	if(tag.length() == 3) {
		if(auto it = context.map_of_ident_names.find(nations::tag_to_int(tag[0], tag[1], tag[2])); it != context.map_of_ident_names.end()) {
			first_ = context.state.world.national_identity_get_nation_from_identity_holder(it->second);
		} else {
			err.accumulated_errors +=
					"invalid tag " + std::string(tag) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	} else {
		err.accumulated_errors +=
				"invalid tag " + std::string(tag) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void vassal_description::second(association_type, std::string_view tag, error_handler& err, int32_t line, scenario_building_context& context) {
	if(tag.length() == 3) {
		if(auto it = context.map_of_ident_names.find(nations::tag_to_int(tag[0], tag[1], tag[2])); it != context.map_of_ident_names.end()) {
			second_ = context.state.world.national_identity_get_nation_from_identity_holder(it->second);
		} else {
			err.accumulated_errors +=
					"invalid tag " + std::string(tag) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	} else {
		err.accumulated_errors +=
				"invalid tag " + std::string(tag) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void vassal_description::start_date(association_type, sys::year_month_day ymd, error_handler& err, int32_t line, scenario_building_context& context) {
	if(sys::absolute_time_point(context.state.current_date.to_ymd(context.state.start_date)) < sys::absolute_time_point(ymd))
		invalid = true;
}
void vassal_description::end_date(association_type, sys::year_month_day ymd, error_handler& err, int32_t line, scenario_building_context& context) {
	if(sys::absolute_time_point(ymd) <= sys::absolute_time_point(context.state.current_date.to_ymd(context.state.start_date)))
		invalid = true;
}

void govt_flag_block::flag(association_type, std::string_view value, error_handler& err, int32_t line, country_history_context& context) {
	if(auto it = context.outer_context.map_of_governments.find(std::string(value)); it != context.outer_context.map_of_governments.end()) {
		flag_ = context.outer_context.state.world.government_type_get_flag(it->second);
	} else {
		err.accumulated_errors += "invalid government type " + std::string(value) + " encountered  (" + err.file_name + " line " +
															std::to_string(line) + ")\n";
	}
}

void upper_house_block::any_value(std::string_view value, association_type, float v, error_handler& err, int32_t line, country_history_context& context) {
	if(!context.holder_id)
		return;

	if(auto it = context.outer_context.map_of_ideologies.find(std::string(value)); it != context.outer_context.map_of_ideologies.end()) {
		context.outer_context.state.world.nation_set_upper_house(context.holder_id, it->second.id, v);
	} else {
		err.accumulated_errors +=
				"invalid ideology " + std::string(value) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void foreign_investment_block::any_value(std::string_view tag, association_type, float v, error_handler& err, int32_t line, country_history_context& context) {
	if(!context.holder_id)
		return;

	if(tag.length() == 3) {
		if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(tag[0], tag[1], tag[2]));
				it != context.outer_context.map_of_ident_names.end()) {
			auto other = context.outer_context.state.world.national_identity_get_nation_from_identity_holder(it->second);
			auto rel_id = context.outer_context.state.world.force_create_unilateral_relationship(other, context.holder_id);
			context.outer_context.state.world.unilateral_relationship_set_foreign_investment(rel_id, v);
		} else {
			err.accumulated_errors +=
					"invalid tag " + std::string(tag) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	} else {
		err.accumulated_errors +=
				"invalid tag " + std::string(tag) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void country_history_file::set_country_flag(association_type, std::string_view value, error_handler& err, int32_t line, country_history_context& context) {
	if(!context.holder_id)
		return;
	if(auto it = context.outer_context.map_of_national_flags.find(std::string(value)); it != context.outer_context.map_of_national_flags.end()) {
		context.outer_context.state.world.nation_set_flag_variables(context.holder_id, it->second, true);
	} else {
		// unused flag variable: ignore
	}
}

void country_history_file::set_global_flag(association_type, std::string_view value, error_handler& err, int32_t line, country_history_context& context) {
	if(!context.holder_id)
		return;
	if(auto it = context.outer_context.map_of_global_flags.find(std::string(value)); it != context.outer_context.map_of_global_flags.end()) {
		context.outer_context.state.national_definitions.set_global_flag_variable(it->second, true);
	} else {
		// unused flag variable: ignore
	}
}

void country_history_file::colonial_points(association_type, int32_t value, error_handler& err, int32_t line, country_history_context& context) {
	context.outer_context.state.world.nation_set_permanent_colonial_points(context.holder_id, int16_t(value));
}

void country_history_file::capital(association_type, int32_t value, error_handler& err, int32_t line,
		country_history_context& context) {
	if(size_t(value) >= context.outer_context.original_id_to_prov_id_map.size()) {
		err.accumulated_errors += "Province id " + std::to_string(value) + " is too large (" + err.file_name + " line " + std::to_string(line) + ")\n";
	} else {
		auto province_id = context.outer_context.original_id_to_prov_id_map[value];
		if(!context.in_dated_block)
			context.outer_context.state.world.national_identity_set_capital(context.nat_ident, province_id);
		if(context.holder_id)
			context.outer_context.state.world.nation_set_capital(context.holder_id, province_id);
	}
}

void country_history_file::any_value(std::string_view label, association_type, std::string_view value, error_handler& err,
		int32_t line, country_history_context& context) {
	if(!context.holder_id)
		return;

	std::string str_label(label);
	if(auto it = context.outer_context.map_of_technologies.find(str_label); it != context.outer_context.map_of_technologies.end()) {
		auto v = parse_bool(value, line, err);
		context.outer_context.state.world.nation_set_active_technologies(context.holder_id, it->second.id, v);
	} else if(auto itb = context.outer_context.map_of_inventions.find(str_label);
						itb != context.outer_context.map_of_inventions.end()) {
		auto v = parse_bool(value, line, err);
		context.outer_context.state.world.nation_set_active_inventions(context.holder_id, itb->second.id, v);
	} else if(auto itc = context.outer_context.map_of_iissues.find(str_label); itc != context.outer_context.map_of_iissues.end()) {
		if(auto itd = context.outer_context.map_of_ioptions.find(std::string(value));
				itd != context.outer_context.map_of_ioptions.end()) {
			context.outer_context.state.world.nation_set_issues(context.holder_id, itc->second, itd->second.id);
		} else {
			err.accumulated_errors += "invalid issue option name " + std::string(value) + " encountered  (" + err.file_name + " line " +
																std::to_string(line) + ")\n";
		}
	} else if(auto ite = context.outer_context.map_of_reforms.find(str_label); ite != context.outer_context.map_of_reforms.end()) {
		if(auto itd = context.outer_context.map_of_roptions.find(std::string(value));
				itd != context.outer_context.map_of_roptions.end()) {
			context.outer_context.state.world.nation_set_reforms(context.holder_id, ite->second, itd->second.id);
		} else {
			err.accumulated_errors += "invalid reform option name " + std::string(value) + " encountered  (" + err.file_name +
																" line " + std::to_string(line) + ")\n";
		}
	} else {
		err.accumulated_errors +=
				"invalid key " + str_label + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void country_history_file::primary_culture(association_type, std::string_view value, error_handler& err, int32_t line,
		country_history_context& context) {
	if(auto it = context.outer_context.map_of_culture_names.find(std::string(value));
			it != context.outer_context.map_of_culture_names.end()) {
		if(!context.in_dated_block)
			context.outer_context.state.world.national_identity_set_primary_culture(context.nat_ident, it->second);
		if(context.holder_id)
			context.outer_context.state.world.nation_set_primary_culture(context.holder_id, it->second);
	} else {
		err.accumulated_errors +=
				"invalid culture " + std::string(value) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void country_history_file::culture(association_type, std::string_view value, error_handler& err, int32_t line,
		country_history_context& context) {
	if(!context.holder_id)
		return;

	if(auto it = context.outer_context.map_of_culture_names.find(std::string(value));
			it != context.outer_context.map_of_culture_names.end()) {
		context.outer_context.state.world.nation_set_accepted_cultures(context.holder_id, it->second, true);
	} else {
		err.accumulated_errors +=
				"invalid culture " + std::string(value) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void country_history_file::remove_culture(association_type, std::string_view value, error_handler& err, int32_t line,
		country_history_context& context) {
	if(!context.holder_id)
		return;

	if(auto it = context.outer_context.map_of_culture_names.find(std::string(value));
			it != context.outer_context.map_of_culture_names.end()) {
		context.outer_context.state.world.nation_set_accepted_cultures(context.holder_id, it->second, false);
	} else {
		err.accumulated_errors +=
			"invalid culture " + std::string(value) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void country_history_file::religion(association_type, std::string_view value, error_handler& err, int32_t line,
		country_history_context& context) {
	if(auto it = context.outer_context.map_of_religion_names.find(std::string(value)); it != context.outer_context.map_of_religion_names.end()) {
		if(!context.in_dated_block)
			context.outer_context.state.world.national_identity_set_religion(context.nat_ident, it->second);
		if(context.holder_id)
			context.outer_context.state.world.nation_set_religion(context.holder_id, it->second);
	} else {
		err.accumulated_errors +=
				"invalid religion " + std::string(value) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void country_history_file::government(association_type, std::string_view value, error_handler& err, int32_t line,
		country_history_context& context) {
	if(!context.holder_id)
		return;

	if(auto it = context.outer_context.map_of_governments.find(std::string(value));
			it != context.outer_context.map_of_governments.end()) {
		context.outer_context.state.world.nation_set_government_type(context.holder_id, it->second);
	} else {
		err.accumulated_errors += "invalid government type " + std::string(value) + " encountered  (" + err.file_name + " line " +
															std::to_string(line) + ")\n";
	}
}

void country_history_file::plurality(association_type, float value, error_handler& err, int32_t line,
		country_history_context& context) {
	if(!context.holder_id)
		return;
	context.outer_context.state.world.nation_set_plurality(context.holder_id, value);
}

void country_history_file::prestige(association_type, float value, error_handler& err, int32_t line,
		country_history_context& context) {
	if(!context.holder_id)
		return;
	context.outer_context.state.world.nation_set_prestige(context.holder_id, value);
}

void country_history_file::nationalvalue(association_type, std::string_view value, error_handler& err, int32_t line,
		country_history_context& context) {
	if(!context.holder_id)
		return;

	if(auto it = context.outer_context.map_of_modifiers.find(std::string(value));
			it != context.outer_context.map_of_modifiers.end()) {
		context.outer_context.state.world.nation_set_national_value(context.holder_id, it->second);
	} else {
		err.accumulated_errors +=
				"invalid modifier " + std::string(value) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void country_history_file::schools(association_type, std::string_view value, error_handler& err, int32_t line,
		country_history_context& context) {
	if(!context.holder_id)
		return;

	if(auto it = context.outer_context.map_of_modifiers.find(std::string(value));
			it != context.outer_context.map_of_modifiers.end()) {
		context.outer_context.state.world.nation_set_tech_school(context.holder_id, it->second);
	} else {
		err.accumulated_errors +=
				"invalid modifier " + std::string(value) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void country_history_file::civilized(association_type, bool value, error_handler& err, int32_t line,
		country_history_context& context) {
	if(!context.holder_id)
		return;
	context.outer_context.state.world.nation_set_is_civilized(context.holder_id, value);
}

void country_history_file::is_releasable_vassal(association_type, bool value, error_handler& err, int32_t line,
		country_history_context& context) {
	context.outer_context.state.world.national_identity_set_is_not_releasable(context.nat_ident, !value);
}

void country_history_file::literacy(association_type, float value, error_handler& err, int32_t line,
		country_history_context& context) {
	if(!context.holder_id)
		return;
	auto fh = fatten(context.outer_context.state.world, context.holder_id);
	for(auto owned_prov : context.outer_context.state.world.nation_get_province_ownership(context.holder_id)) {
		for(auto prov_pop : owned_prov.get_province().get_pop_location()) {
			bool accepted = [&]() {
				if(prov_pop.get_pop().get_culture() == fh.get_primary_culture())
					return true;
				if(fh.get_accepted_cultures(prov_pop.get_pop().get_culture()))
					return true;
				return false;
			}();
			if(accepted) {
				pop_demographics::set_literacy(context.outer_context.state, prov_pop.get_pop(), std::clamp(value, 0.0f, 1.0f));
			}
		}
	}
}

void country_history_file::non_state_culture_literacy(association_type, float value, error_handler& err, int32_t line,
		country_history_context& context) {
	if(!context.holder_id)
		return;
	auto fh = fatten(context.outer_context.state.world, context.holder_id);
	for(auto owned_prov : context.outer_context.state.world.nation_get_province_ownership(context.holder_id)) {
		for(auto prov_pop : owned_prov.get_province().get_pop_location()) {
			bool non_accepted = [&]() {
				if(prov_pop.get_pop().get_culture() == fh.get_primary_culture())
					return false;
				if(fh.get_accepted_cultures(prov_pop.get_pop().get_culture()))
					return false;
				return true;
			}();
			if(non_accepted)
				pop_demographics::set_literacy(context.outer_context.state, prov_pop.get_pop(), std::clamp(value, 0.0f, 1.0f));
		}
	}
}

void country_history_file::consciousness(association_type, float value, error_handler& err, int32_t line,
		country_history_context& context) {
	if(!context.holder_id)
		return;
	for(auto owned_prov : context.outer_context.state.world.nation_get_province_ownership(context.holder_id)) {
		for(auto prov_pop : owned_prov.get_province().get_pop_location()) {
			pop_demographics::set_consciousness(context.outer_context.state, prov_pop.get_pop(), std::clamp(value, 0.0f, 10.0f));
		}
	}
}

void country_history_file::nonstate_consciousness(association_type, float value, error_handler& err, int32_t line,
		country_history_context& context) {
	if(!context.holder_id)
		return;
	for(auto owned_prov : context.outer_context.state.world.nation_get_province_ownership(context.holder_id)) {
		if(owned_prov.get_province().get_is_colonial()) {
			for(auto prov_pop : owned_prov.get_province().get_pop_location()) {
				pop_demographics::set_consciousness(context.outer_context.state, prov_pop.get_pop(), std::clamp(value, 0.0f, 10.0f));
			}
		}
	}
}

void country_history_file::govt_flag(govt_flag_block const& value, error_handler& err, int32_t line,
		country_history_context& context) {
	context.outer_context.state.world.national_identity_set_government_flag_type(context.nat_ident, value.government_, value.flag_);
}

void country_history_file::ruling_party(association_type, std::string_view value, error_handler& err, int32_t line,
		country_history_context& context) {
	if(!context.holder_id)
		return;

	auto value_key = context.outer_context.state.lookup_key(value);

	if(value_key) {
		auto first_party = context.outer_context.state.world.national_identity_get_political_party_first(context.nat_ident);
		auto party_count = context.outer_context.state.world.national_identity_get_political_party_count(context.nat_ident);
		for(uint32_t i = 0; i < party_count; ++i) {
			dcon::political_party_id pid{ dcon::political_party_id::value_base_t(first_party.id.index() + i) };
			auto name = context.outer_context.state.world.political_party_get_name(pid);
			if(name == value_key) {
				context.outer_context.state.world.nation_set_ruling_party(context.holder_id, pid);
				for(auto p_issue : context.outer_context.state.culture_definitions.party_issues) {
					context.outer_context.state.world.nation_set_issues(context.holder_id, p_issue,
							context.outer_context.state.world.political_party_get_party_issues(pid, p_issue));
				}
				return;
			}
		}
		// alright, it didn't belong to that nation -- try checking everything to help broken mods work anyways
		err.accumulated_warnings += "invalid political party " + std::string(value) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
		for(auto p : context.outer_context.state.world.in_political_party) {
			auto name = p.get_name();
			if(name == value_key) {
				context.outer_context.state.world.nation_set_ruling_party(context.holder_id, p);
				for(auto p_issue : context.outer_context.state.culture_definitions.party_issues) {
					context.outer_context.state.world.nation_set_issues(context.holder_id, p_issue,
							context.outer_context.state.world.political_party_get_party_issues(p, p_issue));
				}
				return;
			}
		}
	}
	err.accumulated_errors += "globally invalid political party " + std::string(value) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
}

void country_history_file::decision(association_type, std::string_view value, error_handler& err, int32_t line, country_history_context& context) {
	auto value_key = context.outer_context.state.lookup_key(std::string{ value } + "_title");

	if(!value_key) {
		err.accumulated_errors += "no decision named " + std::string(value) + " found  (" + err.file_name + " line " + std::to_string(line) + ")\n";
		return;
	}

	for(auto d : context.outer_context.state.world.in_decision) {
		auto name = d.get_name();
		if(name == value_key) {
			context.pending_decisions.emplace_back(context.holder_id, d);
			return;
		}
	}

	err.accumulated_errors += "no decision named " + std::string(value) + " found  (" + err.file_name + " line " + std::to_string(line) + ")\n";
}

void commodity_array::finish(scenario_building_context& context) {
	data.resize(context.state.world.commodity_size());
}

void country_file::color(color_from_3i cvalue, error_handler& err, int32_t line, country_file_context& context) {
	context.outer_context.state.world.national_identity_set_color(context.id, cvalue.value);
	for(auto g : context.outer_context.state.world.in_government_type) {
		context.outer_context.state.world.national_identity_set_government_color(context.id, g, cvalue.value);
	}
}

void country_file::template_(association_type, std::string_view value, error_handler& err, int32_t line, country_file_context& context) {
	auto root = simple_fs::get_root(context.outer_context.state.common_fs);
	auto common_dir = simple_fs::open_directory(root, NATIVE("common"));
	auto countries_dir = simple_fs::open_directory(common_dir, NATIVE("templates"));
	if(auto f = simple_fs::open_file(countries_dir, simple_fs::utf8_to_native(value)); f) {
		auto content = simple_fs::view_contents(*f);
		err.file_name = std::string(value);
		parsers::token_generator gen(content.data, content.data + content.file_size);
		parsers::parse_country_file(gen, err, context);
	}
}

void country_file::any_group(std::string_view name, color_from_3i c, error_handler& err, int32_t line, country_file_context& context) {
	if(auto it = context.outer_context.map_of_governments.find(std::string(name)); it != context.outer_context.map_of_governments.end()) {
		context.outer_context.state.world.national_identity_set_government_color(context.id, it->second, c.value);
	} else {
		err.accumulated_errors +=
				"Invalid government type " + std::string(name) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void generic_event::title(association_type, std::string_view value, error_handler& err, int32_t line,
		event_building_context& context) {
	title_ = text::find_or_add_key(context.outer_context.state, value, false);
}

void generic_event::desc(association_type, std::string_view value, error_handler& err, int32_t line,
		event_building_context& context) {
	desc_ = text::find_or_add_key(context.outer_context.state, value, false);
}

void generic_event::issue_group(association_type, std::string_view name, error_handler& err, int32_t line, event_building_context& context) {
	if(auto it = context.outer_context.map_of_iissues.find(std::string(name)); it != context.outer_context.map_of_iissues.end()) {
		issue_group_ = it->second;
	} else {
		err.accumulated_errors += "Invalid issue group " + std::string(name) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void generic_event::option(sys::event_option const& value, error_handler& err, int32_t line, event_building_context& context) {
	if(last_option_added < sys::max_event_options) {
		options[last_option_added] = value;
		if(!value.name && !value.effect) {
			options[last_option_added].name = text::find_or_add_key(context.outer_context.state, "alice_option_no_name", true);
			err.accumulated_warnings += "Event with an option with no name (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
		++last_option_added;
	} else {
		err.accumulated_errors += "Event given too many options (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void generic_event::picture(association_type, std::string_view name, error_handler& err, int32_t line, event_building_context& context) {

	auto root = get_root(context.outer_context.state.common_fs);
	auto gfx = open_directory(root, NATIVE("gfx"));
	auto pictures = open_directory(gfx, NATIVE("pictures"));
	auto events = open_directory(pictures, NATIVE("events"));

	std::string file_name = simple_fs::remove_double_backslashes(std::string("gfx\\pictures\\events\\") + [&]() {
		if(peek_file(events, simple_fs::utf8_to_native(name) + NATIVE(".tga"))) {
			return std::string(name) + ".tga";
		} else if(peek_file(events, simple_fs::utf8_to_native(name) + NATIVE(".dds"))) {
			return std::string(name) + ".tga";
		} else if(peek_file(events, simple_fs::utf8_to_native(name) + NATIVE(".png"))) {
			return std::string(name) + ".tga";
		} else {
			return std::string("GFX_event_no_image.tga");
		}
	}());

	if(auto it = context.outer_context.gfx_context.map_of_names.find(file_name);
			it != context.outer_context.gfx_context.map_of_names.end()) {
		picture_ = it->second;
	} else {
		auto gfxindex = context.outer_context.state.ui_defs.gfx.size();
		context.outer_context.state.ui_defs.gfx.emplace_back();
		ui::gfx_object& new_obj = context.outer_context.state.ui_defs.gfx.back();
		auto new_id = dcon::gfx_object_id(uint16_t(gfxindex));

		context.outer_context.gfx_context.map_of_names.insert_or_assign(file_name, new_id);

		new_obj.number_of_frames = uint8_t(1);

		if(auto itb = context.outer_context.gfx_context.map_of_texture_names.find(file_name);
				itb != context.outer_context.gfx_context.map_of_texture_names.end()) {
			new_obj.primary_texture_handle = itb->second;
		} else {
			auto index = context.outer_context.state.ui_defs.textures.size();
			context.outer_context.state.ui_defs.textures.emplace_back(context.outer_context.state.add_key_win1252(file_name));
			new_obj.primary_texture_handle = dcon::texture_id(uint16_t(index));
			context.outer_context.gfx_context.map_of_texture_names.insert_or_assign(file_name, dcon::texture_id(uint16_t(index)));
		}
		new_obj.flags |= uint8_t(ui::object_type::generic_sprite);

		picture_ = new_id;
	}
}

void history_war_goal::receiver(association_type, std::string_view tag, error_handler& err, int32_t line, war_history_context& context) {
	if(tag.length() == 3) {
		if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(tag[0], tag[1], tag[2]));
				it != context.outer_context.map_of_ident_names.end()) {
			receiver_ = context.outer_context.state.world.national_identity_get_nation_from_identity_holder(it->second);
		} else {
			err.accumulated_errors +=
					"invalid tag " + std::string(tag) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	} else {
		err.accumulated_errors +=
				"invalid tag " + std::string(tag) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void history_war_goal::state_province_id(association_type t, int32_t value, error_handler& err, int32_t line, war_history_context& context) {
	if(0 <= value && size_t(value) < context.outer_context.original_id_to_prov_id_map.size()) {
		state_province_id_ = context.outer_context.original_id_to_prov_id_map[value];
	} else {
		err.accumulated_errors +=
			"history wargoal given an invalid province id (" + err.file_name + ", line " + std::to_string(line) + ")\n";
	}
}

void make_leader_images(scenario_building_context& outer_context) {
	auto root = get_root(outer_context.state.common_fs);
	auto gfx = open_directory(root, NATIVE("gfx"));
	auto infa = open_directory(gfx, NATIVE("interface"));
	auto leaders = open_directory(infa, NATIVE("leaders"));

	auto all_images = simple_fs::list_files(leaders, NATIVE(".dds"));
	for(auto i : all_images) {
		auto native_name = simple_fs::get_file_name(i);
		auto uname = simple_fs::native_to_utf8(native_name);

		bool admiral = false;
		std::string group_name;

		auto apos = uname.find("_admiral_", 0);
		if(apos != std::string::npos) {
			admiral = true;
			group_name = uname.substr(0, apos);
		} else {
			auto gpos = uname.find("_general_", 0);
			if(gpos != std::string::npos) {
				group_name = uname.substr(0, gpos);
			} else {
				continue; // neither admiral nor general
			}
		}

		dcon::leader_images_id category;
		if(auto it = outer_context.map_of_leader_graphics.find(group_name); it != outer_context.map_of_leader_graphics.end()) {
			category = it->second;
		} else {
			category = outer_context.state.world.create_leader_images();
			outer_context.map_of_leader_graphics.insert_or_assign(group_name, category);
		}
		
		std::string file_name = simple_fs::remove_double_backslashes(std::string("gfx\\interface\\leaders\\") + uname);

		auto gfxindex = outer_context.state.ui_defs.gfx.size();
		outer_context.state.ui_defs.gfx.emplace_back();
		ui::gfx_object& new_obj = outer_context.state.ui_defs.gfx.back();
		auto new_id = dcon::gfx_object_id(uint16_t(gfxindex));

		outer_context.gfx_context.map_of_names.insert_or_assign(file_name, new_id);

		new_obj.number_of_frames = uint8_t(1);
		if(auto itb = outer_context.gfx_context.map_of_texture_names.find(file_name); itb != outer_context.gfx_context.map_of_texture_names.end()) {
			new_obj.primary_texture_handle = itb->second;
		} else {
			auto index = outer_context.state.ui_defs.textures.size();
			outer_context.state.ui_defs.textures.emplace_back(outer_context.state.add_key_utf8(file_name));
			new_obj.primary_texture_handle = dcon::texture_id(uint16_t(index));
			outer_context.gfx_context.map_of_texture_names.insert_or_assign(file_name, new_obj.primary_texture_handle);
		}
		new_obj.flags |= uint8_t(ui::object_type::generic_sprite);

		if(admiral) {
			outer_context.state.world.leader_images_get_admirals(category).push_back(new_id);
		} else {
			outer_context.state.world.leader_images_get_generals(category).push_back(new_id);
		}
		
	}
}

void history_war_goal::actor(association_type, std::string_view tag, error_handler& err, int32_t line, war_history_context& context) {
	if(tag.length() == 3) {
		if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(tag[0], tag[1], tag[2]));
				it != context.outer_context.map_of_ident_names.end()) {
			actor_ = context.outer_context.state.world.national_identity_get_nation_from_identity_holder(it->second);
		} else {
			err.accumulated_errors +=
					"invalid tag " + std::string(tag) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	} else {
		err.accumulated_errors +=
				"invalid tag " + std::string(tag) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}
void history_war_goal::country(association_type, std::string_view tag, error_handler& err, int32_t line, war_history_context& context) {
	if(tag.length() == 3) {
		if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(tag[0], tag[1], tag[2]));
				it != context.outer_context.map_of_ident_names.end()) {
			secondary_ = context.outer_context.state.world.national_identity_get_nation_from_identity_holder(it->second);
		} else {
			err.accumulated_errors +=
				"invalid tag " + std::string(tag) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	} else {
		err.accumulated_errors +=
			"invalid tag " + std::string(tag) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}
void history_war_goal::casus_belli(association_type, std::string_view value, error_handler& err, int32_t line, war_history_context& context) {
	if(auto it = context.outer_context.map_of_cb_types.find(std::string(value));
			it != context.outer_context.map_of_cb_types.end()) {
		casus_belli_ = it->second.id;
	} else {
		err.accumulated_errors += "invalid cb type type " + std::string(value) + " encountered  (" + err.file_name + " line " +
															std::to_string(line) + ")\n";
	}
}
void war_block::world_war(association_type, bool v, error_handler& err, int32_t line, war_history_context& context) {
	context.great_war = v;
}
void war_block::add_attacker(association_type, std::string_view tag, error_handler& err, int32_t line,
		war_history_context& context) {
	if(tag.length() == 3) {
		if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(tag[0], tag[1], tag[2]));
				it != context.outer_context.map_of_ident_names.end()) {
			auto tg = context.outer_context.state.world.national_identity_get_nation_from_identity_holder(it->second);
			if(tg)
				context.attackers.push_back(tg);
		} else {
			err.accumulated_errors +=
					"invalid tag " + std::string(tag) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	} else {
		err.accumulated_errors +=
				"invalid tag " + std::string(tag) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void war_block::add_defender(association_type, std::string_view tag, error_handler& err, int32_t line,
		war_history_context& context) {
	if(tag.length() == 3) {
		if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(tag[0], tag[1], tag[2]));
				it != context.outer_context.map_of_ident_names.end()) {
			auto tg = context.outer_context.state.world.national_identity_get_nation_from_identity_holder(it->second);
			if(tg)
				context.defenders.push_back(tg);
		} else {
			err.accumulated_errors +=
					"invalid tag " + std::string(tag) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	} else {
		err.accumulated_errors +=
				"invalid tag " + std::string(tag) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void war_block::rem_attacker(association_type, std::string_view tag, error_handler& err, int32_t line,
		war_history_context& context) {
	if(tag.length() == 3) {
		if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(tag[0], tag[1], tag[2]));
				it != context.outer_context.map_of_ident_names.end()) {
			auto tg = context.outer_context.state.world.national_identity_get_nation_from_identity_holder(it->second);
			std::erase(context.attackers, tg);
		} else {
			err.accumulated_errors +=
					"invalid tag " + std::string(tag) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	} else {
		err.accumulated_errors +=
				"invalid tag " + std::string(tag) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}
void war_block::rem_defender(association_type, std::string_view tag, error_handler& err, int32_t line,
		war_history_context& context) {
	if(tag.length() == 3) {
		if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(tag[0], tag[1], tag[2]));
				it != context.outer_context.map_of_ident_names.end()) {
			auto tg = context.outer_context.state.world.national_identity_get_nation_from_identity_holder(it->second);
			std::erase(context.defenders, tg);
		} else {
			err.accumulated_errors +=
					"invalid tag " + std::string(tag) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	} else {
		err.accumulated_errors +=
				"invalid tag " + std::string(tag) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void enter_war_dated_block(std::string_view label, token_generator& gen, error_handler& err, war_history_context& context) {
	auto ymd = parse_date(label, 0, err);
	if(sys::date(ymd, context.outer_context.state.start_date) <= context.outer_context.state.current_date) {
		parse_war_block(gen, err, context);
	} else {
		gen.discard_group();
	}
}

void war_history_file::name(association_type, std::string_view name, error_handler& err, int32_t line,
		war_history_context& context) {
	context.name = std::string(name);
}

void war_history_file::finish(war_history_context& context) {
	if(context.attackers.size() > 0 && context.defenders.size() > 0 && context.wargoals.size() > 0) {
		auto new_war = fatten(context.outer_context.state.world, context.outer_context.state.world.create_war());
		new_war.set_start_date(sys::date(0));
		new_war.set_primary_attacker(context.attackers[0]);
		new_war.set_primary_defender(context.defenders[0]);
		new_war.set_is_great(context.great_war);
		new_war.set_original_target(context.defenders[0]);
		// new_war.set_name(text::find_or_add_key(context.outer_context.state, context.name));

		new_war.set_name(context.outer_context.state.lookup_key(std::string_view{ "agression_war_name" }));// misspelling is intentional; DO NOT CORRECT

		for(auto n : context.attackers) {
			auto rel = context.outer_context.state.world.force_create_war_participant(new_war, n);
			context.outer_context.state.world.war_participant_set_is_attacker(rel, true);
		}
		for(auto n : context.defenders) {
			auto rel = context.outer_context.state.world.force_create_war_participant(new_war, n);
			context.outer_context.state.world.war_participant_set_is_attacker(rel, false);
		}

		// release puppet if subject declares on overlord or vice versa
		for(auto p : context.outer_context.state.world.war_get_war_participant(new_war)) {
			auto ol_rel = context.outer_context.state.world.nation_get_overlord_as_subject(p.get_nation());
			auto ol = context.outer_context.state.world.overlord_get_ruler(ol_rel);

			for(auto p2 : context.outer_context.state.world.war_get_war_participant(new_war)) {
				if(p2.get_nation() == ol && p.get_is_attacker() != p2.get_is_attacker()) {
					nations::release_vassal(context.outer_context.state, ol_rel);
				}
			}
		}

		for(auto& wg : context.wargoals) {
			auto new_wg = fatten(context.outer_context.state.world, context.outer_context.state.world.create_wargoal());
			new_wg.set_added_by(wg.actor_);
			if(wg.actor_ == context.attackers[0]) {
				new_war.set_name(context.outer_context.state.world.cb_type_get_war_name(wg.casus_belli_));
			}
			new_wg.set_target_nation(wg.receiver_);
			new_wg.set_type(wg.casus_belli_);
			new_wg.set_secondary_nation(wg.secondary_);
			new_wg.set_associated_tag(context.outer_context.state.world.nation_get_identity_from_identity_holder(wg.secondary_));
			new_wg.set_associated_state(context.outer_context.state.world.province_get_state_from_abstract_state_membership(wg.state_province_id_));
			context.outer_context.state.world.force_create_wargoals_attached(new_war, new_wg);
		}
	}
}

void mod_file::name(association_type, std::string_view value, error_handler& err, int32_t line, mod_file_context& context) {
	name_ = std::string(value);
}

void mod_file::path(association_type, std::string_view value, error_handler& err, int32_t line, mod_file_context& context) {
	path_ = std::string(value);
}

void mod_file::user_dir(association_type, std::string_view value, error_handler& err, int32_t line, mod_file_context& context) {
	user_dir_ = std::string(value);
}

void mod_file::replace_path(association_type, std::string_view value, error_handler& err, int32_t line,
		mod_file_context& context) {
	replace_paths.push_back(std::string(value));
}

void mod_file::add_to_file_system(simple_fs::file_system& fs){
	// If there isn't any path then we aren't required to do anything
	if(path_.empty())
		return;

	// Add root of mod_path
	for(auto s : replace_paths) {
		auto const replace_path = simple_fs::correct_slashes(simple_fs::utf8_to_native(s));
		if(replace_path == NATIVE("history")) {
			simple_fs::add_ignore_path(fs, simple_fs::list_roots(fs)[0] + NATIVE("\\history\\countries"));
			simple_fs::add_ignore_path(fs, simple_fs::list_roots(fs)[0] + NATIVE("\\history\\diplomacy"));
			simple_fs::add_ignore_path(fs, simple_fs::list_roots(fs)[0] + NATIVE("\\history\\provinces"));
			simple_fs::add_ignore_path(fs, simple_fs::list_roots(fs)[0] + NATIVE("\\history\\units"));
			simple_fs::add_ignore_path(fs, simple_fs::list_roots(fs)[0] + NATIVE("\\history\\wars"));
		} else if(replace_path == NATIVE("history\\pops")
			|| replace_path == NATIVE("map")
			|| replace_path == NATIVE("map\\terrain")) {
			//no
		} else {
			native_string path_block = simple_fs::list_roots(fs)[0];
			path_block += NATIVE_DIR_SEPARATOR;
			path_block += replace_path;
			if(path_block.back() != NATIVE_DIR_SEPARATOR)
				path_block += NATIVE_DIR_SEPARATOR;

			simple_fs::add_ignore_path(fs, path_block);
		}
	}

	native_string mod_path = simple_fs::list_roots(fs)[0];
	mod_path += NATIVE_DIR_SEPARATOR;
	mod_path += simple_fs::correct_slashes(simple_fs::utf8_to_native(path_));
	add_root(fs, mod_path);
}


void locale_parser::body_feature(association_type, std::string_view value, error_handler& err, int32_t line, sys::state&) {
	body_features.push_back(hb_tag_from_string(value.data(), int(value.length())));
}
void locale_parser::header_feature(association_type, std::string_view value, error_handler& err, int32_t line, sys::state&) {
	header_features.push_back(hb_tag_from_string(value.data(), int(value.length())));
}
void locale_parser::map_feature(association_type, std::string_view value, error_handler& err, int32_t line, sys::state&) {
	map_features.push_back(hb_tag_from_string(value.data(), int(value.length())));
}

void add_locale(sys::state& state, std::string_view locale_name, char const* data_start, char const* data_end) {
	parsers::token_generator gen(data_start, data_end);
	parsers::error_handler err("");

	locale_parser new_locale = parsers::parse_locale_parser(gen, err, state);
	hb_language_t lang = nullptr;

	auto new_locale_id = state.world.create_locale();
	auto new_locale_obj = fatten(state.world, new_locale_id);
	new_locale_obj.set_hb_script(hb_script_from_string(new_locale.script.c_str(), int(new_locale.script.length())));
	new_locale_obj.set_native_rtl(new_locale.rtl);
	new_locale_obj.set_prevent_letterspace(new_locale.prevent_map_letterspacing);

	{
		auto f = new_locale_obj.get_body_font();
		f.resize(uint32_t(new_locale.body_font.length()));
		f.load_range((uint8_t const*)new_locale.body_font.c_str(), (uint8_t const*)new_locale.body_font.c_str() + new_locale.body_font.length());
	}
	{
		auto f = new_locale_obj.get_header_font();
		f.resize(uint32_t(new_locale.header_font.length()));
		f.load_range((uint8_t const*)new_locale.header_font.c_str(), (uint8_t const*)new_locale.header_font.c_str() + new_locale.header_font.length());
	}
	{
		auto f = new_locale_obj.get_map_font();
		f.resize(uint32_t(new_locale.map_font.length()));
		f.load_range((uint8_t const*)new_locale.map_font.c_str(), (uint8_t const*)new_locale.map_font.c_str() + new_locale.map_font.length());
	}
	{
		auto f = new_locale_obj.get_body_font_features();
		f.resize(uint32_t(new_locale.body_features.size()));
		f.load_range(new_locale.body_features.data(), new_locale.body_features.data() + new_locale.body_features.size());
	}
	{
		auto f = new_locale_obj.get_header_font_features();
		f.resize(uint32_t(new_locale.header_features.size()));
		f.load_range(new_locale.header_features.data(), new_locale.header_features.data() + new_locale.header_features.size());
	}
	{
		auto f = new_locale_obj.get_map_font_features();
		f.resize(uint32_t(new_locale.map_features.size()));
		f.load_range(new_locale.map_features.data(), new_locale.map_features.data() + new_locale.map_features.size());
	}
	{
		auto f = new_locale_obj.get_locale_name();
		f.resize(uint32_t(locale_name.length()));
		f.load_range((uint8_t const*)locale_name.data(), (uint8_t const*)locale_name.data() + locale_name.length());
	}
	{
		auto f = new_locale_obj.get_fallback();
		f.resize(uint32_t(new_locale.fallback.length()));
		f.load_range((uint8_t const*)new_locale.fallback.data(), (uint8_t const*)new_locale.fallback.data() + new_locale.fallback.length());
	}
	{
		auto f = new_locale_obj.get_display_name();
		f.resize(uint32_t(new_locale.display_name.length()));
		f.load_range((uint8_t const*)new_locale.display_name.data(), (uint8_t const*)new_locale.display_name.data() + new_locale.display_name.length());
	}
}


} // namespace parsers
