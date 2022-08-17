/*                                                                      
  @@@@@@@@@@@                                       @@@@@@@@@@@         
  @@@@@@@@@@@@@@@@@                           @@@@@@@@@@@@@@@@@         
   @@@@@@@@@@@@@@@@@@    ##########,,,##   @@@@@@@@@@@@@@@@@@           
     @@@@@@@@@@@@@@@@@@  %%%%%%%%%%%%%((  @@@@@@@@@@@@@@@@@@            
       @@@@@@@@@@@@@@@@@%%%%%%%%%%%%%%((%@@@@@@@@@@@@@@@@@              
          @@@@@@@@@@@@@@%%%%%%%%%%%%%%%(%@@@@@@@@@@@@@@                 
            @@@@@@@@@@@ %%%%%%%%%%%%%%%(@@@@@@@@@@@@@                   
               @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@                      
                @@@@@@@▆▆▆▆▆@@@@@@@@@▆▆▆▆▆@@@@@@@                       
               @@@@@▆▆▆▆▆▆▆▆▆▆@@@▆▆▆▆▆▆▆▆@@@@                       
               @@@@#▆▆▆▆▆▆▆▆▆▆@@▆▆▆▆▆▆▆▆▆@@@@                     
               @@@@@▆▆▆▆▆▆▆▆▆@@@▆▆▆▆▆▆▆▆▆@@@@                       
                @@@@@,▆▆▆▆▆▆@@@@@@▆▆▆▆▆▆@@@@@                        
                @@@@@@@@@@@@@@@@ @@@@@@@@@@@@@@@                        
                 @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@                         
                   @@@@@@@@@@@@@@@@@@@@@@@@@@@                          
                    ,@@@@@@@@@ @@@%@@@@@@@@@,                           
                  ,,,,,,@@@@@@@@@@@@@@@@@@,,,,,,                        
                 ,,,,,,,@@@@@@@@@@@@@@@@@@,,,,,,                        
                   ,,,,,,,,,,,,@@@@,,,,,,,,,((                          
                      ((/,,,,,,,,,,,,,,((((%                            
                       %%%%(((((((((((((%%%                             
                      %%%%%%%%%%%%%%%%%%%%%                             
                      %%%%%%%%%%%%%%%%%%%%%%                            
                     %%%%%%*,%,%%%%%%%%%%/%%%                           
===================================================
===================================================
THIS IS A PROFESSIONAL BOT.
THEIR NAME IS LIPPO, I GUESS???

AND NOW THEY'RE WRITTEN IN C++! SHINY!

Written by Diane Sparks for the VGDC @ UCI server
Uses D++ by Craig Edwards
Art generated on manytools.org
===================================================
===================================================
*/

#include <dpp/dpp.h>
#include <dpp/fmt/format.h>

#include <iostream>
#include <regex>
#include <fstream>
#include <ctime>
#include <cstring>

namespace {
	bool lab_open = false;
	bool plant_reminder_sent_today = false;
	int cached_weekday = 7;

	constexpr dpp::snowflake VgdcServerId = 228326116270538753;
	constexpr dpp::snowflake RoleOperations = 364492667335213057;
	constexpr dpp::snowflake RoleAdmin = 230588792820596739;
	constexpr dpp::snowflake RoleAlum = 365231018824826901;
	constexpr dpp::snowflake RolePlantMoms = 967273024501977149;
	constexpr dpp::snowflake ChannelLabStatus = 629369478462963722;
	constexpr dpp::snowflake ChannelTableSlackers = 361379365134663691; 

	constexpr const char* TokenFile = "token/token.txt";

	static std::regex secret_lab_regex{R"reg((?:[s$]\s*(?:[e3 ]\s*)+[ck]\s*[r4]\s*(?:[e3 i1]\s*)+[t7]\s*([e3 ]\s*)*\s*[l1]\s*[a@8 ]\s*[b8]))reg", std::regex_constants::icase};
	static std::regex url_regex{R"reg(https?:\/\/.+\..+)reg", std::regex_constants::icase};
	static std::regex scam_regex{R"reg(\bnitro\b)reg", std::regex_constants::icase};
	static std::regex everyone_regex{R"reg(@everyone)reg"};
	static std::regex spamton_regex{R"reg(\bspamton\b)reg", std::regex_constants::icase}; // not my idea -Diane
}

void set_lab_open(dpp::cluster& bot, bool open) {
	lab_open = open;

	dpp::presence presence{
		open ? dpp::ps_online : dpp::ps_dnd,
		dpp::activity{
			dpp::at_game,
			open ? std::string("Game Lab OPEN") : std::string("Game Lab CLOSED"),
			open ? std::string("OPEN") : std::string("CLOSED"),
			std::string()
		}
	};

	bot.set_presence(presence);
}


std::string mention(dpp::snowflake id, bool role = false) {
	return fmt::format("<@{}{}>", role ? "&" : "", id);
}


int main(int argc, char* argv[]) {
	std::ifstream token_file{TokenFile};
	if (!token_file.is_open()) {
		std::cerr << "Failed to open token file!" << std::endl;
		return 1;
	}

	std::string token;
	token_file >> token;
	token_file.close();

	uint32_t intents = dpp::i_default_intents | dpp::i_message_content;
	dpp::cluster bot{token, intents};

	// CL options
	bool op_milgro = false;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--milgro")) {
			op_milgro = true;
		}
	}

	bot.on_ready([&bot, &op_milgro] (const dpp::ready_t&  event) {
		if (dpp::run_once<struct lippo_setup>()) {
			std::cout << fmt::format("Connected as {}", bot.me.username) << std::endl;

			set_lab_open(bot, false);
		
			{
				std::time_t now = std::time(nullptr);
				std::tm* time = std::localtime(&now);
				cached_weekday = time->tm_wday;
			}

			bot.start_timer([&bot] (dpp::timer timer) {
					std::time_t now = std::time(nullptr);
					std::tm* time = std::localtime(&now);

					if (lab_open && (time->tm_hour >= 22 || time->tm_hour <= 5)) {
						set_lab_open(bot, false);
					}
			}, 540);

			if (op_milgro) {
				bot.start_timer([&bot] (dpp::timer timer) {
					std::time_t now = std::time(nullptr);
					std::tm* time = std::localtime(&now);
			
					if (time->tm_wday != cached_weekday) {
						cached_weekday = time->tm_wday;
						plant_reminder_sent_today = false;
					}
			
					if (!plant_reminder_sent_today && time->tm_wday == 2 && time->tm_hour == 14) {
						bot.message_create(dpp::message(ChannelTableSlackers, fmt::format("{} Don't forget to water Milgro.", mention(RolePlantMoms, true))));
						plant_reminder_sent_today = true;
					}
				}, 600);
			}

			dpp::slashcommand command_labopen{"labopen", "Change Game Lab status to open", bot.me.id};
			command_labopen.disable_default_permissions();
			dpp::slashcommand command_labclose{"labclose", "Change Game Lab status to closed", bot.me.id};
			command_labclose.disable_default_permissions();

			bot.global_command_create(command_labopen);
			bot.global_command_create(command_labclose);
		}
	});

	bot.on_message_create([&bot] (const dpp::message_create_t&  event) {
		if (event.msg.author.id == bot.me.id) {
			return;
		}

		if (std::regex_search(event.msg.content, url_regex) && std::regex_search(event.msg.content, scam_regex)) {
			std::string message = std::regex_replace(event.msg.content, everyone_regex, "");
			bot.message_delete(event.msg.id, event.msg.channel_id);
			bot.message_create(dpp::message(event.msg.channel_id, "I just automatically removed a message that contained words we've recently seen in malicious messages. If this is a mistake, please DM one of the programming officers."));
			bot.message_create(dpp::message(event.msg.channel_id, fmt::format("Deleted message:\n{}", std::regex_replace(message, url_regex, "[LINK BLOCKED]"))));
			return;
		}

		// Spamton Easter Egg
		if (std::regex_search(event.msg.content, url_regex) && std::regex_search(event.msg.content, spamton_regex)) {
			std::string message = event.msg.content;
			bot.message_delete(event.msg.id, event.msg.channel_id);
			bot.message_create(dpp::message(event.msg.channel_id, std::regex_replace(message, url_regex, "[[HYPERLINK BLOCKED]]")));
			bot.message_create(dpp::message(event.msg.channel_id, "You don't have enough Kromer to post that."));
			return;
		}

		if (std::regex_search(event.msg.content, secret_lab_regex)) {
			event.reply("I think you mean \"Quiet Lab.\"", true);
			return;
		}
	});

	bot.on_interaction_create([&bot] (const dpp::interaction_create_t& event) {
		if (event.command.get_command_name() == "labopen") {
			set_lab_open(bot, true);
			event.reply("Game lab is now OPEN");
			return;
		} else if (event.command.get_command_name() == "labclose") {
			set_lab_open(bot, false);
			event.reply("Game lab is now CLOSED");
			return;
		}
	});

	bot.start(false);
}
